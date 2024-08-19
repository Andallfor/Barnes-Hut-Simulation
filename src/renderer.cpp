#include "renderer.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include <tchar.h>

// see https://github.com/ocornut/imgui/tree/master/examples/example_win32_opengl3

bool Renderer::initialize() {
    wc = { sizeof(wc), CS_OWNDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"main", nullptr };
    ::RegisterClassExW(&wc);
    winHandle = ::CreateWindowW(wc.lpszClassName, L"Barnes Hut Simulation", WS_OVERLAPPEDWINDOW, 100, 100, 800, 800, nullptr, nullptr, wc.hInstance, nullptr);

    if (!createDevice(winHandle, &g_mainWindow)) {
        cleanupDevice(winHandle, &g_mainWindow);
        ::DestroyWindow(winHandle);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return false;
    }

    wglMakeCurrent(g_mainWindow.hDC, g_hRC);

    // Show the window
    ::ShowWindow(winHandle, SW_SHOWDEFAULT);
    ::UpdateWindow(winHandle);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplWin32_InitForOpenGL(winHandle);
    ImGui_ImplOpenGL3_Init();

    return true;
}

bool Renderer::update() {
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
        if (msg.message == WM_QUIT) return false;
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    return true;
}

void Renderer::render() {
    // Rendering
    ImGui::Render();
    glViewport(0, 0, g_width, g_height);
    glClearColor(0.45f, 0.55f, 0.6f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Present
    ::SwapBuffers(g_mainWindow.hDC);
}

void Renderer::cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    cleanupDevice(winHandle, &g_mainWindow);
    wglDeleteContext(g_hRC);
    ::DestroyWindow(winHandle);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
}

bool Renderer::createDevice(HWND handle, WGL_WindowData* data) {
    HDC hDc = ::GetDC(handle);
    PIXELFORMATDESCRIPTOR pfd = { 0 };
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    const int pf = ::ChoosePixelFormat(hDc, &pfd);
    if (pf == 0) return false;
    if (::SetPixelFormat(hDc, pf, &pfd) == FALSE) return false;
    ::ReleaseDC(handle, hDc);

    data->hDC = ::GetDC(handle);
    if (!g_hRC) g_hRC = wglCreateContext(data->hDC);
    return true;
}

void Renderer::cleanupDevice(HWND handle, WGL_WindowData* data) {
    wglMakeCurrent(nullptr, nullptr);
    ::ReleaseDC(handle, data->hDC);
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;

    switch (msg) {
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED) {
            Renderer::getInstance().g_width = LOWORD(lParam);
            Renderer::getInstance().g_height = HIWORD(lParam);
        }
        return 0;
    case WM_SYSCOMMAND:
        // Disable ALT application menu
        if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }

    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
