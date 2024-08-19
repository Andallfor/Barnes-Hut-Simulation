#include <iostream>
#include "renderer.h"

int main() {
    Renderer& red = Renderer::getInstance();

    if (!red.initialize()) return 1;

    const int width = 256;
    const int height = 256;

    GLubyte buffer[height][width][3];
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < height; j++) {
            int c = ((((i & 0x8) == 0) ^ ((j & 0x8)) == 0)) * 255;
            buffer[i][j][0] = (GLubyte) c;
            buffer[i][j][1] = (GLubyte) c;
            buffer[i][j][2] = (GLubyte) c;
        }
    }

    GLuint imgTex;
    glGenTextures(1, &imgTex);
    glBindTexture(GL_TEXTURE_2D, imgTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);

    bool showDemoWindow = true;
    while (red.update()) {
        //if (showDemoWindow) ImGui::ShowDemoWindow(&showDemoWindow);

        ImGui::GetBackgroundDrawList()->AddImage((ImTextureID) imgTex, ImVec2(0, 0), ImVec2(width, height));

        red.render();
    }

    red.cleanup();

    return 0;
}
