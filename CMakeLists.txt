
cmake_minimum_required (VERSION 3.8)

# Добавьте источник в исполняемый файл этого проекта.
add_executable (Course_paper "Course_paper.cpp" "Course_paper.h")

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
find_package(OpenCV REQUIRED)

target_link_libraries(Course_paper ${OpenCV_LIBS})