cmake_minimum_required(VERSION 3.24)
project(Asteroids)

set(CMAKE_CXX_STANDARD 17)
include_directories(include /opt/homebrew/include/SDL2)
add_compile_options(-Wall)
add_library(console-game-engine
        include/SimpleGameEngine.hpp
        include/SimpleGameEngine.cpp)
target_link_libraries(console-game-engine -lSDL2 -lSDL2_ttf)
target_link_libraries(console-game-engine -L/opt/homebrew/lib/)
add_executable(asteroids src/main.cpp)
target_link_libraries(asteroids console-game-engine)

