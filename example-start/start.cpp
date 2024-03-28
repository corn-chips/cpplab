#include <Windows.h>
#include <windowengine.hpp>

using namespace cpplab;

class StartApp : public ICpplabApp {
	// Inherited via ICpplabApp

	TextElement* textbox;

	void OnWindowEntry(WindowEngine* engine) override {
		
		vec2 windowsize = engine->GetWindowSize();
		engine->SetBackgroundColor(Color(1.0f, 1.0f));

		TextElementConfiguration txtConf;
		txtConf.setBackgroundColor(Color(0.95f, 1.0f))
			.setBorderColor(Color(0.0f))
			.setBorderWidth(5.f)
			.setRoundedEdgeRadius(5.f)
			.setTextMarginPx(5.f)
			.setText(L"Example")
			.setTextMarginPx(10.f)
			.setTextHorizontalAlignment(HAlign::CENTER)
			.setTextVerticalAlignment(VAlign::CENTER)
			.setTextSize(24.f)
			.SetDimension(vec2(200.f, 150.f))
			.SetHidden(false);

		this->textbox = new TextElement(txtConf);
		this->textbox->setRelativePosition(vec2(
			(windowsize.x/2) - (this->textbox->getDimensions().x/2), (windowsize.y / 2) - (this->textbox->getDimensions().y / 2)
		));
		this->textbox->show();
		engine->GetRootNode()->AddChildElement("element", this->textbox);
	}

	void OnWindowExit() override {}
	void OnWindowResize(vec2 newSize) override {
		this->textbox->setRelativePosition(vec2(
			(newSize.x / 2) - (this->textbox->getDimensions().x / 2), (newSize.y / 2) - (this->textbox->getDimensions().y / 2)
		));
	}
	void OnGlobalMouseMove(vec2 pos) override {}
	void OnGlobalMouseDown(vec2 pos, IMouseEventListener::Button button) override {}
	void OnGlobalMouseUp(vec2 pos, IMouseEventListener::Button button) override {}
	void OnKeyDown(uint32_t vKey) override {}
	void OnKeyUp(uint32_t vKey) override {}
	void OnCharacterInput(wchar_t wchar) override {}
};


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	SetProcessDPIAware(); // important!!

	const wchar_t* windowName = L"Start window";

	WindowEngineConfiguration conf;
	conf.Dimensions.enableClamp = true;
	conf.Dimensions.minWindowSize = vec2(400.f, 400.f);
	conf.Dimensions.maxWindowSize = vec2(1080.f, 1080.f);

	WindowEngine we(hInstance, nCmdShow, windowName, conf);
	StartApp app{};

	try {
		we.StartWindowLoop(&app);
	}
	catch (std::runtime_error err) {
		MessageBoxA(NULL, err.what(), "Unexpected error!", NULL);
	}

	return 0;
}