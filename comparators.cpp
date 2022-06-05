#include "comparators.h"
#include "object.h"
#include "object_holder.h"

#include <functional>
#include <optional>
#include <iostream>
#include <sstream>

using namespace std;

namespace Runtime {

bool Equal(ObjectHolder lhs, ObjectHolder rhs) {
	{
		auto res = Compare<Runtime::Number, int>(lhs, rhs, 
			[=](int l, int r) { return l == r; });
		if (res != nullopt) {
			return *res;
		}
	}
	{
		auto res = Compare<Runtime::String, std::string>(lhs, rhs, 
			[=](std::string l, std::string r) { return l == r; });
		if (res != nullopt) {
			return *res;
		}
	} 
	std::cerr	<< "Compared strange objects, rhs: " << typeid(rhs).name() << std::endl 
				<< "lhs: " << typeid(lhs).name() << std::endl;
	return false;
}


bool Less(ObjectHolder lhs, ObjectHolder rhs) {
	{
		auto res = Compare<Runtime::Number, int>(lhs, rhs,
			[=](int l, int r) { return l < r; });
		if (res != nullopt) {
			return *res;
		}
	}
	{
		auto res = Compare<Runtime::String, std::string>(lhs, rhs,
			[=](std::string l, std::string r) { return l < r; });
		if (res != nullopt) {
			return *res;
		}
	}
	std::cerr	<< "Compared strange objects, rhs: " << typeid(rhs).name() << std::endl
				<< "lhs: " << typeid(lhs).name() << std::endl;
	return false;
}

} /* namespace Runtime */
