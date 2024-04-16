#include <Windows.h>
#include <windowengine.hpp>
#include "countryflagquiz.hpp"
#include <csv-reader.hpp>

using namespace cpplab;

//https://studio.code.org/projects/applab/NpAI-L7GyKbs8kBRxxseMP_1U1qPUxgiyXxZILWtBuQ/edit

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    SetProcessDPIAware();


    const wchar_t* windowName = L"Country Flag Quiz";

    WindowEngineConfiguration conf;
    conf.Dimensions.initialWindowSize = vec2(335.f, 480.f);
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