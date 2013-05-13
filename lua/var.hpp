#pragma once

#include <type_traits>
#include <iostream>

namespace lua
{

template<typename T>
class result {
public:
	bool success() const {
		return success_;
	}

	error error() const {
		if(success_) {
			throw exception("Trying to get error from successful result.");
		}
		return err_;
	}

	T value() const {
		if(!success_) {
			throw exception("Trying to get value from failed result.");
		}
		return val_;
	}

	const operator T() const {
		return value();
	}

	const operator bool() const {
		return success_;
	}

private:
	result(lua::error&& err) : err_{err}, success_{false} {}
	result(const lua::error& err) : err_{err}, success_{false} {}
	result(T&& val) : val_{val}, success_{true} {}
	result(const T& val) : val_{val}, success_{true} {}

	bool success_;
	union {
		lua::error err_;
		T val_;
	};
	friend class var;
};

template<>
class result<void> {
public:
	
	bool has_value() const {
		return success_;
	}

	error error() const {
		return err_;
	}

	const operator bool() const {
		return success_;
	}

private:
	result() : success_{true} {}
	result(lua::error&& err) : err_{err}, success_{false} {}
	result(const lua::error& err) : err_{err}, success_{false} {}

	bool success_;
	lua::error err_;
	friend class var;
};

class var {
public:
	var(const var& other) = default;
	var(var&& other) = default;

	// Gets the value of the var
	val get_value() const {
		push();
		return val(L);
	}

	// Gets the value of the var typed
	template <typename T>
	T get() const {
		return get_value().get<T>();	
	}

	template <typename T>
	bool is() const {
		stack_guard g(L);
		return dirty_is<T>();
	}

	template <typename T>
	T as(T&& fallback) {
		stack_guard g(L);
		return dirty_is<T>() ? val(L).get<T>() : fallback;
	}

	// Assigns the var with another var
	var& operator=(const var& var) {
		stack_guard g(L);
		parent_key_();
		if(L == var.L) {
			stack_guard g2(L, true);
			var.push();
		} else {
			var.get_value().push(L);
		}
		lua_settable(L, virtual_index_ ? virtual_index_ : -3);
		return *this;
	}
	
	var& operator=(const val& val) {
		stack_guard g(L);
		parent_key_();
		val.push(L);
		lua_settable(L, virtual_index_ ? virtual_index_ : -3);
		return *this;
	}

	var operator[](val idx) {
		return var(*this, idx);
	}

	var operator[](var idx) {
		return var(*this, idx.get_value());
	}

	template<typename... TArgs>
	result<void> operator()(TArgs... args) {
		return invoke<void>(args...);
	}

	template<typename TOut, typename... TArgs>
	result<TOut> invoke(TArgs... args) {
		if(dirty_is<TOut(TArgs...)>()) {
			val::push_all<TArgs...>(L, args...);
			return caller<TOut>::call(L, sizeof...(TArgs));
		}
		throw exception("Tried to invoke non-function.");
	}

	void do_chunk(const std::string& str) {
		luaL_loadstring(L, str.c_str());
		int idx = lua_gettop(L);
		{
			stack_guard g(L);
			push();
			if(lua_istable(L, -1)) {
				lua_setfenv(L, idx);
			}
		}
		lua_call(L, 0, LUA_MULTRET);
	}

protected:
	void push() const {
		parent_key_();
		lua_gettable(L, virtual_index_ ? virtual_index_ : -2);
	}

	template<typename T>
	bool dirty_is() const {
		push();
		return typed_is<T>::is(L);
	}
	
	template<typename T, class Enable = void>
	struct caller {
		static result<T> call(lua_State* L, int nargs) {
			auto error = root.call(nargs, sizeof(T));
			if(error) {
				return *error;
			}
			return val::popper<T>::pop(L);
		}
	};

	template<typename T>
	struct caller<T, typename std::enable_if<std::is_void<T>::value>::type> {
		static result<T> call(lua_State* L, int nargs) {
			auto error = root.call(nargs, sizeof(T));
			if (error) {
				return *error;
			}
			return result<T>();
		}
	};

	template<typename T, class Enable = void>
	struct typed_is {
		static inline bool is(lua_State* L) {
			std::cout << "Unknown Type check." << std::endl;
			return false;
		}
	};

	template<typename T>
	struct typed_is<T, typename std::enable_if<std::is_arithmetic<T>::value>::type> {
		static inline bool is(lua_State* L) {
			return !lua_isnoneornil(L,-1) && (lua_isboolean(L,-1) || lua_isnumber(L,-1));
		}
	};

	template<typename T>
	struct typed_is<T, typename std::enable_if<std::is_same<T, std::string>::value>::type> {
		static inline bool is(lua_State* L) {
			return !lua_isnoneornil(L,-1) && lua_isstring(L,-1);
		}
	};

	template<typename T>
	struct typed_is<T, typename std::enable_if<std::is_same<T, const char*>::value>::type> {
		static inline bool is(lua_State* L) {
			return !lua_isnoneornil(L,-1) && lua_isstring(L,-1);
		}
	};

	template<typename T>
	struct typed_is<T, typename std::enable_if<std::is_function<T>::value>::type> {
		static inline bool is(lua_State* L) {
			return !lua_isnoneornil(L,-1) && lua_isfunction(L,-1);
		}
	};

	template<typename T>
	struct typed_is<T, typename std::enable_if<std::is_pointer<T>::value>::type> {
		static inline bool is(lua_State* L) {
			return !lua_isnoneornil(L,-1) && lua_islightuserdata(L,-1);
		}
	};

	var(var var, val key) 
		: L{var.L}
		, parent_key_{[var, key]() { var.push(); key.push(var.L); }}
		, virtual_index_{0}
	{}

	var(lua_State* L, int virtual_index, val key) 
		: L{L}
		, parent_key_{[L, key](){key.push(L);}}
		, virtual_index_{virtual_index}
	{}

	lua_State* L;
	std::function<void(void)> parent_key_;
	int virtual_index_;

	friend class root;
};

}