
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 23)

# Добавьте источник в исполняемый файл этого проекта.
add_executable (visual_cryptograpy "main.cpp" "encrypt.h" "decrypt.h" "decrypt.cpp" "encrypt.cpp")

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
find_package(OpenCV REQUIRED)

target_link_libraries(visual_cryptograpy ${OpenCV_LIBS})