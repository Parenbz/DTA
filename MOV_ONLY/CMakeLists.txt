project(MOV_ONLY)

add_executable(MOV main.cpp ShadowMemory.h ShadowMemory.cpp)

target_link_libraries(MOV QBDI)