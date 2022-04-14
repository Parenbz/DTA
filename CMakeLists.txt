project(test)

add_executable(main main.cpp ShadowMemory.h ShadowMemory.cpp)

target_link_libraries(main QBDI)