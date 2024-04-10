#include <windowengine.hpp>

using namespace cpplab;

class CountryFlagQuizApp : public cpplab::ICpplabApp {
private:

public:
    // Inherited via ICpplabApp
    void OnWindowEntry(WindowEngine* engine) override;

    void OnWindowExit() override;

    void OnWindowResize(vec2 newSize) override;

    void OnGlobalMouseMove(vec2 pos) override;

    void OnGlobalMouseDown(vec2 pos, IMouseEventListener::Button button) override;

    void OnGlobalMouseUp(vec2 pos, IMouseEventListener::Button button) override;

    void OnKeyDown(uint32_t vKey) override;

    void OnKeyUp(uint32_t vKey) override;

    void OnCharacterInput(wchar_t wchar) override;

};