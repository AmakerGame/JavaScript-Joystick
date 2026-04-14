cmake_minimum_required(VERSION 3.15)
project(PS_Controller_Link)

set(CMAKE_CXX_STANDARD 17)

find_package(hidapi REQUIRED)
find_package(httplib REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(ControllerApp src/main.cpp src/controller.cpp)

if(WIN32)
    # Називаємо файл old.exe для сумісності з Win7
    set_target_properties(ControllerApp PROPERTIES OUTPUT_NAME "old")
    set_target_properties(ControllerApp PROPERTIES WIN32_EXECUTABLE TRUE)
    target_link_libraries(ControllerApp PRIVATE shell32 user32 gdi32 ws2_32)
    add_definitions(-D_WIN32_WINNT=0x0601)
elseif(APPLE)
    set_target_properties(ControllerApp PROPERTIES OUTPUT_NAME "Controller_MacOS")
else()
    set_target_properties(ControllerApp PROPERTIES OUTPUT_NAME "Controller_Linux")
endif()

target_link_libraries(ControllerApp PRIVATE 
    hidapi::hidapi httplib::httplib nlohmann_json::nlohmann_json
)
