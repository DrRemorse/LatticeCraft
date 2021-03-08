#pragma once
#include <fmod/fmod.hpp>
#include <fmod/fmod_errors.h>
#include <cstdio>

namespace sound
{
  inline void ERRCHECK_fn(FMOD_RESULT result, const char *file, int line)
  {
      if (result != FMOD_OK)
      {
        fprintf(stderr, "%s(%d): FMOD error %d - %s",
                file, line, result, FMOD_ErrorString(result));
        std::abort();
      }
  }
  #define SND_CHECK(_result) ERRCHECK_fn(_result, __FILE__, __LINE__)
}
