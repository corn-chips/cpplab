#pragma once
#include "engineelements.hpp"
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>
#include <wincodecsdk.h>
#include <mutex> 
#include <set>

namespace cpplab {

    //microsoft documentation thing
    template <class T> void SafeRelease(T** ppT)
    {
        if (*ppT)
        {
            (*ppT)->Release();
            *ppT = NULL;
        }
    }

    template<typename T>
    struct LinkedListNode {
        T data;
        LinkedListNode<T>* nextNode = nullptr;
    };

    struct NodeRenderData {
        ElementNode* elementNode;
    };
    struct TextElementNodeRenderData : public NodeRenderData {
        //store shape and text renderer to reuse
        D2D1_ROUNDED_RECT boxShape;
        D2D1_RECT_F textboxShape;
        IDWriteTextFormat* textFormat;
    };
    struct ImageElementNodeRenderData : public NodeRenderData {
        D2D1_ROUNDED_RECT boxShape;
        ID2D1Bitmap* image;
        ID2D1PathGeometry* clipMask;
    };

    class WindowD2DRenderer {
    private:
        HWND windowHandle;
        ElementNode* rootElementNode;
        LinkedListNode<NodeRenderData*> persistentDepthSortedNodes;
        std::set<ElementNode*> existingSortedNodes;

        ID2D1Factory* d2dFactory;
        ID2D1HwndRenderTarget* renderTarget;
        ID2D1SolidColorBrush* solidColorBrush;
        ID2D1Layer* clipLayer;

        IDWriteFactory* dwriteFactory;

        IWICImagingFactory* wicImgFactory;

        Color backgroundColor;

        unsigned int dpi;

        void BuildNodeList();

        void BuildD2DResources();
        void FreeD2DResources();

        void PerformElementDraw(NodeRenderData* element);

        float scaleDPI(float val);
        D2D1_RECT_F scaleDPI(D2D1_RECT_F rect);
        D2D1_POINT_2F scaleDPI(D2D1_POINT_2F point);
    public:
        WindowD2DRenderer(HWND windowHandle, ElementNode* rootNode, unsigned int dpi);
        ~WindowD2DRenderer();

        void Draw();
        bool CheckNodeModification();
        void SetBackgroundColor(Color c);
        void RebuildD2DResources();
    };
}