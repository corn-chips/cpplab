#pragma once
#include <string>
#include <map>
#include <mutex>
#include <vector>
#include <atomic>
#include <sstream>
#include <exception>
#include <cmath>

#include "fonts.hpp"

#define max(a, b) (((a) > (b)) ? (a) : (b))

namespace cpplab {
    struct vec2 {
        float x, y;
        vec2() { this->x = 0.f; this->y = 0.f; };
        vec2(float x_, float y_) : x{ x_ }, y{ y_ } {};

        vec2 operator+(const vec2& addend) {
            return vec2(this->x + addend.x, this->y + addend.y);
        }
        vec2 operator-(const vec2& subtractor) {
            return vec2(this->x - subtractor.x, this->y - subtractor.y);
        }
        bool operator==(const vec2& rhs) {
            return (this->x == rhs.x && this->y == rhs.y);
        }
        bool operator!=(const vec2& rhs) {
            return !(this->operator==(rhs));
        }
    };
    struct Color {
        float r, b, g, a;
        Color() {
            this->r = 1.0f;
            this->g = 1.0f;
            this->b = 1.0f;
            this->a = 1.0f;
        }
        Color(float r, float g, float b, float a) {
            this->r = r;
            this->g = g;
            this->b = b;
            this->a = a;
        }
        Color(float r, float g, float b) {
            this->r = r;
            this->g = g;
            this->b = b;
            this->a = 1.0f;
        }
        Color(unsigned int r, unsigned int g, unsigned int b) {
            this->r = std::fminf(r / 255.f, 1.0f);
            this->g = std::fminf(g / 255.f, 1.0f);
            this->b = std::fminf(b / 255.f, 1.0f);
            this->a = 1.0f;
        }
        Color(float v) {
            this->r = v;
            this->g = v;
            this->b = v;
            this->a = 1.0f;
        }
        Color(float v, float a) {
            this->r = v;
            this->g = v;
            this->b = v;
            this->a = a;
        }
    };

    class IMouseEventListener {
    public:
        enum class Button {
            PRIMARY, SECONDARY, MIDDLE, SIDE_1, SIDE_2
        };

        //mouse events
        virtual void OnMouseDown(vec2 clickpoint, Button button, size_t depthOrder) = 0;
        virtual void OnMouseUp(vec2 releasepoint, Button button) = 0;
        virtual void OnMouseHover(vec2 currentpos, size_t depthOrder) = 0;
        virtual void OnMouseLeaveHover(vec2 currentpos) = 0;
    };

    enum class NodeType {
        BASE_ELEMENT, TEXT_AREA, IMAGE // add more elements cause its kinda lacking rn
    };

    struct ElementNodeConfiguration {
    public:
        float depth;
        vec2 relativePos, dimensions;
        bool hidden;

        ElementNodeConfiguration() {
            this->depth = 0.0f;
            this->relativePos = vec2(0.f, 0.f);
            this->dimensions = vec2(0.f, 0.f);
            this->hidden = true;
        }
        ElementNodeConfiguration& SetDepth(float depth) {
            this->depth = depth;
            return *this;
        }
        ElementNodeConfiguration& SetRelativePosition(vec2 relPos) {
            this->relativePos = relPos;
            return *this;
        }
        ElementNodeConfiguration& SetDimension(vec2 dim) {
            this->dimensions = dim;
            return *this;
        }
        ElementNodeConfiguration& SetHidden(bool state) {
            this->hidden = state;
            return *this;
        }
    };
    class ElementNode {
        friend class WindowD2DRenderer;
        friend class WindowData;
    protected:
        static const bool DoElementErrorChecking = true;

        std::mutex elementLock;
        std::atomic_bool modified = true;

        float depth = 0.0f;
        vec2 relativePos = vec2(0, 0);
        vec2 dimensions = vec2(0, 0);

        ElementNode* parentNode = NULL;
        std::map<std::string, ElementNode*> childNodes;

        bool hidden = true;
        std::string selfId;

        NodeType type = NodeType::BASE_ELEMENT;

        std::map<std::string, IMouseEventListener*> clickListeners;

