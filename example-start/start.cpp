#include <Windows.h>
#include <windowengine.hpp>

#include <random>

using namespace cpplab;

class StartApp : public ICpplabApp {
	// Inherited via ICpplabApp

	TextElementConfiguration txtconf;
	std::vector<TextElement*> points;
	ElementNode* rootnode;

	void OnWindowEntry(WindowEngine* engine) override {
		const int pointsnum = 16;
		rootnode = engine->GetRootNode();
		
		std::random_device rd;
		std::mt19937 rng(rd());
		std::uniform_real_distribution<> distr(0.0f, 1.0f);

		txtconf.backgroundColor = Color(1.f, 0.f, 0.f);
		txtconf.borderColor = Color(0.0f);
		txtconf.borderWidthPx = 5.f;
		txtconf.dimensions = vec2(20.f, 20.f);
		txtconf.hidden = false;
		txtconf.roundedEdgePx = 5.f;
		txtconf.startingText = std::wstring();

		for (int i = 0; i < pointsnum; i++) {
			TextElement* point = new TextElement(txtconf);
			points.push_back(point);
			vec2 randpos(distr(rng), distr(rng));
			vec2 windowsize = engine->GetWindowSize();
			randpos = vec2(randpos.x * windowsize.x, randpos.y * windowsize.y);
			point->setRelativePosition(randpos);
			engine->GetRootNode()->AddChildElement(std::string("point") + std::to_string(i), point);
		}
	}

	void OnWindowExit() override {}
	void OnWindowResize(vec2 newSize) override {
		
	}
	void OnGlobalMouseMove(vec2 pos) override {}
	void OnGlobalMouseDown(vec2 pos, IMouseEventListener::Button button) override {}
	void OnGlobalMouseUp(vec2 pos, IMouseEventListener::Button button) override {}
	void OnKeyDown(uint32_t vKey) override {
		if (vKey == VK_SPACE) {
			for (const std::string& id : rootnode->GetAllChildElements()) {
				rootnode->DeleteChildElement(id);
			}
			std::vector<TextElement*> newPoints;
			while (points.size() > 1) {
				TextElement* currentElement = points[0];
				int closestElementidx = 1;
				for (int j = 2; j < points.size(); j++) {
					auto Squared = [](float val) -> float { return val * val; };
					auto ElementDistance = [Squared](vec2 posA, vec2 posB) -> float {
						return sqrtf(Squared(posB.x - posA.x) + Squared(posB.y - posA.y));
					};

					float closestElementDistance = ElementDistance(currentElement->getRelativePosition(), points[closestElementidx]->getRelativePosition());
					float otherElementDistance = ElementDistance(currentElement->getRelativePosition(), points[j]->getRelativePosition());
					if (otherElementDistance < closestElementDistance) {
						closestElementidx = j;
					}
				}

				TextElement* newMidElement = new TextElement(txtconf);

				//find midpoint
				vec2 midpoint(
					(currentElement->getRelativePosition().x + points[closestElementidx]->getRelativePosition().x) / 2.f,
					(currentElement->getRelativePosition().y + points[closestElementidx]->getRelativePosition().y) / 2.f
				);
				newMidElement->setRelativePosition(midpoint);
				newPoints.push_back(newMidElement);

				delete points[0];
				delete points[closestElementidx];

				points.erase(points.begin() + closestElementidx);
				points.erase(points.begin());
			}
			for (int i = 0; i < newPoints.size(); i++) {
				rootnode->AddChildElement(std::string("point") + std::to_string(i), newPoints[i]);
			}
			for (auto& a : newPoints) {
				this->points.push_back(a);
			}
		}
	}
	void OnKeyUp(uint32_t vKey) override {}
	void OnCharacterInput(wchar_t wchar) override {}
};


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	SetProcessDPIAware(); // important!!

	const wchar_t* windowName = L"Start window";

	WindowEngineConfiguration conf;
	conf.Dimensions.initialWindowSize = vec2(500.f, 500.f);
	conf.AdvancedProperties.WindowCreation.resizable = false;

	WindowEngine we(hInstance, nCmdShow, windowName, conf);
	StartApp app{};

	try {
		we.StartWindowLoop(&app);
	}
	catch (std::runtime_error err) {
		MessageBoxA(NULL, err.what(), "Unexpected error!", NULL);
	}

	return 0;-
}