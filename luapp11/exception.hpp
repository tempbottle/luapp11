#pragma once

#include <exception>
#include <string>
#include <sstream>

namespace luapp11 {
class exception : public std::exception {
 public:
  const char* what() const noexcept override { return what_.c_str(); }

  const std::string& stack() const { return stack_; }

 private:
  exception(std::string what) : what_(what), stack_() {}

  exception(std::string what, lua_State* L)
      : what_(what),
        stack_(stackdump(L)) {
  }

  static std::string stackdump(lua_State* L) {
    std::stringstream ss;
    ss << "Stack Dump:" << std::endl;

    const int top = lua_gettop(L);
    for (int i = 1; i <= top; i++) {
      ss << lua_typename(L, i);
      switch (lua_type(L, i)) {
        case LUA_TSTRING:
          ss << ": " << lua_tostring(L, i) << std::endl;
          break;
        case LUA_TNUMBER:
          ss << ": " << lua_tonumber(L, i) << std::endl;
          break;
        case LUA_TBOOLEAN:
          ss << ": " << (lua_toboolean(L, i) ? "true" : "false") << std::endl;
          break;
        case LUA_TUSERDATA:
        case LUA_TLIGHTUSERDATA:
          ss << ": " << lua_topointer(L, i) << std::endl;
          break;
        case LUA_TTHREAD:
        default:
          ss << std::endl;
          break;
      }
    }
    ss << std::endl;
    return ss.str();
  }

  std::string what_;
  std::string stack_;

  friend class error;
  friend class var;
  friend class val;
  friend class root;
  template <typename T> friend class result;
};

class error {
 public:
  ~error() = default;
  error(const error&) = default;

  enum class type {
    none = 0, runtime = LUA_ERRRUN, memory = LUA_ERRMEM, error = LUA_ERRERR
  };

  const type error_type() const { return type_; }

  const std::string& message() const { return message_; }

  const std::string& stack() const { return stack_; }

  const explicit operator bool() const { return type_ != type::none; }

 private:
  error() : type_ { type::none }
  {}
  error(int t, std::string message, lua_State* L) : type_ { (type) t }
  , message_ { message }
  , stack_ { exception::stackdump(L) }
  {}

  type type_;
  std::string message_;
  std::string stack_;

  friend class var;
  friend class val;
  friend class root;
  template <typename T> friend class result;
};
}