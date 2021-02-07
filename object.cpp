#include "object.h"
#include "statement.h"

#include <sstream>
#include <string_view>
#include <iostream>

using namespace std;

namespace Runtime {

void ClassInstance::Print(std::ostream& os) {
	if (HasMethod("__str__", 0)) {
		ObjectHolder str_obj;
		str_obj = Call("__str__", {});
		auto obj = str_obj.TryAs<Runtime::String>();
		if (obj) {
			obj->Print(os);
		}
		else {
			str_obj.TryAs<Runtime::Number>()->Print(os);
		}
	}
	else {
		os << this;
	}
}

bool ClassInstance::HasMethod(const std::string& method, size_t argument_count) const {
	const auto* m = class_.GetMethod(method);
	if (m && m->formal_params.size() == argument_count) {
		return true;
	}
	return false;
}

const Closure& ClassInstance::Fields() const {
	return closure_;
}

Closure& ClassInstance::Fields() {
	return closure_;
}

ClassInstance::ClassInstance(const Class& cls) 
	: class_(cls) {}

ObjectHolder ClassInstance::Call(const std::string& method, const std::vector<ObjectHolder>& actual_args) {
	try {
		const auto* m = class_.GetMethod(method);
		if (!m) {
			return ObjectHolder::None();
		}
		closure_["self"] = ObjectHolder::Share(*this);
		for (size_t i = 0; i < m->formal_params.size() && actual_args.size(); ++i) {
			closure_[m->formal_params[i]] = actual_args[i];
		}

		return m->body->Execute(closure_);
	}
	catch (ObjectHolder obj) {
		return obj;
	}
}

Class::Class(std::string name, std::vector<Method> methods, const Class* parent) 
	: name_(name) {
	parent_ = parent;
	methods_ = std::move(methods);
}

const Method* Class::GetMethod(const std::string& name) const {
	for (size_t i = 0; i < methods_.size(); ++i) {
		if (methods_[i].name == name) {
			return &methods_[i];
		}
	}
	if (parent_) {
		const auto* m = parent_->GetMethod(name);
		if (m) {
			return m;
		}
	}

	return nullptr;
}

void Class::Print(ostream& os) {
	os << "Class::Print()" << std::endl;
}

const std::string& Class::GetName() const {
	return name_;
}

void Bool::Print(std::ostream& os) {
	os << (GetValue() ? "True" : "False");
}

} /* namespace Runtime */
