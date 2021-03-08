#include "script.hpp"

#include <library/log.hpp>
using namespace library;

using namespace luabridge;

Script::Script()
{
	this->state = luaL_newstate();
  luaL_openlibs(this->state);
  // add game interface
  add_lua_interface();
  // add engine common script
  if (add_file("mod/std/scripts/init.lua") == false) {
    throw std::runtime_error("Missing engine common script");
  }
}
Script::~Script()
{
	if (this->state) lua_close(this->state);
}

bool Script::add_file(std::string file)
{
	// add script file
	if (luaL_loadfile(this->state, file.c_str())
   || lua_pcall(this->state, 0, LUA_MULTRET, 0)) {
    fprintf(stderr, "\nFATAL ERROR:\n  %s\n\n",
		        lua_tostring(this->state, -1));
    return false;
  }
  return true;
}
