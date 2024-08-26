#include <iostream>
#include "renderer.h"
#include "universe.h"

const int width = 800;
const int height = 800;

int main() {
    Renderer& red = Renderer::getInstance();

    if (!red.initialize(width, height)) return 1;

    Universe* universe = new Universe(width, height, 400);
    universe->registerGalaxy({200, 200}, 3000, 10e6, {0, 0}, {1, 70});
    double r = 150;
    double v = std::sqrt(body::G * 10e6 / r);
    double a = 45.0 * 3.14159 / 180.0;
    universe->registerGalaxy({200 + r * std::cos(a), 200 - r * std::sin(a)}, 1000, 10e5, {-v * std::cos(a), -v * std::sin(a)}, {1, 40});

    GLuint imgTex;
    glGenTextures(1, &imgTex);

    glBindTexture(GL_TEXTURE_2D, imgTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    bool debug = false;
    bool drawQuadBounds = false;
    bool drawSameDepthOnly = false;
    bool run = false;
    int depth = -1;
    while (red.update()) {
        if (run) universe->step();

        ImGui::Begin("Debug");

        ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("FPS: %.1f", io.Framerate);

        ImGui::Checkbox("Run", &run);
        ImGui::Checkbox("Debug", &debug);

        if (debug) {
            ImGui::Text("Depth: %i", depth);
            ImGui::SameLine();
            if (ImGui::Button("^")) depth++;
            ImGui::SameLine();
            if (ImGui::Button("v")) depth--;

            if (depth < -1) depth = -1;

            ImGui::Checkbox("Only Show Matching Depth", &drawSameDepthOnly);

            ImGui::Checkbox("Draw Quad Bounds", &drawQuadBounds);
            if (ImGui::Button("Print all visible bodies")) {
                universe->traverse([depth] (body* b, int d) -> bool {
                    if (depth == -1 || d == depth) {
                        std::cout << (b->isLeaf() ? "Leaf" : "Internal")
                                  << " body (" << b->pos.x << ", " << b->pos.y << ") has mass "
                                  << b->mass
                                  << " with bounds ({" << b->bounds.ll.x << ", " << b->bounds.ll.y
                                  << "}, {" << b->bounds.ur.x << ", " << b->bounds.ur.y << "})"
                                  << std::endl;
                    }

                    return true;
                });
                std::cout << std::endl;
            }

            if (ImGui::Button("Check Parent")) {
                universe->traverse([] (body* b, int) -> bool {
                    for (int i = 0; i < 4; i++) {
                        if (b->children[i]) {
                            if (b->children[i]->parent != b) std::cout << "Fail" << std::endl;
                        }
                    }

                    return true;
                });

                std::cout << "Finished Check" << std::endl;
            }

            if (ImGui::Button("Step")) universe->step();

            if (ImGui::IsMousePosValid()) ImGui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
            else ImGui::Text("Mouse pos: <INVALID>");
        }
        ImGui::End();

        snapshotConfig config = {debug, depth, drawQuadBounds, drawSameDepthOnly};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, universe->snapshot(config));
        ImGui::GetBackgroundDrawList()->AddImage((void*) imgTex, ImVec2(0, 0), ImVec2(width, height));

        red.render();
    }

    delete universe;
    red.cleanup();
    glDeleteTextures(1, &imgTex);

    return 0;
}
