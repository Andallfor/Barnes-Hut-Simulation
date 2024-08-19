#include <d3d12.h>
#include <dxgi1_4.h>
#include <iostream>
#include "renderer.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "imgui_internal.h"

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
