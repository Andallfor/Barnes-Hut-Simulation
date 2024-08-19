#include <iostream>
#include "renderer.h"

int main() {
    Renderer& red = Renderer::getInstance();

    if (!red.initialize()) return 1;

    bool showDemoWindow = true;
    while (red.update()) {
        if (showDemoWindow) ImGui::ShowDemoWindow(&showDemoWindow);

        red.render();
    }

    red.cleanup();

    return 0;
}
