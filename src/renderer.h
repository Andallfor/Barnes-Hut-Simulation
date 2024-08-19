#ifndef RENDERER_H
#define RENDERER_H

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// """"adapated"""" from https://github.com/ocornut/imgui/tree/master/examples/example_win32_directx12
class Renderer {
private:
    struct FrameContext {
        ID3D12CommandAllocator* CommandAllocator;
        UINT64                  FenceValue;
    };

    static int const             NUM_FRAMES_IN_FLIGHT = 3;
    static int const             NUM_BACK_BUFFERS = 3;

    FrameContext                 g_frameContext[NUM_FRAMES_IN_FLIGHT] = {};
    UINT                         g_frameIndex = 0;
    ID3D12Device*                g_pd3dDevice = nullptr;
    ID3D12DescriptorHeap*        g_pd3dRtvDescHeap = nullptr;
    ID3D12DescriptorHeap*        g_pd3dSrvDescHeap = nullptr;
    ID3D12CommandQueue*          g_pd3dCommandQueue = nullptr;
    ID3D12GraphicsCommandList*   g_pd3dCommandList = nullptr;
    ID3D12Fence*                 g_fence = nullptr;
    HANDLE                       g_fenceEvent = nullptr;
    UINT64                       g_fenceLastSignaledValue = 0;
    IDXGISwapChain3*             g_pSwapChain = nullptr;
    bool                         g_SwapChainOccluded = false;
    HANDLE                       g_hSwapChainWaitableObject = nullptr;
    ID3D12Resource*              g_mainRenderTargetResource[NUM_BACK_BUFFERS] = {};
    D3D12_CPU_DESCRIPTOR_HANDLE  g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};

    WNDCLASSEXW wc;
    HWND hwnd;

    Renderer() {}

    bool createDeviceD3D(HWND hWnd);
    void cleanupDeviceD3D();
    void createRenderTarget();
    void cleanupRenderTarget();
    void waitForFrame();
    FrameContext* waitForNextFrameResources();

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
