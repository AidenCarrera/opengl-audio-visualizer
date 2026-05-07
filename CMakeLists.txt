cmake_minimum_required (VERSION 3.5)

project (vermilion9)

LINK_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib )

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Second, for multi-config builds (e.g. msvc)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/lib )
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/lib )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

set (CMAKE_DEBUG_POSTFIX "_d")

find_package(OpenGL REQUIRED)

if(WIN32)
set(COMMON_LIBS vermilion ${OPENGL_LIBRARIES} optimized glfw3 debug glfw3_d winmm)
elseif (UNIX)
set(COMMON_LIBS vermilion ${OPENGL_LIBRARIES} glfw ${GLFW_LIBRARIES} GL rt dl)
else()
set(COMMON_LIBS vermilion)
endif()

set(COMMON_HEADERS include/vgl.h include/vapp.h)

set(COMMON_LIBS ${COMMON_LIBS} ${EXTRA_LIBS})

add_library(vermilion
            lib/gl3w.c
            lib/LoadShaders.cpp
            lib/targa.cpp
            lib/vdds.cpp
            lib/loadtexture.cpp
            lib/vermilion.cpp
            lib/vbm.cpp
)

set(RUN_DIR ${PROJECT_SOURCE_DIR}/bin)

set(EXAMPLES
  demo
)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

foreach(EXAMPLE ${EXAMPLES})
  add_executable(${EXAMPLE} src/${EXAMPLE}/${EXAMPLE}.cpp ${COMMON_HEADERS} external/kissfft/kiss_fft.c external/kissfft/kiss_fftr.c)
  set_property(TARGET ${EXAMPLE} PROPERTY DEBUG_POSTFIX _d)

  target_link_libraries(${EXAMPLE} ${COMMON_LIBS})

  set_target_properties(${EXAMPLE} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
      RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin
      RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin
      VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
  )

endforeach()

IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_LINUX")
ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

include_directories( include )
include_directories(lib/glfw/include)
include_directories(${CMAKE_SOURCE_DIR}/external/kissfft)

add_subdirectory(lib/glfw)