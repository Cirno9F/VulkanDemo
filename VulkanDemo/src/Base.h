#pragma once

#include "Log.h"

#include <memory>

template<typename T>
using Scope = std::unique_ptr<T>;
template<typename T, typename ... Args>
constexpr Scope<T> CreateScope(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
using Ref = std::shared_ptr<T>;
template<typename T, typename ... Args>
constexpr Ref<T> CreateRef(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

#define ASSERT_IFNOT(x, ...) { if(!(x)) { LOG_ERROR("Assertion Failed: {0}" ,__VA_ARGS__);  __debugbreak(); } }

#define ASSERT(...) { LOG_ERROR("Assertion Failed: {0}" , __VA_ARGS__);  __debugbreak(); } 