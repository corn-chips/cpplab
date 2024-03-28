#include "windowengine.hpp"
#include <functional>
#include <algorithm>

namespace cpplab {

    void WindowData::OnInitialize(HWND windowHWND, ICpplabApp* app) {
        this->rootWindowNode = new ElementNode();
        this->windowApp = app;
        this->windowRenderer = new WindowD2DRenderer(windowHWND, this->rootWindowNode, this->windowDPI);
    }
    void WindowData::OnDestroy() {
        delete this->rootWindowNode;
        delete this->windowRenderer;
    }

    LRESULT WindowData::OnMouseMoveEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        //set mouse state
        uint8_t prevMouseState = this->mouseState;
        this->mouseState = wParam & 0xff;
        
        vec2 prevMousePosition = vec2(this->mousePosition);
        this->mousePosition.x = static_cast<float>(lParam & 0x0000ffff);
        this->mousePosition.y = static_cast<float>((lParam & 0xffff0000) >> 16);

        this->ProcessElementMouseEvents(prevMouseState, prevMousePosition);
        return 0;
    }
    LRESULT WindowData::OnKBEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (uMsg == WM_KEYDOWN) {
            this->windowApp->OnKeyDown(static_cast<uint16_t>(wParam));
        }
        else if (uMsg == WM_KEYUP) {
            this->windowApp->OnKeyUp(static_cast<uint16_t>(wParam));
        }
        else if (uMsg == WM_CHAR) {
            this->windowApp->OnCharacterInput(static_cast<wchar_t>(wParam));
        }

        return 0;
    }


    void WindowData::ProcessElementMouseEvents(uint8_t prevMouseState, vec2 prevMousePosition) {

        //get elements that are under the cursor
        std::set<ElementNode*, SetDepthComparator> currentSet;
        
        //recursive lambda
        std::function<void(ElementNode*)> checkNodesClick = [&checkNodesClick, &currentSet, this](ElementNode* currentNode) {
            if (currentNode->MouseOverCheck(this->mousePosition)) {
                if (!currentSet.contains(currentNode)) {
                    currentSet.insert(currentNode);
                }
            }
            for (auto& childNodes : currentNode->childNodes) {
                checkNodesClick(childNodes.second);
            }
        };
        checkNodesClick(this->rootWindowNode);

        //if (currentSet.size() == 0) return;

        //use the win32 macros
        std::map<uint8_t, IMouseEventListener::Button> enumbitMappings = { 
            {MK_LBUTTON, IMouseEventListener::Button::PRIMARY},
            {MK_RBUTTON, IMouseEventListener::Button::SECONDARY},
            {MK_MBUTTON, IMouseEventListener::Button::MIDDLE},
            {MK_XBUTTON1, IMouseEventListener::Button::SIDE_1},
            {MK_XBUTTON2, IMouseEventListener::Button::SIDE_2},
        };

        //do mouse global behaviors
        //mouse move
        if (prevMousePosition != this->mousePosition) {
            this->windowApp->OnGlobalMouseMove(this->mousePosition);
        }
        //mouse down
        for (auto& button : enumbitMappings) {
            if ((this->mouseState & button.first) == button.first && (prevMouseState & button.first) == 0) {
                this->windowApp->OnGlobalMouseDown(this->mousePosition, button.second);
            }
        }
        //mouse up
        for (auto& button : enumbitMappings) {
            if ((this->mouseState & button.first) == 0 && (prevMouseState & button.first) == button.first) {
                this->windowApp->OnGlobalMouseUp(this->mousePosition, button.second);
            }
        }

        //mouse press
        //when new mouse state shows pressed but prev doesnt
        for (auto& button : enumbitMappings) {
            if ((this->mouseState & button.first) == button.first && (prevMouseState & button.first) == 0) {
                for (size_t idx = 0; auto & e : currentSet) {
                    for (auto& l : e->clickListeners) {
                        l.second->OnMouseDown(this->mousePosition, button.second, idx);
                    }
                    ++idx;
                }
            }
        }

        //mouse over
        for (size_t idx = 0; ElementNode* element : currentSet) {
            for (auto& l : element->clickListeners) {
                l.second->OnMouseHover(this->mousePosition, idx);
            }
            ++idx;
        }
             
        //mouse not over
        //when element exists in old element set but not in new set
        {
            std::set<ElementNode*, SetDepthComparator> tempSet = this->hoveredElements;
            for (ElementNode* n : currentSet) {
                if (tempSet.count(n) > 0) {
                    tempSet.erase(n);
                }
            }
            for (ElementNode* n : tempSet) {
                for (auto& l : n->clickListeners) {
                    l.second->OnMouseLeaveHover(this->mousePosition);
                }
            }
        }

        //mouse release
        //when new mouse state shows release but prev shows pressed
        for (auto& button : enumbitMappings) {
            if ((this->mouseState & button.first) == 0 && (prevMouseState & button.first) == button.first) {
                for (size_t idx = 0; auto& e : hoveredElements) {
                    for (auto& l : e->clickListeners) {
                        l.second->OnMouseUp(this->mousePosition, button.second);
                    }
                    ++idx;
                }
            }
        }

        //update hovered elements
        this->hoveredElements = currentSet;
    }
}