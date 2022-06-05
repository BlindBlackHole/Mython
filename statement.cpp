#include "statement.h"
#include "object.h"

#include <iostream>
#include <sstream>
#include <optional>
#include <variant>

using namespace std;

namespace Ast {

using Runtime::Closure;

ObjectHolder Assignment::Execute(Closure& closure) {
	closure[var_name] = right_value->Execute(closure);
	return closure[var_name];
}

Assignment::Assignment(std::string var, std::unique_ptr<Statement> rv) : var_name(var), right_value(move(rv)) {
}

VariableValue::VariableValue(std::string var) {
	dotted_ids.push_back(var);
}

VariableValue::VariableValue(std::vector<std::string> dotted_ids) : dotted_ids(dotted_ids){
}

ObjectHolder VariableValue::Execute(Closure& closure) {
	auto var = closure.find(dotted_ids[0]);
	if (var == closure.end()) {
		throw std::runtime_error("There is no such variable: " + dotted_ids[0]);
	}
	ObjectHolder object = closure[dotted_ids[0]];
	for (size_t i = 1; i < dotted_ids.size(); ++i) {
		auto obj_name = dotted_ids[i];
		auto obj = object;
		auto obj_inst = obj.TryAs<Runtime::ClassInstance>();
		object = obj_inst->Fields()[obj_name];
	}
	return object;
}

unique_ptr<Print> Print::Variable(std::string var) {
	return std::make_unique<Print>(std::make_unique<VariableValue>(var));
}

Print::Print(unique_ptr<Statement> argument) {
	args.push_back(std::move(argument));
}

Print::Print(vector<unique_ptr<Statement>> args) : args(move(args)) {
}

ObjectHolder Print::Execute(Closure& closure) {
	for (size_t i = 0; i < args.size(); ++i) {
		//Convert every arg in string
		Stringify to_string(move(args[i]));
		//Print converted string
		ObjectHolder temp;
		try {
			temp = to_string.Execute(closure); 
			temp->Print(*output);
		}
		catch (ObjectHolder obj) {
			obj->Print(*output);
		}
		if (i != args.size() - 1) {
			*output << " ";
		}
	}
	*output << "\n";
	return ObjectHolder::None();
}

ostream* Print::output = &cout;

void Print::SetOutputStream(ostream& output_stream) {
  output = &output_stream;
}

MethodCall::MethodCall(
	std::unique_ptr<Statement> object
	, std::string method
	, std::vector<std::unique_ptr<Statement>> args
) : object(move(object)), method(std::move(method)), args(move(args))
{
}

ObjectHolder MethodCall::Execute(Closure& closure) {
	auto obj = object->Execute(closure).TryAs<Runtime::ClassInstance>();
	if (obj->HasMethod(method, args.size())) {
		std::vector<ObjectHolder> obj_args;
		for (auto& arg : args) {
			obj_args.push_back(arg->Execute(closure));
		}
		return obj->Call(method, obj_args);
	}
	return ObjectHolder::None();
}

FunctionDefinition::FunctionDefinition(ObjectHolder func)
    : func(std::move(func)) {}

ObjectHolder FunctionDefinition::Execute(Runtime::Closure&) {
    return func;
}

FunctionCall::FunctionCall(
      std::unique_ptr<Statement> function
    , std::string method
    , std::vector<std::unique_ptr<Statement>> args
) : function(std::move(function)), method(std::move(method)), args(move(args))
{}

ObjectHolder FunctionCall::Execute(Closure& closure) {
    auto func = function->Execute(closure).TryAs<Runtime::Function>();
    std::vector<ObjectHolder> func_args;
    for (auto& arg : args) {
        func_args.push_back(arg->Execute(closure));
    }
    return func->Call(func_args);
}

ObjectHolder Stringify::Execute(Closure& closure) {
	std::ostringstream str;
	ObjectHolder obj;
	obj = argument->Execute(closure);
	if (!obj) {
		return ObjectHolder::Own(Runtime::String("None"));
	}
	obj->Print(str);
	return ObjectHolder::Own(Runtime::String(str.str()));
}

ObjectHolder Add::Execute(Closure& closure) {
	{
		auto obj_l = lhs->Execute(closure);
		auto obj_r = rhs->Execute(closure);
		auto obj_lhs = obj_l.TryAs<Runtime::ClassInstance>();
		if (obj_lhs) {
			if (obj_lhs->HasMethod("__add__", 1)) {
				return obj_lhs->Call("__add__", { obj_r });
			}
		}
		auto obj_rhs = obj_r.TryAs<Runtime::ClassInstance>();
		if (obj_rhs) {
			if (obj_rhs->HasMethod("__add__", 1)) {
				return obj_rhs->Call("__add__", { obj_l });
			}
		}
	}
	auto obj_l = lhs->Execute(closure);
	auto obj_r = rhs->Execute(closure);
	auto obj_lhs = obj_l.TryAs<Runtime::Number>();
	auto obj_rhs = obj_r.TryAs<Runtime::Number>();
	if (obj_lhs == nullptr || obj_rhs == nullptr) {
		auto obj_lhs1 = lhs->Execute(closure); 
		auto obj_rhs1 = rhs->Execute(closure); 
		auto str2 = obj_lhs1.TryAs<Runtime::String>();
		auto str = obj_rhs1.TryAs<Runtime::String>();
		if (!str || !str2) {
			throw std::runtime_error("Adding strange objects");
		}
		auto s = str->GetValue();
		auto s2 = str2->GetValue();
		auto sum = s2 + s;
		auto obj_sum = Runtime::String(sum);
		return ObjectHolder::Own(move(obj_sum));
	}
	auto sum = obj_lhs->GetValue() + obj_rhs->GetValue();
	auto obj_sum = Runtime::ValueObject<int>(sum);
	return ObjectHolder::Own(move(obj_sum));
}

ObjectHolder Sub::Execute(Closure& closure) {
	auto obj_l = lhs->Execute(closure);
	auto obj_r = rhs->Execute(closure);
	auto obj_lhs = obj_l.TryAs<Runtime::Number>();
	auto obj_rhs = obj_r.TryAs<Runtime::Number>();
	if (obj_lhs == nullptr || obj_rhs == nullptr) {
		return ObjectHolder::None();
	}
	int sum = obj_lhs->GetValue() - obj_rhs->GetValue();
	Runtime::Number obj_sum = Runtime::Number(sum);
	return ObjectHolder::Own(move(obj_sum));
}

ObjectHolder Mult::Execute(Runtime::Closure& closure) {
	auto obj_l = lhs->Execute(closure);
	auto obj_r = rhs->Execute(closure);
	auto obj_lhs = obj_l.TryAs<Runtime::Number>();
	auto obj_rhs = obj_r.TryAs<Runtime::Number>();
	if (obj_lhs == nullptr || obj_rhs == nullptr) {
		return ObjectHolder::None();
	}
	auto sum = obj_lhs->GetValue() * obj_rhs->GetValue();
	auto obj_sum = Runtime::ValueObject<int>(sum);
	return ObjectHolder::Own(move(obj_sum));
}

ObjectHolder Div::Execute(Runtime::Closure& closure) {
	auto obj_l = lhs->Execute(closure);
	auto obj_r = rhs->Execute(closure);
	auto obj_lhs = obj_l.TryAs<Runtime::Number>();
	auto obj_rhs = obj_r.TryAs<Runtime::Number>();
	if (obj_lhs == nullptr || obj_rhs == nullptr) {
		return ObjectHolder::None();
	}
	if (obj_rhs->GetValue() == 0) {
		cerr << "Divided on zero!" << endl;
		return ObjectHolder::None();
	}
	auto sum = obj_lhs->GetValue() / obj_rhs->GetValue();
	auto obj_sum = Runtime::ValueObject<int>(sum);
	return ObjectHolder::Own(move(obj_sum));
}

ObjectHolder Compound::Execute(Closure& closure) {
	for (auto& statement : statements) {
		try {
			statement->Execute(closure);
		}
		catch (ObjectHolder obj) {
			throw obj;
		}
	}
	return ObjectHolder::None();
}

ObjectHolder Return::Execute(Closure& closure) {
	throw statement->Execute(closure);
}

ClassDefinition::ClassDefinition(ObjectHolder class_) : cls(class_) {
}

ObjectHolder ClassDefinition::Execute(Runtime::Closure&) {
	return cls;
}

FieldAssignment::FieldAssignment(
  VariableValue object, std::string field_name, std::unique_ptr<Statement> rv
)
  : object(std::move(object))
  , field_name(std::move(field_name))
  , right_value(std::move(rv))
{
}

ObjectHolder FieldAssignment::Execute(Runtime::Closure& closure) {
	auto obj = object.Execute(closure);
	auto class_obj = obj.TryAs<Runtime::ClassInstance>();
	class_obj->Fields()[field_name] = right_value->Execute(closure);
	return class_obj->Fields()[field_name];
}

IfElse::IfElse(
	std::unique_ptr<Statement> condition,
	std::unique_ptr<Statement> if_body,
	std::unique_ptr<Statement> else_body
)	: condition(move(condition)),
	if_body(move(if_body)),
	else_body(move(else_body))
{
}

ObjectHolder IfElse::Execute(Runtime::Closure& closure) {
	bool key = false;
	//if condition is bool
	auto cond_bool_statement = condition->Execute(closure);
	auto cond_bool = cond_bool_statement.TryAs<Runtime::Bool>();
	if (cond_bool) {
		key = cond_bool->GetValue();
	}
	//if conditoin is number
	auto cond_num = condition->Execute(closure).TryAs<Runtime::Number>();
	if (cond_num) {
		key = (cond_num->GetValue() != 0);
	}
	//if condition is string
	auto cond_str = condition->Execute(closure).TryAs<Runtime::String>();
	if (cond_str) {
		key = (!cond_str->GetValue().empty());
	}
	//if condition is Object
	auto cond_obj = condition->Execute(closure).TryAs<Runtime::ClassInstance>();
	if (cond_obj) {
		key = true;
	}

	if (key) {
		return if_body->Execute(closure);
	}
	else if (else_body) { //Else_body can be empty
		return else_body->Execute(closure);
	}
	return ObjectHolder::None();
}

template<typename T>
std::optional<T*> CheckValueType(ObjectHolder obj) {
	auto* typed_obj = obj.TryAs<T>();
	if (typed_obj) {
		return typed_obj;
	}
	return nullopt;
}

pair<bool, bool> GetTypedValues(ObjectHolder obj_l, ObjectHolder obj_r) {
	bool lhs_bool = false, rhs_bool = false;
	//bool
	if (auto obj = CheckValueType<Runtime::Bool>(obj_l)) {
		lhs_bool = obj.value()->GetValue();
	}
	if (auto obj = CheckValueType<Runtime::Bool>(obj_r)) {
		rhs_bool = obj.value()->GetValue();
	}
	//number
	if (auto obj = CheckValueType<Runtime::Number>(obj_l)) {
		lhs_bool = obj.value()->GetValue() != 0 ? true : false;
	}
	if (auto obj = CheckValueType<Runtime::Number>(obj_r)) {
		rhs_bool = obj.value()->GetValue() != 0 ? true : false;
	}
	//string
	if (auto obj = CheckValueType<Runtime::String>(obj_l)) {
		lhs_bool = !obj.value()->GetValue().empty();
	}
	if (auto obj = CheckValueType<Runtime::String>(obj_r)) {
		rhs_bool = !obj.value()->GetValue().empty();
	}
	//Class object
	if (auto obj = CheckValueType<Runtime::ClassInstance>(obj_l)) {
		lhs_bool = true;
	}
	if (auto obj = CheckValueType<Runtime::ClassInstance>(obj_r)) {
		rhs_bool = true;
	}
	return { lhs_bool, rhs_bool };
}


ObjectHolder Or::Execute(Runtime::Closure& closure) {
	auto obj_l = lhs->Execute(closure);
	auto obj_r = rhs->Execute(closure);
	auto [lhs_bool, rhs_bool] = GetTypedValues(obj_l, obj_r);
	if (lhs_bool || rhs_bool) {
		return ObjectHolder::Own(Runtime::Bool(true));
	}
	return ObjectHolder::Own(Runtime::Bool(false));
}

ObjectHolder And::Execute(Runtime::Closure& closure) {
	auto obj_l = lhs->Execute(closure);
	auto obj_r = rhs->Execute(closure);
	auto [lhs_bool, rhs_bool] = GetTypedValues(obj_l, obj_r);
	if (lhs_bool && rhs_bool) {
		return ObjectHolder::Own(Runtime::Bool(true));
	}
	return ObjectHolder::Own(Runtime::Bool(false));
}

ObjectHolder Not::Execute(Runtime::Closure& closure) {
	auto obj = argument->Execute(closure);
	auto [lhs_bool, rhs_bool] = GetTypedValues(obj, obj);
	return ObjectHolder::Own(Runtime::Bool(!lhs_bool));
}

Comparison::Comparison(
  Comparator cmp, unique_ptr<Statement> lhs, unique_ptr<Statement> rhs
) : comparator(cmp), left(move(lhs)), right(move(rhs))
{
}

ObjectHolder Comparison::Execute(Runtime::Closure& closure) {
	bool ans = comparator(left->Execute(closure), right->Execute(closure));
	return ObjectHolder::Own(Runtime::Bool(ans));
}

NewInstance::NewInstance(
  const Runtime::Class& class_, std::vector<std::unique_ptr<Statement>> args
)
  : class_(class_)
  , args(std::move(args))
{
}

NewInstance::NewInstance(const Runtime::Class& class_) : NewInstance(class_, {}) {
}

ObjectHolder NewInstance::Execute(Runtime::Closure& closure) {
	Runtime::ClassInstance inst(class_);
	std::vector<ObjectHolder> actual_args;
	for (const auto& arg : args) {
		actual_args.push_back(arg->Execute(closure));
	}
	inst.Call("__init__", actual_args);
    return ObjectHolder::Own(move(inst));
}


} /* namespace Ast */
