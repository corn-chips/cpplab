#include <Windows.h>
#include <windowengine.hpp>
#include "countryflagquiz.hpp"

using namespace cpplab;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    SetProcessDPIAware();

    const wchar_t* windowName = L"Country Flag Quiz";

    WindowEngineConfiguration conf;
    conf.Dimensions.initialWindowSize = vec2(320.f, 450.f);
    conf.AdvancedProperties.WindowCreation.resizable = false;

    WindowEngine we(hInstance, nCmdShow, windowName, conf);
    CountryFlagQuizApp app;

    try {
        we.StartWindowLoop(&app);
    } 
    catch (std::runtime_error err) {
        MessageBoxA(NULL, err.what(), "Unexpected error!", NULL);
    }

    return 0;

}