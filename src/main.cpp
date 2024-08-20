#include <iostream>
#include "renderer.h"
#include "universe.h"

const int width = 800;
const int height = 800;

int main() {
    Renderer& red = Renderer::getInstance();

    if (!red.initialize(width, height)) return 1;

    Universe universe(width, height, 400);
    universe.registerStar({230, 230}, 1, {std::sqrt(body::G * 50000 / 20), 0});
    universe.registerStar({230, 210}, 50000, {0, 0});

    //universe.registerStar({190, 210}, 25000, {0, -1}); // remove for stable orbit

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
        if (run) universe.step();

        ImGui::Begin("Debug");
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
