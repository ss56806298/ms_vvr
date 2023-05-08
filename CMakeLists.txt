cmake_minimum_required(VERSION 3.0)

project(ms_vvr)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
src/websocket.h
src/websocket.cpp
src/log.h
src/log.cpp
src/module-singleton.h
src/common-def.h
src/ssml-creator.cpp
src/ssml-creator.h
src/utilities.h
src/ms-vvr.h)

option(VVC_GENE_SHARED_LIB "Don't build project as DLL." OFF)

if (VVC_GENE_SHARED_LIB)
	add_library(ms_vvr SHARED ${SOURCE_FILES} src/dllmain.cpp)
	add_definitions(-DDLLExport)
else()
	add_executable(ms_vvr ${SOURCE_FILES} src/main.cpp)
endif()

include_directories(thirdparty)

include_directories(thirdparty/libwebsockets)

include_directories(thirdparty/fast-cpp-csv-parser)

target_link_directories(ms_vvr PUBLIC thirdparty/libwebsockets/libs)

target_link_directories(ms_vvr PUBLIC thirdparty/openssl/libs)

target_link_libraries(ms_vvr PUBLIC Ws2_32
Winmm
IPHLPAPI
Crypt32
libssl
libcrypto
websockets_static)

add_subdirectory(thirdparty/tinyxml2)
target_include_directories(ms_vvr PUBLIC thirdparty/tinyxml2)
target_link_libraries(ms_vvr PUBLIC thirdparty/tinyxml2/$<CONFIG>/tinyxml2)
# target_link_options(ms_vvr ignore:4099)