
SET(COMMON_SOURCES
    db/blockdata.cpp
    db/itemdata.cpp
    biomes.cpp
    readonly_blocks.cpp
  )

add_library(common STATIC ${COMMON_SOURCES})
