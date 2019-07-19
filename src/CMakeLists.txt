set(EXECUTABLE_SRC_LIST "main.c")
set(SHARED_SRC_LIST "common.c")

add_subdirectory(3rdparty/VelocityRaptor/VelocityRaptor)
include_directories(3rdparty/VelocityRaptor/VelocityRaptor/include)

include(libsuperderpy-src)

target_link_libraries(libbob VelocityRaptor)
