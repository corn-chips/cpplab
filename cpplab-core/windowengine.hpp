#pragma once
#ifndef UNICODE
#define UNICODE
#endif 
#include <string>
#include <Windows.h>
#include <map>
#include <mutex>
#include <vector>
#include "windowrenderer.hpp"

namespace cpplab {
    //todo:
    // high priority:
    // add keyboard events: DONE
    // make some sort of reusable config struct for elements: DONE
    // image rendering DONE
    // set window icon and title at runtime
    // debug window
    // 
    // low priority:
    // improve render efficiency
    // window hiearchy/inheritance
    // add more primitive render types
    // animation engine
    // multithreadable: task based? or std::async and promises + futures
    // development toolkit: seperate app that helps with design
    // script language?

    class WindowEngine;

    enum VKeyboardMappings {
        KEY_0 = 0x30,
        KEY_1 = 0x31,
        KEY_2 = 0x32,
        KEY_3 = 0x33,
        KEY_4 = 0x34,
        KEY_5 = 0x35,
        KEY_6 = 0x36,
        KEY_7 = 0x37,
        KEY_8 = 0x38,
        KEY_9 = 0x39,

        // Keys A - Z
        KEY_A = 0x41,
        KEY_B = 0x42,
        KEY_C = 0x43,
        KEY_D = 0x44,
        KEY_E = 0x45,
        KEY_F = 0x46,
        KEY_G = 0x47,
        KEY_H = 0x48,
        KEY_I = 0x49,
        KEY_J = 0x4A,
        KEY_K = 0x4B,
        KEY_L = 0x4C,
        KEY_M = 0x4D,
        KEY_N = 0x4E,
        KEY_O = 0x4F,
        KEY_P = 0x50,
        KEY_Q = 0x51,
        KEY_R = 0x52,
        KEY_S = 0x53,
        KEY_T = 0x54,
        KEY_U = 0x55,
        KEY_V = 0x56,
        KEY_W = 0x57,
        KEY_X = 0x58,
        KEY_Y = 0x59,
        KEY_Z = 0x5A
    };

    class ICpplabApp {
    public:
        virtual void OnWindowEntry(WindowEngine* engine) = 0;
        virtual void OnWindowExit() = 0;

        virtual void OnWindowResize(vec2 newSize) = 0;

        virtual void OnGlobalMouseMove(vec2 pos) = 0;
        virtual void OnGlobalMouseDown(vec2 pos, IMouseEventListener::Button button) = 0;
        virtual void OnGlobalMouseUp(vec2 pos, IMouseEventListener::Button button) = 0;

        virtual void OnKeyDown(uint32_t vKey) = 0;
        virtual void OnKeyUp(uint32_t vKey) = 0;
        virtual void OnCharacterInput(wchar_t wchar) = 0;
    };

    class WindowData {
    public:
        ElementNode* rootWindowNode = nullptr;
        WindowD2DRenderer* windowRenderer = nullptr;
        ICpplabApp* windowApp = nullptr;
        unsigned int windowDPI = 0;

        //setup window instance data
        // mouse position
        // global click events
        // global keyboard events
        // window size
        vec2 windowSize = vec2(0.f, 0.f);

        //min and max window sizes
        bool clampWindowSize = false;
        vec2 maxWindowSize = vec2(0.f, 0.f), minWindowSize = vec2(0.f, 0.f);

        void OnInitialize(HWND windowHWND, ICpplabApp* app);
        void OnDestroy();
        
        uint8_t mouseState;
        vec2 mousePosition  = vec2(0.f, 0.f);

        struct SetDepthComparator {
            bool operator() (const ElementNode* rhs, const ElementNode* lhs) const {
                if (rhs != nullptr && lhs != nullptr) {
                    if (rhs->depth == lhs->depth) {
                        return reinterpret_cast<long>(rhs) > reinterpret_cast<long>(lhs);
                    }
                    return rhs->depth > lhs->depth;
                }
                return false;
            }
        };
        std::set<ElementNode*, SetDepthComparator> hoveredElements;

        LRESULT OnMouseMoveEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        LRESULT OnKBEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        void ProcessElementMouseEvents(uint8_t prevMouseState, vec2 prevMousePosition);
    };

    struct WindowEngineConfiguration {
        struct {
            vec2 initialWindowSize = vec2(640.f, 480.f);

            bool enableClamp = false;
            vec2 maxWindowSize = vec2(0.f, 0.f), minWindowSize = vec2(0.f, 0.f);

            bool enableDefaultPosition = false;
            vec2 defaultWindowPosition = vec2(0.f, 0.f);
        } Dimensions;
        struct {
            bool alwaysOnTop = false;
        } Properties;
        struct {
            struct {
                bool border = true;
                bool title_bar = true;
                bool resizable = true;
                bool showWindowMenu = true;
                bool minimizable = true;
                bool maximizable = true;
                enum StartingState {
                    MINIMIZED, MAXIMIZED, DEFAULT
                } WindowStartingState = DEFAULT;

                bool popupMode = false;
            } WindowCreation;
        } AdvancedProperties;
    };

    class WindowEngine {
    private:
        HWND windowHandle;
        int nCmdShow;

        static std::mutex wDMLock;
        static std::map<HWND, WindowData*> windowDataMap;

        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    public:
        WindowEngine(
            HINSTANCE hInstance,
            int nCmdShow,
            const wchar_t* windowName,
            WindowEngineConfiguration conf
        );
        ~WindowEngine();

        void StartWindowLoop(ICpplabApp* app);

        ElementNode* GetRootNode();

        void SetBackgroundColor(Color c);
        void SetWindowClampSize(vec2 maxSize, vec2 minSize, bool clampEnabled = true);
        vec2 GetWindowSize();
        bool GetKeyPressedState(uint32_t vKey);
    };
}