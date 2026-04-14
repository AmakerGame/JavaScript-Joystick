cmake_minimum_required(VERSION 3.15)
project(PS_Controller_Link)

set(CMAKE_CXX_STANDARD 17)

find_package(hidapi REQUIRED)
find_package(httplib REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(ControllerApp src/main.cpp src/controller.cpp)

# Додаємо бібліотеки Windows для трею та прибираємо консоль
if(WIN32)
    target_link_libraries(ControllerApp PRIVATE shell32 user32 gdi32)
    set_target_properties(ControllerApp PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

target_link_libraries(ControllerApp PRIVATE 
    hidapi::hidapi 
    httplib::httplib 
    nlohmann_json::nlohmann_json
)