        virtual bool MouseOverCheck(vec2 windowMousePosition) {
            const vec2 bbox_tl = this->getTruePosition();
            const vec2 bbox_br = this->getDimensions() + bbox_tl;

            return windowMousePosition.x > bbox_tl.x &&
                   windowMousePosition.x < bbox_br.x &&
                   windowMousePosition.y > bbox_tl.y &&
                   windowMousePosition.y < bbox_br.y;
        }

    private:
        bool RecursiveCheckModification() {
            if (this->modified) return true;
            if (this->childNodes.size() == 0) return false;

            std::lock_guard<std::mutex> guard(this->elementLock);
            for (auto& childNodePair : this->childNodes) {
                if (childNodePair.second->RecursiveCheckModification()) return true;
            }
            return false;
        }
        void ResetModification() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->modified = false;
        }

        void setSelfId(const std::string& id) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->selfId = id;
        }
    public:
        ElementNode() = default;
        ElementNode(const ElementNodeConfiguration& config) {
            this->depth = config.depth;
            this->relativePos = config.relativePos;
            this->dimensions = config.dimensions;
            this->hidden = config.hidden;
        }
        ElementNode(ElementNode&&) = delete;
        ElementNode(const ElementNode&) = delete;
        virtual ~ElementNode() {};

        NodeType getNodeType() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->type;
        }
        std::string getSelfId() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->selfId;
        }

        virtual void setDepth(float depth) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->depth = depth;
            this->modified = true;
        }
        virtual float getDepth() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->depth;
        }

        virtual vec2 getRelativePosition() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->relativePos;
        }
        virtual void setRelativePosition(vec2 pos) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->relativePos = pos;
            this->modified = true;
        }
        
        virtual vec2 getTruePosition() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            if (parentNode != NULL) {
                return parentNode->getTruePosition() + this->relativePos;
            }
            else {
                return this->relativePos;
            }
        }

        virtual void setDimensions(vec2 dim) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->dimensions = dim;
            this->modified = true;
        }
        virtual vec2 getDimensions() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->dimensions;
        }

        virtual void show() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->hidden = false;
            this->modified = true;
        }
        virtual void showAllChildren() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->hidden = false;
            this->modified = true;
            for (auto& elmt : this->childNodes) {
                elmt.second->showAllChildren();
            }
        }
        virtual void hide() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->hidden = true;
            this->modified = true;
        }
        virtual void hideAllChildren() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->hidden = true;
            this->modified = true;
            for (auto& elmt : this->childNodes) {
                elmt.second->hideAllChildren();
            }
        }
        virtual bool isHidden() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->hidden;
        }

        virtual void AddChildElement(const std::string& strId, ElementNode* element) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            if (ElementNode::DoElementErrorChecking && this->childNodes.contains(strId)) {
                std::stringstream errmsg;
                errmsg << "ID collision detected! Attempted to add element with specified id \"" << strId
                    << "\" to element \"" << this->selfId << "\", but the parent element already contains a child node of that id.";
                throw std::runtime_error(errmsg.str());
            }
            if (ElementNode::DoElementErrorChecking) {
                for (const auto& elemt : this->childNodes) {
                    if (elemt.second == element) {
                        std::stringstream errmsg;
                        errmsg << "Double add detected! Attempeted to add element with specified id \"" <<
                            strId << "\" to element \"" <<
                            this->selfId << "\" but a child element with the same pointer was detected.";
                        throw std::runtime_error(errmsg.str());
                    }
                }
            }
            this->childNodes.insert({ {strId, element} });
            element->parentNode = this;
            element->setSelfId(strId);
            this->modified = true;
        }
        //users are responsible for their own memory cleanup! keep track of your pointers
        virtual void DeleteChildElement(const std::string& strId) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            if (ElementNode::DoElementErrorChecking && !this->childNodes.contains(strId)) {
                std::stringstream errmsg;
                errmsg << "Invalid element deletion detected! " <<
                    "Attempted to delete an element of id \"" << strId <<
                    "\" from the element \"" << this->selfId <<
                    "\", but the parent element does not contain a child element with the specified id.";
                throw std::runtime_error(errmsg.str());
            }
            this->childNodes.erase(strId);
            this->modified = true;
        }

        virtual void AddClickListener(const std::string& listenerId, IMouseEventListener* listener) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            //check id exists
            if (ElementNode::DoElementErrorChecking && this->clickListeners.contains(listenerId)) {
                std::stringstream errmsg;
                errmsg << "ID collision detected! Attempted to add listener with specified id \"" << listenerId
                    << "\" to element \"" << this->selfId << "\", but the parent element already contains a click listener of that id.";
                throw std::runtime_error(errmsg.str());
            }
            //check listener exists
            if (ElementNode::DoElementErrorChecking) {
                for (const auto& lstnr : this->clickListeners) {
                    if (lstnr.second == listener) {
                        std::stringstream errmsg;
                        errmsg << "Double add detected! Attempeted to add listener with specified id \"" <<
                            listenerId << "\" to element \"" <<
                            this->selfId << "\" but an existing click listener with the same pointer was detected.";
                        throw std::runtime_error(errmsg.str());
                    }
                }
            }

            this->clickListeners.insert({ {listenerId, listener } });
        }
        virtual void DeleteClickListener(const std::string& listenerId) {
            //check id exists
            if (ElementNode::DoElementErrorChecking && !this->clickListeners.contains(listenerId)) {
                std::stringstream errmsg;
                errmsg << "Invalid click listener deletion detected! " <<
                    "Attempted to delete a click listener of id \"" << listenerId <<
                    "\" from the element \"" << this->selfId << "\", but the element does not contain a click listener with the specified id.";
                throw std::runtime_error(errmsg.str());
            }
            this->clickListeners.erase(listenerId);
        }

        virtual ElementNode* GetChildElement(const std::string& strId) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            if (this->childNodes.contains(strId)) {
                return this->childNodes[strId];
            }
            else if (ElementNode::DoElementErrorChecking) {
                std::stringstream errmsg;
                errmsg << "Invalid element access detected! " <<
                    "Attempted to access an element of id \"" << strId <<
                    "\" from the element \"" << this->selfId <<
                    "\", but the parent element does not contain a child element with the specified id.";
                throw std::runtime_error(errmsg.str());
            }
            else return nullptr;
        }
        virtual std::vector<std::string> GetAllChildElements() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            std::vector<std::string> childElementsIds;
            for (auto& elmt : this->childNodes) {
                childElementsIds.push_back(elmt.first);
            }
            return childElementsIds;
        }
        virtual size_t RecursiveCountAllChildNodes() {
            size_t count = 0;
            count += this->childNodes.size();
            for (auto& node : this->childNodes) {
                count += node.second->RecursiveCountAllChildNodes();
            }
            return count;
        }
    };

    /*class PrimitiveShapeElement : public ElementNode {
    public:
        enum class Shape {
            REGULAR_POLYGON, ELLIPSE, STAR
        };

    private:
        Shape sh;
        Color color;

        int polygon_sides;
        int star_vertices;
        int star_innerRD;

    public:
        PrimitiveShapeElement() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->sh = Shape::REGULAR_POLYGON;
            this->color = Color();

            this->polygon_sides = 4;
            this->star_vertices = 5;
            this->star_innerRD = 10;

            this->type = NodeType::PRIMITIVE_SHAPE;
        }

        void setShape(Shape sh) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->sh = sh;
            this->modified = true;
        }
        Shape getShape() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->sh;
        }

        void setPolygonSides(int sides) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->polygon_sides = max(sides, 3);
            this->modified = true;
        }
        int getPolygonSides() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->polygon_sides;
        }

        void setStarVertices(int vertices) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->star_vertices = max(4, vertices);
            this->modified = true;
        }
        int getStarVertices() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->star_vertices;
        }
        void setStarInnerRadius(int px) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->star_innerRD = max(px, 1);
            this->modified = true;
        }
        int getStarInnerRadius() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->star_innerRD;
        }
    };*/

    //enums
    enum class HAlign {
        LEFT, CENTER, RIGHT, JUSTIFY
    };
    enum class VAlign {
        TOP, CENTER, BOTTOM
    };

    struct TextElementConfiguration : public ElementNodeConfiguration {
    public:
        Color textColor, backgroundColor, borderColor;
        float borderWidthPx, roundedEdgePx, textMarginPx, textPtSize;
        FontStyle fontStyle;
        HAlign textHorizontalAlignment;
        VAlign textVerticalAlignment;

        std::wstring startingText;

        TextElementConfiguration() {
            this->textColor = Color(0.0f);
            this->backgroundColor = Color(1.0f, 0.0f);
            this->borderColor = Color(0.0f);
            this->borderWidthPx = 0.f;
            this->roundedEdgePx = 0.f;
            this->textMarginPx = 0.f;

            this->fontStyle = FontStyle::Arial;
            this->textHorizontalAlignment = HAlign::LEFT;
            this->textVerticalAlignment = VAlign::TOP;
            this->textPtSize = 12.f;

            this->startingText = L"TextElementConfiguration startingText was left unchanged from default.";
        }
        TextElementConfiguration& setTextColor(Color c) {
            this->textColor = c;
            return *this;
        }
        TextElementConfiguration& setBackgroundColor(Color c) {
            this->backgroundColor = c;
            return *this;
        }
        TextElementConfiguration& setBorderColor(Color c) {
            this->borderColor = c;
            return *this;
        }
        TextElementConfiguration& setBorderWidth(float px) {
            this->borderWidthPx = px;
            return *this;
        }
        TextElementConfiguration& setRoundedEdgeRadius(float px) {
            this->roundedEdgePx = px;
            return *this;
        }
        TextElementConfiguration& setTextMarginPx(float px) {
            this->textMarginPx = px;
            return *this;
        }
        TextElementConfiguration& setFontStyle(FontStyle style) {
            this->fontStyle = style;
            return *this;
        }
        TextElementConfiguration& setTextHorizontalAlignment(HAlign alignment) {
            this->textHorizontalAlignment = alignment;
            return *this;
        }
        TextElementConfiguration& setTextVerticalAlignment(VAlign alignment) {
            this->textVerticalAlignment = alignment;
            return *this;
        }
        TextElementConfiguration& setTextSize(float ptSize) {
            this->textPtSize = ptSize;
            return *this;
        }
        TextElementConfiguration& setText(const std::wstring& text) {
            this->startingText = text;
            return *this;
        }
    };

    class TextElement : public ElementNode {
    private:
        // text color
        // background color
        // border color
        // border width
        // rounded edge radius
        // text margin
        // font style
        // text alignment
        //

        Color textColor, backgroundColor, borderColor;
        float borderWidthPx;
        float roundedEdgePx;
        float textMarginPx;

        FontStyle fontStyle;
        HAlign textHAlign;
        VAlign textVAlign;
        float textPointSize;

        std::wstring text;
    public:
        TextElement() {
            this->textColor = Color(0.0f);
            this->backgroundColor = Color(1.0f, 0.0f);
            this->borderColor = Color(0.0f);
            this->borderWidthPx = 0.f;
            this->roundedEdgePx = 0.f;
            this->textMarginPx = 0.f;

            this->fontStyle = FontStyle::Arial;
            this->textHAlign = HAlign::LEFT;
            this->textVAlign = VAlign::TOP;
            this->textPointSize = 12.f;

            this->type = NodeType::TEXT_AREA;
            this->text = L"Use setText to set element text.";
        }
        TextElement(const TextElementConfiguration& config) : 
        ElementNode(reinterpret_cast<const ElementNodeConfiguration&>(config)) {
            this->textColor = config.textColor;
            this->backgroundColor = config.backgroundColor;
            this->borderColor = config.borderColor;
            this->borderWidthPx = config.borderWidthPx;
            this->roundedEdgePx = config.roundedEdgePx;
            this->textMarginPx = config.textMarginPx;

            this->fontStyle = config.fontStyle;
            this->textHAlign = config.textHorizontalAlignment;
            this->textVAlign = config.textVerticalAlignment;
            this->textPointSize = config.textPtSize;

            this->text = config.startingText;

            this->type = NodeType::TEXT_AREA;
            
        }

        void setText(const std::wstring& text) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->text = text;
            this->modified = true;
        }
        std::wstring getText() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->text;
        }
            
        
        Color getTextColor() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->textColor;
        }
        void setTextColor(Color color) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->textColor = color;
            this->modified = true;
        }

        Color getBackgroundColor() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->backgroundColor;
        }
        void setBackgroundColor(Color color) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->backgroundColor = color;
            this->modified = true;
        }

        Color getBorderColor() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->borderColor;
        }
        void setBorderColor(Color color) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->borderColor = color;
            this->modified = true;
        }

        float getBorderWidth() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->borderWidthPx;
        }
        void setBorderWidth(float width) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->borderWidthPx = width;
            this->modified = true;
        }


        float getRoundedEdge() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->roundedEdgePx;
        }
        void setRoundedEdge(float radius) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->roundedEdgePx = radius;
            this->modified = true;
        }

        float getTextMargin() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->textMarginPx;
        }
        void setTextMargin(float margin) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->textMarginPx = margin;
            this->modified = true;
        }

        float getTextPointSize() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->textPointSize;
        }
        void setTextPointSize(float pt) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->textPointSize = pt;
            this->modified = true;
        }

        FontStyle getFontStyle() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->fontStyle;
        }
        void setFontStyle(FontStyle style) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->fontStyle = style;
            this->modified = true;
        }

        HAlign getTextHAlign() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->textHAlign;
        }
        void setTextHAlign(HAlign align) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->textHAlign = align;
            this->modified = true;
        }

        VAlign getTextVAlign() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->textVAlign;
        }
        void setTextVAlign(VAlign align) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->textVAlign = align;
            this->modified = true;
        }
    };
    
    enum class ImageFillMode {
        FILL, COVER, CONTAIN
    };
    enum class ImageType {
        FILE, RESOURCE
    };
    enum class BorderWrapMode {
        FIT_TO_IMAGE, FIT_TO_BOUNDING_BOX
    };

    class ImageElement : public ElementNode {
        friend class WindowD2DRenderer;
    private:
        Color borderColor;
        float borderWidthPx;
        float roundedEdgePx;
        ImageFillMode fillMode;
        BorderWrapMode borderMode;

        std::wstring imageName;
        ImageType imgType;
        bool imgModified;
    public:
        ImageElement() {
            this->borderColor = Color(0.0f);
            this->fillMode = ImageFillMode::CONTAIN;
            this->borderWidthPx = 0.f;
            this->roundedEdgePx = 0.f;
            this->type = NodeType::IMAGE;
            this->imageName = L"";
            this->imgType = ImageType::FILE;
            this->imgModified = true;
            this->borderMode = BorderWrapMode::FIT_TO_IMAGE;
        }

        void setBorderWrapMode(BorderWrapMode mode) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->borderMode = mode;
            this->modified = true;
        }

        BorderWrapMode getBorderWrapMode() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->borderMode;
        }

        std::wstring getImageFileName() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->imageName;
        }
        void setImageFile(const std::wstring& file, ImageType type) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->imageName = file;
            this->imgType = type;
            this->modified = true;
            this->imgModified = true;
        }

        Color getBorderColor() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->borderColor;
        }
        void setBorderColor(const Color& color) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->borderColor = color;
            this->modified = true;
        }

        float getBorderWidthPx() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->borderWidthPx;
        }
        void setBorderWidthPx(float width) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->borderWidthPx = width;
            this->modified = true;
        }

        float getRoundedEdgePx() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->roundedEdgePx;
        }
        void setRoundedEdgePx(float radius) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->roundedEdgePx = radius;
            this->modified = true;
        }

        ImageFillMode getFillMode() {
            std::lock_guard<std::mutex> guard(this->elementLock);
            return this->fillMode;
        }
        void setFillMode(ImageFillMode mode) {
            std::lock_guard<std::mutex> guard(this->elementLock);
            this->fillMode = mode;
            this->modified = true;
        }
    };
}