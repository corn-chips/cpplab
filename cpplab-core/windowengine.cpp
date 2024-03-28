#include "windowengine.hpp"
#include <Windows.h>
namespace cpplab {
    std::map<HWND, WindowData*> WindowEngine::windowDataMap = std::map<HWND, WindowData*>();
    std::mutex WindowEngine::wDMLock = std::mutex();

    LRESULT CALLBACK WindowEngine::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {

        case WM_DESTROY: {
            if (WindowEngine::windowDataMap.contains(hwnd)) {
                WindowData& currentWindowData = *WindowEngine::windowDataMap[hwnd];
                currentWindowData.windowApp->OnWindowExit();
            }
            PostQuitMessage(0);
            return 0;
        }

        case WM_PAINT: {
            WindowData& currentWindowData = *WindowEngine::windowDataMap[hwnd];
            currentWindowData.windowRenderer->Draw();
            return 0;
        }

        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            
            if (WindowEngine::windowDataMap.contains(hwnd)) {
                WindowData& currentWindowData = *WindowEngine::windowDataMap[hwnd];
                currentWindowData.windowRenderer->RebuildD2DResources();
                currentWindowData.windowSize.x = static_cast<float>(width);
                currentWindowData.windowSize.y = static_cast<float>(height);
                currentWindowData.windowApp->OnWindowResize(currentWindowData.windowSize);
                return 0;
            }
            break;
        }
        case WM_GETMINMAXINFO:
            if (WindowEngine::windowDataMap.contains(hwnd)){
                MINMAXINFO* minmaxinfo = (MINMAXINFO*)lParam;
                WindowData& cwData = *WindowEngine::windowDataMap[hwnd];
                if (cwData.clampWindowSize) {
                    minmaxinfo->ptMinTrackSize.x = static_cast<LONG>(cwData.minWindowSize.x);
                    minmaxinfo->ptMinTrackSize.y = static_cast<LONG>(cwData.minWindowSize.y);
                    minmaxinfo->ptMaxTrackSize.x = static_cast<LONG>(cwData.maxWindowSize.x);
                    minmaxinfo->ptMaxTrackSize.y = static_cast<LONG>(cwData.maxWindowSize.y);
                }
                return 0;
            }
            break;
        case WM_MOUSEMOVE:
            if (WindowEngine::windowDataMap.contains(hwnd)) {
                return WindowEngine::windowDataMap[hwnd]->OnMouseMoveEvent(hwnd, uMsg, wParam, lParam);
            }
            break;;
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_CHAR:
            if (WindowEngine::windowDataMap.contains(hwnd)) {
                return WindowEngine::windowDataMap[hwnd]->OnKBEvent(hwnd, uMsg, wParam, lParam);
            }
            break;
        }
        
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }

    WindowEngine::WindowEngine(
        HINSTANCE hInstance,
        int nCmdShow,
        const wchar_t* windowName,
        WindowEngineConfiguration conf
    ) {
        const wchar_t classname[] = L"Crypt2pWindowClass";

        WNDCLASSEXW wc = { };

        wc.cbSize = sizeof(WNDCLASSEX);
        wc.hIcon = NULL;
        wc.lpfnWndProc = &WindowEngine::WindowProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpszClassName = classname;

        RegisterClassExW(&wc);

        int posX = CW_USEDEFAULT, posY = CW_USEDEFAULT;
        if (!conf.Dimensions.enableDefaultPosition) {
            posX = static_cast<int>(conf.Dimensions.defaultWindowPosition.x);
            posY = static_cast<int>(conf.Dimensions.defaultWindowPosition.y);
        }
        DWORD exflags = NULL;
        DWORD flags = NULL;

        if (conf.Properties.alwaysOnTop) {
            exflags |= WS_EX_TOPMOST;
        }

        if (conf.AdvancedProperties.WindowCreation.border) flags |= WS_BORDER;
        if (conf.AdvancedProperties.WindowCreation.title_bar) flags |= WS_CAPTION;
        if (conf.AdvancedProperties.WindowCreation.resizable) flags |= WS_THICKFRAME;
        if (conf.AdvancedProperties.WindowCreation.showWindowMenu) flags |= WS_SYSMENU | WS_CAPTION;
        if (conf.AdvancedProperties.WindowCreation.minimizable) flags |= WS_MINIMIZEBOX;
        if (conf.AdvancedProperties.WindowCreation.maximizable) flags |= WS_MAXIMIZEBOX;
        if (conf.AdvancedProperties.WindowCreation.popupMode) flags |= WS_POPUP;
        switch (conf.AdvancedProperties.WindowCreation.WindowStartingState)
        {
        case 0: // start minimized
            flags |= WS_MINIMIZE;
            break;
        case 1: // start maximized
            flags |= WS_MAXIMIZE;
            break;
        default:
            break;
        }

        //dpi awareness
        HWND tempwindow = CreateWindowExW(
            NULL, classname, windowName, WS_DISABLED, 0, 0, 0, 0, NULL, NULL, hInstance, NULL
        );
        unsigned int dpi = GetDpiForWindow(tempwindow);
        /*DestroyWindow(tempwindow);*/

        constexpr float defaultDPI = 96.f;

        this->windowHandle = CreateWindowExW(
            exflags,
            classname,
            windowName,
            flags,

            // Size and position
            //CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            posX, posY, 
            static_cast<int>((dpi * conf.Dimensions.initialWindowSize.x)/defaultDPI),
            static_cast<int>((dpi * conf.Dimensions.initialWindowSize.y)/defaultDPI),

            NULL,       // Parent window    
            NULL,       // Menu
            hInstance,  // Instance handle
            NULL        // Additional application data
        );
        if (this->windowHandle == NULL) {
            //error
            throw std::runtime_error("Window Creation created NULL handle!");
        }

        this->nCmdShow = nCmdShow;

        WindowEngine::wDMLock.lock();
        WindowEngine::windowDataMap.insert({ {this->windowHandle, new WindowData() } });
        RECT rc = {};
        GetClientRect(this->windowHandle, &rc);
        WindowEngine::windowDataMap[this->windowHandle]->windowSize.x = static_cast<float>(rc.right - rc.left);
        WindowEngine::windowDataMap[this->windowHandle]->windowSize.y = static_cast<float>(rc.bottom - rc.top);
        WindowEngine::windowDataMap[this->windowHandle]->windowDPI = dpi;
        if (conf.Dimensions.enableClamp) {
            WindowEngine::windowDataMap[this->windowHandle]->clampWindowSize = true;
            WindowEngine::windowDataMap[this->windowHandle]->maxWindowSize = conf.Dimensions.maxWindowSize;
            WindowEngine::windowDataMap[this->windowHandle]->minWindowSize = conf.Dimensions.minWindowSize;
        }

        WindowEngine::wDMLock.unlock();
    }
    WindowEngine::~WindowEngine() {
        std::lock_guard<std::mutex> guard(WindowEngine::wDMLock);
        WindowEngine::windowDataMap[this->windowHandle]->OnDestroy();
        delete WindowEngine::windowDataMap[this->windowHandle];
    }

    void WindowEngine::StartWindowLoop(ICpplabApp* app) {
        if (this->windowHandle == NULL) {
            throw std::runtime_error("Attempted to start WindowLoop with a NULL window handle!");
        }

        //do window init stuff here for some odd reason
        //no try catch needed, just let error go up the call stack
        WindowEngine::wDMLock.lock();
        WindowEngine::windowDataMap[this->windowHandle]->OnInitialize(
            this->windowHandle, app
        );
        this->wDMLock.unlock();

        app->OnWindowEntry(this);
        ShowWindow(this->windowHandle, this->nCmdShow);

        MSG msg = { };

        while (GetMessage(&msg, NULL, 0, 0) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            //force redraw on node modify
            if (WindowEngine::windowDataMap[this->windowHandle]->windowRenderer->CheckNodeModification()) {
                InvalidateRect(this->windowHandle, NULL, NULL);
            }
        }
    }


    ElementNode* WindowEngine::GetRootNode() {
        std::lock_guard<std::mutex> guard(WindowEngine::wDMLock);
        return WindowEngine::windowDataMap[this->windowHandle]->rootWindowNode;
    }
    void WindowEngine::SetBackgroundColor(Color c) {
        std::lock_guard<std::mutex> guard(WindowEngine::wDMLock);
        if(WindowEngine::windowDataMap[this->windowHandle]->windowRenderer != nullptr)
            WindowEngine::windowDataMap[this->windowHandle]->windowRenderer->SetBackgroundColor(c);
    }
    void WindowEngine::SetWindowClampSize(vec2 maxSize, vec2 minSize, bool clampEnabled ) {
        std::lock_guard<std::mutex> guard(WindowEngine::wDMLock);
        WindowData& wdata = *WindowEngine::windowDataMap[this->windowHandle];
        if (!clampEnabled) {
            wdata.clampWindowSize = false;
            return;
        }

        wdata.maxWindowSize = maxSize;
        wdata.minWindowSize = minSize;

        return;
    }
    vec2 WindowEngine::GetWindowSize() {
        std::lock_guard<std::mutex> guard(WindowEngine::wDMLock);
        if (WindowEngine::windowDataMap[this->windowHandle]->windowRenderer != nullptr) {
            return WindowEngine::windowDataMap[this->windowHandle]->windowSize;
        }
        else return vec2(0.f, 0.f);
    }
    bool WindowEngine::GetKeyPressedState(uint32_t vKey)
    {
        SHORT state = GetKeyState(vKey);
        return (0x8000 & state) == 0x8000; //high order bit
    }
}