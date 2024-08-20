#include <iostream>
#include "renderer.h"
#include "universe.h"

const int width = 800;
const int height = 800;

int main() {
    Renderer& red = Renderer::getInstance();

    if (!red.initialize(width, height)) return 1;

    Universe universe(width, height);

    universe.registerStar({200, 50}, 100);
    universe.registerStar({600, 500}, 100);
    universe.registerStar({700, 150}, 100);
    universe.registerStar({50, 200}, 100);
    universe.registerStar({190, 80}, 100);
    universe.registerStar({300, 220}, 10000000);
    universe.registerStar({350, 680}, 100);

    GLuint imgTex;
    glGenTextures(1, &imgTex);
    glBindTexture(GL_TEXTURE_2D, imgTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    bool debug = false;
    bool drawQuadBounds = false;
    bool drawSameDepthOnly = false;
    int depth = -1;
    while (red.update()) {
        ImGui::Begin("Debug");
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
                universe.traverse([depth] (body* b, int d) -> bool {
                    if (depth == -1 || d == depth) {
                        std::cout << (b->isLeaf() ? "Leaf" : "External") << " body (" << b->pos.x << ", " << b->pos.y << ") has mass " << b->mass << std::endl;
                    }

                    return true;
                });
                std::cout << std::endl;
            }

            if (ImGui::Button("Step")) universe.step();

            ImGuiIO& io = ImGui::GetIO();
            if (ImGui::IsMousePosValid()) ImGui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
            else ImGui::Text("Mouse pos: <INVALID>");
        }
        ImGui::End();

        snapshotConfig config = {debug, depth, drawQuadBounds, drawSameDepthOnly};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, universe.snapshot(config));
        ImGui::GetBackgroundDrawList()->AddImage((ImTextureID) imgTex, ImVec2(0, 0), ImVec2(width, height));

        red.render();
    }

    red.cleanup();

    return 0;
}
