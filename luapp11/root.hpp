#pragma once

namespace luapp11 {

class root {
 public:
  root() : L { luaL_newstate() }
  {
    luaL_openlibs(L);
    lua_atpanic(L, &panic);
  }

  var operator[](val key) const { return var(L, LUA_GLOBALSINDEX, key); }

 private:
  static int panic(lua_State* L) { throw luapp11::exception("lua panic", L); }

  lua_State* L;

  friend void do_chunk(const std::string& str);
};

static root root;

inline void do_chunk(const std::string& str) {
  luaL_dostring(root.L, str.c_str());
}

}