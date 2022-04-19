project(ADD)

add_executable(ADD main.cpp ShadowMemory.h ShadowMemory.cpp)

target_link_libraries(ADD QBDI)