cmake_minimum_required(VERSION 3.15)
project(ControllerLink)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

FetchContent_Declare(httplib GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git GIT_TAG v0.14.3)
FetchContent_MakeAvailable(httplib)

add_executable(ControllerApp src/main.cpp src/controller.cpp)

if(WIN32)
    add_definitions(-DNOMINMAX -D_WIN32_WINNT=0x0601)
    set_target_properties(ControllerApp PROPERTIES OUTPUT_NAME "old" WIN32_EXECUTABLE TRUE)
    find_package(hidapi REQUIRED)
    target_link_libraries(ControllerApp PRIVATE hidapi::hidapi ws2_32 shell32 user32 gdi32)
elseif(APPLE)
    set_target_properties(ControllerApp PROPERTIES OUTPUT_NAME "Controller_MacOS")
    find_library(HIDAPI_LIB hidapi)
    target_link_libraries(ControllerApp PRIVATE ${HIDAPI_LIB})
else()
    set_target_properties(ControllerApp PROPERTIES OUTPUT_NAME "Controller_Linux")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(HIDAPI REQUIRED hidapi-libusb)
    target_link_libraries(ControllerApp PRIVATE ${HIDAPI_LIBRARIES})
    target_include_directories(ControllerApp PRIVATE ${HIDAPI_INCLUDE_DIRS})
endif()

target_link_libraries(ControllerApp PRIVATE nlohmann_json::nlohmann_json httplib::httplib)
