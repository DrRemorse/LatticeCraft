#include <stdio.h>
#include <stdlib.h>
// #include <cxxabi.h>
#include <string>

#define NUM_SYMBOLS  15

const std::string demangle(const char* name)
{
  // int status = -4;
  // char* res = abi::__cxa_demangle(name, NULL, NULL, &status);
  std::string ret_val( /* (status == 0) ? res : */ name);
  // std::free(res);
  return ret_val;
}

#ifdef __linux__
#include <execinfo.h>

void dump_trace()
{
  void*  array[NUM_SYMBOLS];
  size_t size    = backtrace(array, NUM_SYMBOLS);
  char** strings = backtrace_symbols (array, size);

  for (size_t i = 0; i < size; i++) {
    std::string whole(strings[i]);
    auto one = whole.find_first_of("(")+1;
    auto two = whole.find("+", one);

    std::string before;
    std::string name;
    std::string after;

    if (two == whole.npos) {
      before = ""; after = ""; name = whole;
    }
    else {
      before = std::string(whole, 0, one);
      name   = std::string(whole, one, two - one);
      after  = std::string(whole, two);
    }

    printf("[%zu] %s %s %s\n", i,
                  before.c_str(),
                  demangle(name.c_str()).c_str(),
                  after.c_str());
  }
  free(strings);
}

#else

#include <windows.h>
#include <imagehlp.h>
void dump_trace()
{
  void         * stack[100];
  unsigned short frames;
  SYMBOL_INFO  * symbol;
  HANDLE         process;

  process = GetCurrentProcess();

  SymInitialize( process, NULL, TRUE );

  frames               = CaptureStackBackTrace( 0, 100, stack, NULL );
  symbol               = (SYMBOL_INFO*) std::calloc( sizeof(SYMBOL_INFO) + 256, 1 );
  symbol->MaxNameLen   = 255;
  symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

  for (auto i = 0; i < frames; i++)
  {
    SymFromAddr( process, (DWORD64) stack[i], 0, symbol );

    printf( "%u: %s - %p\n", frames - i - 1, symbol->Name, (void*) symbol->Address );
  }
  std::free(symbol);
}

#endif
