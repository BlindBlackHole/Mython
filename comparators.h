#pragma once

#include "object_holder.h"
#include <optional>
#include <functional>

namespace Runtime {

	template<typename T1, typename T2>
	std::optional<bool> Compare(
		ObjectHolder lhs, ObjectHolder rhs, std::function<bool(T2, T2)> comp) {
		auto obj_lhs = lhs.TryAs<T1>();
		auto obj_rhs = rhs.TryAs<T1>();
		if (obj_lhs && obj_rhs) {
			return comp(obj_lhs->GetValue(), obj_rhs->GetValue());
		}
		return std::nullopt;
	}

bool Equal(ObjectHolder lhs, ObjectHolder rhs);
bool Less(ObjectHolder lhs, ObjectHolder rhs);

inline bool NotEqual(ObjectHolder lhs, ObjectHolder rhs) {
  return !Equal(lhs, rhs);
}

inline bool Greater(ObjectHolder lhs, ObjectHolder rhs) {
  return !Less(lhs, rhs) && !Equal(lhs, rhs);
}

inline bool LessOrEqual(ObjectHolder lhs, ObjectHolder rhs) {
  return !Greater(lhs, rhs);
}

inline bool GreaterOrEqual(ObjectHolder lhs, ObjectHolder rhs) {
  return !Less(lhs, rhs);
}

} /* namespace Runtime */
