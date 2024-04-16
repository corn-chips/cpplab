#include "windowrenderer.hpp"
#include "engineelements.hpp"
#include <functional>
#include <dwrite.h>

#include <d2d1_1.h>

namespace cpplab {
    WindowD2DRenderer::WindowD2DRenderer(HWND windowHWND, ElementNode* rootNode, unsigned int dpi) {
        this->d2dFactory = NULL;
        this->renderTarget = NULL;
        this->solidColorBrush = NULL;

        this->windowHandle = windowHWND;
        this->rootElementNode = rootNode;
        this->rootElementNode->selfId = "Window Root Node";

        this->persistentDepthSortedNodes.data = new NodeRenderData();
        this->persistentDepthSortedNodes.data->elementNode = rootNode;
        this->existingSortedNodes.insert(rootNode);

        this->dpi = dpi;

        this->BuildD2DResources();
    }
    WindowD2DRenderer::~WindowD2DRenderer() {
        this->FreeD2DResources();
    }

    void WindowD2DRenderer::BuildD2DResources() {
        //d2d factory
        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &this->d2dFactory);
        if (FAILED(hr)) {
            throw std::runtime_error("[Cpplab Renderer] During D2D Initialization: Failure creating D2D Factory!");
        }

        //dwrite factory
        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&dwriteFactory));
        if (FAILED(hr)) {
            throw std::runtime_error("[Cpplab Renderer] During D2D Initialization: Failure creating DWrite Factory!");
        }

        //wicimg factory
        CoInitialize(NULL);
        hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&(this->wicImgFactory))
        );
        if (FAILED(hr)) {
            throw std::runtime_error("[Cpplab Renderer] During WIC Initialization: Failure creating WICImagingFactory instance!");
        }

        //d2d render target
        RECT rc;
        GetClientRect(windowHandle, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(
            static_cast<UINT32>(rc.right), static_cast<UINT32>(rc.bottom)
        );

        hr = this->d2dFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(windowHandle, size),
            &this->renderTarget
        );
        if (FAILED(hr)) {
            throw std::runtime_error("[Cpplab Renderer] During D2D Initialization: Failure creating Render Target!");
        }

        //d2d brush
        //any color will do, we just change later when needed
        const D2D1_COLOR_F color = D2D1::ColorF(0.0f, 0.0f, 0.0f);
        hr = this->renderTarget->CreateSolidColorBrush(color, &this->solidColorBrush);
        if (FAILED(hr)) {
            this->FreeD2DResources();
            throw std::runtime_error("[Cpplab Renderer] During D2D Initialization: Failure creating color brush!");
        }

        hr = this->renderTarget->CreateLayer(&this->clipLayer);
        if(FAILED(hr)) {
            this->FreeD2DResources();
            throw std::runtime_error("[Cpplab Renderer] During D2D Initialization: Failure creating clip layer!");
        }
    }
    void WindowD2DRenderer::FreeD2DResources() {
        SafeRelease(&this->renderTarget);
        SafeRelease(&this->solidColorBrush);
        SafeRelease(&this->clipLayer);
    }
    bool WindowD2DRenderer::CheckNodeModification() {
        return this->rootElementNode->RecursiveCheckModification();
    }

    void WindowD2DRenderer::BuildNodeList() {
        //check if node no longer loaded
        std::map<ElementNode*, bool> nodeLoaded;
        for (ElementNode* p_n : this->existingSortedNodes) {
            nodeLoaded.insert({ {p_n, false } });
        }


        //traverse root node
        //use recursive function (this is a stack destroyer lol)
        std::function<void(ElementNode*)> updateNode = [this, &updateNode, &nodeLoaded](ElementNode* currentNode) {
            //update node loaded
            if (nodeLoaded.contains(currentNode)) {
                if (nodeLoaded[currentNode] == true) {
                    //how tf, node apparently loaded twice?
                    std::stringstream errmsg;
                    errmsg << "[Cpplab Renderer] During drawlist generation: " <<
                        "Node double loaded! Element of id \"" << currentNode->getSelfId() <<
                        "\" was seen twice. It is likely that the element was added to two or more seperate parent elements. Are you sure this is intended behavior?";
                    throw std::runtime_error(errmsg.str());
                }
                nodeLoaded[currentNode] = true;
            }

            //check if node already in linked list
            if (existingSortedNodes.contains(currentNode)) {
                //skip to checking child nodes
                std::vector<std::string> childIds = currentNode->GetAllChildElements();
                for (const std::string& id : childIds) {
                    updateNode(currentNode->GetChildElement(id));
                }
                return;
            }
            //otherwise add this node
            //traverse the sorted nodes until the currently indexed node has a higher depth
            //insert new linkedlist node between the previously indexed node and currently indexed node
            
            //new node
            LinkedListNode<NodeRenderData*>* newNode = new LinkedListNode<NodeRenderData*>();
            if (currentNode->getNodeType() == NodeType::TEXT_AREA) {
                newNode->data = new TextElementNodeRenderData();
            }
            else if (currentNode->getNodeType() == NodeType::IMAGE) {
                newNode->data = new ImageElementNodeRenderData();
            }
            else {
                newNode->data = new NodeRenderData();
            }
            newNode->data->elementNode = currentNode;
            newNode->data->nodeDepth = currentNode->getDepth();

            /* Locate the node before the point of insertion */
            LinkedListNode<NodeRenderData*>* currentlyIndexed = &persistentDepthSortedNodes;
            while (currentlyIndexed->nextNode != nullptr
                && currentlyIndexed->nextNode->data->nodeDepth // calling deleted nodes for depth!
                < newNode->data->nodeDepth) {
                currentlyIndexed = currentlyIndexed->nextNode;
            }
            newNode->nextNode = currentlyIndexed->nextNode;
            currentlyIndexed->nextNode = newNode;

            existingSortedNodes.insert(currentNode);
            
            //check child nodes
            std::vector<std::string> childIds = currentNode->GetAllChildElements();
            for (const std::string& id : childIds) {
                updateNode(currentNode->GetChildElement(id));
            }
            return;
        };
        updateNode(rootElementNode);

        //remove elements from sorted list if no longer exists in element tree
        for (auto nodeExistsPair : nodeLoaded) {
            if (nodeExistsPair.second) continue;

            //find node in the list
            //ignoring the first node is ok cause it never gets deleted i hope
            LinkedListNode<NodeRenderData*>* lastIndexedNode = nullptr;
            LinkedListNode<NodeRenderData*>* currentlyIndexedNode = &this->persistentDepthSortedNodes;
            while (currentlyIndexedNode->data->elementNode != nodeExistsPair.first) {
                if (currentlyIndexedNode->nextNode == nullptr) {
                    currentlyIndexedNode = nullptr;
                    break;
                }
                lastIndexedNode = currentlyIndexedNode;
                currentlyIndexedNode = currentlyIndexedNode->nextNode;
            }
            if (currentlyIndexedNode == nullptr) {
                //wasnt found?? skip regardless
                continue;
            }

            //found in node
            //current node should point to the node after
            lastIndexedNode->nextNode = currentlyIndexedNode->nextNode;
            delete currentlyIndexedNode->data;
            delete currentlyIndexedNode;
        }
    }

    void WindowD2DRenderer::Draw() {
        //draw background color
        PAINTSTRUCT ps;
        BeginPaint(this->windowHandle, &ps);
        this->renderTarget->BeginDraw();
        this->renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
        this->renderTarget->Clear(D2D1::ColorF(
            this->backgroundColor.r,
            this->backgroundColor.g,
            this->backgroundColor.b,
            this->backgroundColor.a
        ));
        
        //draw back to front?
        //create depth sorted element array (find a way to presort)
        //check tree for node count change (small optimization since i think counting nodes is slightly cheaper)
        if((this->rootElementNode->RecursiveCountAllChildNodes() + 1) != this->existingSortedNodes.size())
            this->BuildNodeList();

        //node list should be ordered from least to greatest now
        //too lazy to do some reverse stuff so ill make larger depth on top.
        LinkedListNode<NodeRenderData*>* currentNode = &this->persistentDepthSortedNodes;
        while (true) {
            this->PerformElementDraw(currentNode->data);
            currentNode->data->elementNode->ResetModification();
            if (currentNode->nextNode == nullptr) break;
            currentNode = currentNode->nextNode;
        }

        HRESULT hr = this->renderTarget->EndDraw();
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET) {
            this->FreeD2DResources();
            this->BuildD2DResources();
        }
        EndPaint(this->windowHandle, &ps);
    }

    void WindowD2DRenderer::SetBackgroundColor(Color c) {
        this->backgroundColor = c;
        InvalidateRect(this->windowHandle, NULL, NULL);
    }
    void WindowD2DRenderer::RebuildD2DResources() {
        this->FreeD2DResources();

        //d2d render target
        RECT rc;
        GetClientRect(windowHandle, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(
            static_cast<UINT32>(rc.right), static_cast<UINT32>(rc.bottom)
        );

        HRESULT hr = this->d2dFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(windowHandle, size),
            &this->renderTarget
        );
        if (FAILED(hr)) {
            throw std::runtime_error("[Cpplab Renderer] During D2D Resource Rebuild: Failure creating D2D Render Target!");
        }

        //d2d brush
        //any color will do, we just change later when needed
        const D2D1_COLOR_F color = D2D1::ColorF(0.0f, 0.0f, 0.0f);
        hr = this->renderTarget->CreateSolidColorBrush(color, &this->solidColorBrush);
        if (FAILED(hr)) {
            this->FreeD2DResources();
            throw std::runtime_error("[Cpplab Renderer] During D2D Resource Rebuild: Failure creating D2D Color Brush!");
        }

        hr = this->renderTarget->CreateLayer(&this->clipLayer);
        if (FAILED(hr)) {
            this->FreeD2DResources();
            throw std::runtime_error("[Cpplab Renderer] During D2D Resource Rebuild: Failure creating clip layer!");
        }
    }
}