#ifndef RENDERER_H
#define RENDERER_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#include <windows.h>
#include <GL/GL.h>
#include <tchar.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// """"adapated"""" from https://github.com/ocornut/imgui/tree/master/examples/example_win32_opengl3
class Renderer {
private:
    Renderer() {}

    struct WGL_WindowData { HDC hDC; };

    HGLRC g_hRC;
    WGL_WindowData g_mainWindow;
    int g_width, g_height;

    WNDCLASSEXW wc;
    HWND winHandle;

    bool createDevice(HWND handle, WGL_WindowData* data);
    void cleanupDevice(HWND handle, WGL_WindowData* data);

    friend LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
    // https://stackoverflow.com/questions/1008019/how-do-you-implement-the-singleton-design-pattern
    static Renderer& getInstance() {
        static Renderer instance;
        return instance;
    }

    Renderer(Renderer const &) = delete;
    Renderer& operator=(const Renderer &) = delete;

    bool initialize();
    bool update();
    void render();
    void cleanup();
};

#endif
