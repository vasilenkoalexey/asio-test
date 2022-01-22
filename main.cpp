
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>

#include "asio.hpp"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl2.h"
#include "cbor11.h"
#include "client.h"
#include "imgui.h"
#include "map.h"
#include "server.h"
#include "unit.h"

static void glfw_error_callback(int error, const char *description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(int argc, char *argv[]) {
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        return 1;
    }

    GLFWwindow *window = glfwCreateWindow(1280, 720, "asio-test", NULL, NULL);
    if (window == NULL) {
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Enable vsync

    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    asio::io_context io_context0;
    asio::ip::tcp::endpoint endpoint(
        asio::ip::address::from_string("127.0.0.1"), 3000);
    network::Server server(io_context0, endpoint);

    const auto t = std::make_shared<unit_type>();
    map world(100, 100);
    unit unit1(world.get(3, 5), .300f, t);
    unit unit2(world.get(30, 5), .300f, t);

    unit1.move_to(world.get(1, 1));
    unit2.move_to(world.get(3, 3));

    cbor::array delta = cbor::array{};

    asio::io_context io_context1;
    asio::ip::tcp::resolver resolver(io_context1);
    auto endpoints = resolver.resolve(endpoint);
    network::Client client(io_context1, endpoints,
                           [&](const uint32_t opcode, const std::string &body) {
                               std::cerr << "Message: " << body << "\n";
                           });

    client.write_msg(0, "test");

    while (!glfwWindowShouldClose(window)) {

        // update game state
        unit1.update(delta);
        unit2.update(delta);

        // send state changes to clients
        if (!delta.empty()) {
            server.broadcast(0, delta);
            delta.clear();
        }

        // poll asyn io events on server
        server.poll();

        // poll asyn io events on client
        client.poll();

        // poll keyboard/mouse events
        glfwPollEvents();

        // render
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (ImGui::Begin("Dear ImGui Metrics/Debugger")) {
            // Basic info
            ImGui::Text("Dear ImGui %s", ImGui::GetVersion());
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                        1000.0f / io.Framerate, io.Framerate);
            ImGui::Text("%d vertices, %d indices (%d triangles)",
                        io.MetricsRenderVertices, io.MetricsRenderIndices,
                        io.MetricsRenderIndices / 3);
            ImGui::Text("%d visible windows, %d active allocations",
                        io.MetricsRenderWindows, io.MetricsActiveAllocations);

            ImGui::End();
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        const static ImVec4 clear_color = ImVec4(.45f, .55f, .6f, 1.f);
        glClearColor(clear_color.x * clear_color.w,
                     clear_color.y * clear_color.w,
                     clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}