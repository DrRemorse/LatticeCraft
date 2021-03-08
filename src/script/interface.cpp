#include "script.hpp"

using namespace luabridge;

static void print(const std::string& text) {
  printf(">> %s\n", text.c_str());
}

#include <cstring>
//#include <unistd.h>
#include <direct.h>
static inline std::string current_directory() {
  char buffer[512];
  char* ret = _getcwd(buffer, sizeof(buffer));
  size_t len = strnlen(ret, sizeof(buffer));
  ret[len++] = '/'; ret[len] = 0;
  return std::string(ret, len);
}

void Script::add_lua_interface()
{
  auto gns = getGlobalNamespace(this->state);
  gns.beginNamespace("cc")
      .addFunction("print", print)
  .endNamespace();
}
