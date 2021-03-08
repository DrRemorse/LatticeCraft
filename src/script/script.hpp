#ifndef SCRIPT_SCRIPT_HPP
#define SCRIPT_SCRIPT_HPP

#include <common.hpp>
#include <string>
#include <stdexcept>
extern "C" {
  //#include <lua5.1/lua.h>
  //#include <lua5.1/lualib.h>
  //#include <lua5.1/lauxlib.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include <LuaBridge/LuaBridge.h>

class Script {
public:
  // call any script function, with any parameters
  // returns lua reference
  template <typename... Args>
  inline luabridge::LuaRef retcall(const std::string& name, Args&&...);
  // returns nada
  template <typename... Args>
  inline void call(const std::string& name, Args&&...);

  // create Lua object
  auto ref() const { return luabridge::LuaRef(this->state); }
  auto new_table() const { return luabridge::newTable(this->state); }

  bool add_file(std::string file);

  Script();
	~Script();
private:
  void add_lua_interface();

	lua_State* state;
};

template <typename... Args>
inline luabridge::LuaRef Script::retcall (const std::string& name, Args&&... args)
{
  auto lref = luabridge::getGlobal(this->state, name.c_str());
  if (lref.isNil()) throw std::runtime_error(
              std::string("LUA: '") + name + "' did not exist");
  auto ret = lref(std::forward<Args>(args)...);
  return ret;
}

template <typename... Args>
inline void Script::call (const std::string& name, Args&&... args)
{
  auto lref = luabridge::getGlobal(this->state, name.c_str());
  if (lref.isNil()) throw std::runtime_error(
              std::string("LUA: '") + name + "' did not exist");
  lref(std::forward<Args>(args)...);
}

#endif
