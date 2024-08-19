#include <iostream>
#include "renderer.h"
#include "universe.h"

const int width = 800;
const int height = 800;

int main() {
    Renderer& red = Renderer::getInstance();

    if (!red.initialize(width, height)) return 1;

    Universe universe(width, height);

    universe.registerStar({200, 50}, 1);
    universe.registerStar({600, 500}, 1);
    universe.registerStar({700, 150}, 1);
    universe.registerStar({50, 200}, 1);
    universe.registerStar({190, 80}, 1);
    universe.registerStar({300, 220}, 1);
    universe.registerStar({350, 680}, 1);

    GLuint imgTex;
    glGenTextures(1, &imgTex);
    glBindTexture(GL_TEXTURE_2D, imgTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    bool drawQuadBounds = false;
    int depth = -1;
    while (red.update()) {
        ImGui::Begin("Debug");
        ImGui::Checkbox("Draw quadtree bounds", &drawQuadBounds);

        if (drawQuadBounds) {
            if (ImGui::Button("^")) depth++;
            ImGui::SameLine();
            if (ImGui::Button("v")) depth--;

            if (depth < -1) depth = -1;

            ImGui::Text("Depth: %i", depth);
            ImGui::SameLine();
        }
        ImGui::End();

        snapshotConfig config = {
            {drawQuadBounds, depth}
        };

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, universe.snapshot(config));
        ImGui::GetBackgroundDrawList()->AddImage((ImTextureID) imgTex, ImVec2(0, 0), ImVec2(width, height));

        red.render();
    }

    red.cleanup();

    return 0;
}
