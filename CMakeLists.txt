cmake_minimum_required (VERSION 3.8)
project ("asio-test")
set(ASIO asio-1.20.0)
set(GLEW glew-2.2.0)
set(GLFW glfw-3.3.5)
set(IMGUI imgui-1.85)

add_subdirectory(${PROJECT_SOURCE_DIR}/${GLEW}/build/cmake EXCLUDE_FROM_ALL)
add_subdirectory(${PROJECT_SOURCE_DIR}/${GLFW} EXCLUDE_FROM_ALL)

add_executable (asio-test "main.cpp" "client.h" "message_header.h" "server.h" "cbor11.h" "cbor11.cpp" ${IMGUI}/imconfig.h ${IMGUI}/imgui.cpp ${IMGUI}/imgui.h ${IMGUI}/imgui_draw.cpp ${IMGUI}/imgui_internal.h ${IMGUI}/imgui_tables.cpp ${IMGUI}/imgui_widgets.cpp ${IMGUI}/imstb_rectpack.h ${IMGUI}/imstb_textedit.h ${IMGUI}/imstb_truetype.h ${IMGUI}/backends/imgui_impl_glfw.cpp ${IMGUI}/backends/imgui_impl_glfw.h ${IMGUI}/backends/imgui_impl_opengl2.cpp ${IMGUI}/backends/imgui_impl_opengl2.h)
set_target_properties(asio-test PROPERTIES CXX_STANDARD 20)
add_library(asio INTERFACE)

IF (WIN32)
    target_link_options(asio-test PRIVATE "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
ENDIF()

if(MSVC)
    target_compile_options(asio-test PUBLIC "/Zc:__cplusplus")
endif()

set(BUILD_UTILS OFF CACHE BOOL "" FORCE)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

target_link_libraries(asio-test PRIVATE asio glew_s glfw)
target_include_directories(asio-test PRIVATE ${PROJECT_SOURCE_DIR}/${ASIO}/include)

target_include_directories(asio-test PRIVATE
    ${PROJECT_SOURCE_DIR}/${GLEW}/include
)

target_include_directories(asio-test PRIVATE
    ${PROJECT_SOURCE_DIR}/${IMGUI}/
)
