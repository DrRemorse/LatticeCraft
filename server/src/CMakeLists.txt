
set(SUB_SOURCES
    server_main.cpp
    client.cpp
    pollinator.cpp
  )

FUNCTION(PREPEND var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

PREPEND(SOURCES ${CMAKE_CURRENT_SOURCE_DIR} ${SUB_SOURCES})
set(SOURCES "${SOURCES}" PARENT_SCOPE)
