project(MOV_ONLY)
find_package(QBDI REQUIRED)

add_executable(MOV main.cpp ShadowMemory.h ShadowMemory.cpp)

target_link_libraries(MOV QBDI)