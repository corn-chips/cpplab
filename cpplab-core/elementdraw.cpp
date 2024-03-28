#include "windowrenderer.hpp"

namespace cpplab {

    std::string wstr_to_utf8(const std::wstring& wstr)
    {
        int wstr_len = (int)wcslen(wstr.c_str());
        int num_chars = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr_len, NULL, 0, NULL, NULL);
        CHAR* strTo = (CHAR*)malloc((num_chars + 1) * sizeof(CHAR));
        if (strTo)
        {
            WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr_len, strTo, num_chars, NULL, NULL);
            strTo[num_chars] = '\0';
        }

        std::string ret(strTo);
        free(strTo);
        return ret;
    }

    D2D1_COLOR_F ToD2D1Color(Color cpplabColor) {
        D2D1_COLOR_F tempColor = { };
        tempColor.r = cpplabColor.r;
        tempColor.g = cpplabColor.g;
        tempColor.b = cpplabColor.b;
        tempColor.a = cpplabColor.a;
        return tempColor;
    }

    void WindowD2DRenderer::PerformElementDraw(NodeRenderData* element) {
        if (element->elementNode->getNodeType() == NodeType::TEXT_AREA) {
            TextElementNodeRenderData* elementData = static_cast<TextElementNodeRenderData*>(element);
            TextElement* textElement = (TextElement*)elementData->elementNode;
            if (textElement->modified) {
                //build shapes
                elementData->boxShape = D2D1::RoundedRect(
                    D2D1::RectF(0.0f, 0.0f, scaleDPI(textElement->getDimensions().x), scaleDPI(textElement->getDimensions().y)),
                    scaleDPI(textElement->getRoundedEdge()),
                    scaleDPI(textElement->getRoundedEdge())
                );

                //build text
                this->dwriteFactory->CreateTextFormat(
                    FontStyleToString(textElement->getFontStyle()),
                    NULL,
                    DWRITE_FONT_WEIGHT_NORMAL,
                    DWRITE_FONT_STYLE_NORMAL,
                    DWRITE_FONT_STRETCH_NORMAL,
                    scaleDPI(textElement->getTextPointSize()),
                    L"en-us",
                    &(elementData->textFormat)
                );
                if (textElement->getTextHAlign() == HAlign::LEFT) {
                    elementData->textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
                } else if (textElement->getTextHAlign() == HAlign::CENTER) {
                    elementData->textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
                } else if (textElement->getTextHAlign() == HAlign::RIGHT) {
                    elementData->textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
                } else if (textElement->getTextHAlign() == HAlign::JUSTIFY) {
                    elementData->textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_JUSTIFIED);
                }

                if (textElement->getTextVAlign() == VAlign::TOP) {
                    elementData->textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
                } else if (textElement->getTextVAlign() == VAlign::CENTER) {
                    elementData->textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
                } else if (textElement->getTextVAlign() == VAlign::BOTTOM) {
                    elementData->textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
                }
                

                //build textbox
                elementData->textboxShape = D2D1::RectF(
                    0.f, 0.f,
                    scaleDPI(textElement->getDimensions().x - 2 * (textElement->getTextMargin())),
                    scaleDPI(textElement->getDimensions().y - 2 * (textElement->getTextMargin()))
                );
            }
            
            this->renderTarget->SetTransform(D2D1::Matrix3x2F::Translation(
                scaleDPI(textElement->getTruePosition().x),
                scaleDPI(textElement->getTruePosition().y)
            ));

            //draw fill then outline
            //bg box
            this->solidColorBrush->SetColor(ToD2D1Color(textElement->getBackgroundColor()));
            this->renderTarget->FillRoundedRectangle(elementData->boxShape, this->solidColorBrush);
            this->solidColorBrush->SetColor(ToD2D1Color(textElement->getBorderColor()));
            this->renderTarget->DrawRoundedRectangle(elementData->boxShape, this->solidColorBrush, textElement->getBorderWidth());

            //draw text
            this->renderTarget->SetTransform(D2D1::Matrix3x2F::Translation(
                scaleDPI(textElement->getTruePosition().x + textElement->getTextMargin()),
                scaleDPI(textElement->getTruePosition().y + textElement->getTextMargin())
            ));
            this->solidColorBrush->SetColor(ToD2D1Color(textElement->getTextColor()));
            this->renderTarget->DrawTextW(
                textElement->getText().c_str(),
                textElement->getText().length(),
                elementData->textFormat,
                elementData->textboxShape,
                this->solidColorBrush,
                D2D1_DRAW_TEXT_OPTIONS_NONE,
                DWRITE_MEASURING_MODE_NATURAL
            );
        }
        else if (element->elementNode->getNodeType() == NodeType::IMAGE) {
            ImageElementNodeRenderData* elementData = static_cast<ImageElementNodeRenderData*>(element);
            ImageElement* imgElement = static_cast<ImageElement*>(elementData->elementNode);

            //rebuilding image resources is expensive so only do that sometimes
            if (imgElement->imgModified) {
                if (imgElement->imgType == ImageType::FILE) {
                    IWICBitmapDecoder* decoder = nullptr;
                    HRESULT hr = this->wicImgFactory->CreateDecoderFromFilename(
                        imgElement->getImageFileName().c_str(),
                        NULL,
                        GENERIC_READ,
                        WICDecodeMetadataCacheOnLoad,
                        &decoder
                    );
                    if (FAILED(hr)) {
                        std::stringstream ss;
                        ss << "[Cpplab Renderer] During WIC Image Decoder Creation: Failed to create image decoder for file " <<
                            wstr_to_utf8(imgElement->imageName);
                        SafeRelease(&decoder);
                        throw std::runtime_error(ss.str());
                    }

                    IWICBitmapFrameDecode* decFrame;
                    hr = decoder->GetFrame(0, &decFrame);
                    if (FAILED(hr)) {
                        std::stringstream ss;
                        ss << "[Cpplab Renderer] During WIC Image Decoder Frame Retrieval: Failed to retrieve image frame for file " <<
                            wstr_to_utf8(imgElement->imageName);
                        SafeRelease(&decoder);
                        SafeRelease(&decFrame);
                        throw std::runtime_error(ss.str());
                    }

                    IWICFormatConverter* converter = nullptr;
                    hr = wicImgFactory->CreateFormatConverter(&converter);
                    if (FAILED(hr)) {
                        SafeRelease(&decoder);
                        SafeRelease(&decFrame);
                        SafeRelease(&converter);
                        throw std::runtime_error("[Cpplab Renderer] During WIC Format Converter creation: Failed to create WICFormatConverter!");
                    }

                    hr = converter->Initialize(
                        decFrame,
                        GUID_WICPixelFormat32bppPBGRA,
                        WICBitmapDitherTypeNone,
                        NULL,
                        0.f,
                        WICBitmapPaletteTypeMedianCut
                    );
                    if (FAILED(hr)) {
                        std::stringstream ss;
                        ss << "[Cpplab Renderer] During WIC Format Converter creation: Failed to initialize WICFormatConverter for file" <<
                            wstr_to_utf8(imgElement->imageName);
                        SafeRelease(&decoder);
                        SafeRelease(&decFrame);
                        SafeRelease(&converter);
                        throw std::runtime_error(ss.str());
                    }

                    hr = this->renderTarget->CreateBitmapFromWicBitmap(
                        converter,
                        NULL,
                        &(elementData->image)
                    );
                    if (FAILED(hr)) {
                        std::stringstream ss;
                        ss << "[Cpplab Renderer] During WicBitmap to D2D Bitmap: Failed to create D2D bitmap for file " <<
                            wstr_to_utf8(imgElement->imageName);
                        SafeRelease(&decoder);
                        SafeRelease(&decFrame);
                        SafeRelease(&converter);
                        throw std::runtime_error(ss.str());
                    }
                    SafeRelease(&decoder);
                    SafeRelease(&decFrame);
                    SafeRelease(&converter);
                }
                else if (imgElement->imgType == ImageType::RESOURCE) {
                    // Locate the resource.
                    HRSRC imageResHandle = FindResourceW(NULL,
                        imgElement->imageName.c_str(),
                        RT_BITMAP
                    );
                    if (!imageResHandle) {
                        std::stringstream ss;
                        ss << "[Cpplab Renderer] When finding resource named " <<
                            wstr_to_utf8(imgElement->imageName) << ": Could not create resource handle.";
                        throw std::runtime_error(ss.str());
                    }

                    HGLOBAL imageResDataHandle = LoadResource(NULL, imageResHandle);
                    if (!imageResDataHandle) {
                        std::stringstream ss;
                        ss << "[Cpplab Renderer] When loading resource named " <<
                            wstr_to_utf8(imgElement->imageName) << ": Could not create resource handle.";
                        throw std::runtime_error(ss.str());
                    }

                    void* imgfile = LockResource(imageResDataHandle);
                    if (!imgfile) {
                        std::stringstream ss;
                        ss << "[Cpplab Renderer] When locking resource named " <<
                            wstr_to_utf8(imgElement->imageName) << ": Could not lock resource.";
                        throw std::runtime_error(ss.str());
                    }

                    DWORD imgFileSize = SizeofResource(NULL, imageResHandle);
                    if (!imgFileSize) {
                        std::stringstream ss;
                        ss << "[Cpplab Renderer] When retrieving resource size named " <<
                            wstr_to_utf8(imgElement->imageName) << ": Could not get resource size.";
                        throw std::runtime_error(ss.str());
                    }

                    IWICStream* wicstream = nullptr;
                    HRESULT hr = this->wicImgFactory->CreateStream(&wicstream);
                    if (FAILED(hr)) {
                        std::stringstream ss;
                        ss << "[Cpplab Renderer] When creating WICStream for resource named " <<
                            wstr_to_utf8(imgElement->imageName) << ": Could not create stream.";
                        throw std::runtime_error(ss.str());
                    }

                    hr = wicstream->InitializeFromMemory(
                        reinterpret_cast<BYTE*>(imgfile),
                        imgFileSize
                    );
                    if (FAILED(hr)) {
                        std::stringstream ss;
                        ss << "[Cpplab Renderer] When initializing WICStream for resource named " <<
                            wstr_to_utf8(imgElement->imageName) << ": Could not initialize resource stream.";
                        throw std::runtime_error(ss.str());
                    }

                    IWICBitmapDecoder* decoder = nullptr;
                    hr = this->wicImgFactory->CreateDecoderFromStream(
                        wicstream,
                        NULL,
                        WICDecodeMetadataCacheOnLoad,
                        &decoder
                    );
                    if (FAILED(hr)) {
                        std::stringstream ss;
                        ss << "[Cpplab Renderer] During WIC Image Decoder Creation: Failed to create image decoder for resource " <<
                            wstr_to_utf8(imgElement->imageName);
                        SafeRelease(&decoder);
                        throw std::runtime_error(ss.str());
                    }

                    IWICBitmapFrameDecode* decFrame;
                    hr = decoder->GetFrame(0, &decFrame);
                    if (FAILED(hr)) {
                        std::stringstream ss;
                        ss << "[Cpplab Renderer] During WIC Image Decoder Frame Retrieval: Failed to retrieve image frame for resource " <<
                            wstr_to_utf8(imgElement->imageName);
                        SafeRelease(&decoder);
                        SafeRelease(&decFrame);
                        throw std::runtime_error(ss.str());
                    }

                    IWICFormatConverter* converter = nullptr;
                    hr = wicImgFactory->CreateFormatConverter(&converter);
                    if (FAILED(hr)) {
                        SafeRelease(&decoder);
                        SafeRelease(&decFrame);
                        SafeRelease(&converter);
                        throw std::runtime_error("[Cpplab Renderer] During WIC Format Converter creation: Failed to create WICFormatConverter!");
                    }

                    hr = converter->Initialize(
                        decFrame,
                        GUID_WICPixelFormat32bppPBGRA,
                        WICBitmapDitherTypeNone,
                        NULL,
                        0.f,
                        WICBitmapPaletteTypeMedianCut
                    );
                    if (FAILED(hr)) {
                        std::stringstream ss;
                        ss << "[Cpplab Renderer] During WIC Format Converter creation: Failed to initialize WICFormatConverter for resource" <<
                            wstr_to_utf8(imgElement->imageName);
                        SafeRelease(&decoder);
                        SafeRelease(&decFrame);
                        SafeRelease(&converter);
                        throw std::runtime_error(ss.str());
                    }

                    hr = this->renderTarget->CreateBitmapFromWicBitmap(
                        converter,
                        NULL,
                        &(elementData->image)
                    );
                    if (FAILED(hr)) {
                        std::stringstream ss;
                        ss << "[Cpplab Renderer] During WicBitmap to D2D Bitmap: Failed to create D2D bitmap for resource " <<
                            wstr_to_utf8(imgElement->imageName);
                        SafeRelease(&decoder);
                        SafeRelease(&decFrame);
                        SafeRelease(&converter);
                        throw std::runtime_error(ss.str());
                    }
                    SafeRelease(&decoder);
                    SafeRelease(&decFrame);
                    SafeRelease(&converter);
                }
            }

            auto GenImageBox = [](vec2 dimensions, D2D1_SIZE_F imageDimensions, ImageFillMode mode) -> D2D1_RECT_F {
                if (mode == ImageFillMode::FILL) { //stretch image to fill entire bounding box
                    return D2D1::RectF(
                        0.f, 0.f,
                        dimensions.x, dimensions.y
                    );
                }
                else if (mode == ImageFillMode::COVER) {  //image maintains aspect ratio and is guaranteed to cover all pixels of the bounding box
                    //calculate ratio of image side lengths to box sides lengths
                    vec2 imToBoxRatio = vec2(
                        imageDimensions.width / dimensions.x,
                        imageDimensions.height / dimensions.y
                    );

                    //scale by forcing the smaller ratio to reach one
                    float recipr = imToBoxRatio.x < imToBoxRatio.y ?
                        dimensions.x / imageDimensions.width :
                        dimensions.y / imageDimensions.height;

                    return D2D1::RectF(
                        0.f, 0.f,
                        imageDimensions.width * recipr, imageDimensions.height * recipr
                    );
                }
                else if (mode == ImageFillMode::CONTAIN) { //image maintains aspect ratio and all pixels of the image are guaranteed to be present in the bounding box
                    //calculate ratio of image side lengths to box sides lengths
                    vec2 imToBoxRatio = vec2(
                        imageDimensions.width / dimensions.x,
                        imageDimensions.height / dimensions.y
                    );
                    //scale by forcing the larger ratio to reach one
                    float recipr = imToBoxRatio.x > imToBoxRatio.y ?
                        dimensions.x / imageDimensions.width :
                        dimensions.y / imageDimensions.height;
                    imToBoxRatio.x = imToBoxRatio.x * recipr;
                    imToBoxRatio.y = imToBoxRatio.y * recipr;

                    return D2D1::RectF(
                        0.f, 0.f,
                        imageDimensions.width*recipr, imageDimensions.height* recipr
                    );
                }
            };


            D2D1_RECT_F imgrect = GenImageBox(imgElement->dimensions, elementData->image->GetSize(), imgElement->getFillMode());
            //calculate image position
            vec2 imgpos = vec2(
                (imgElement->getTruePosition().x + (imgElement->getDimensions().x / 2)) - (imgrect.right / 2),
                (imgElement->getTruePosition().y + (imgElement->getDimensions().y / 2)) - (imgrect.bottom / 2)
            );

            //other things
            if (imgElement->modified) {
                //building the box
                if (imgElement->getBorderWrapMode() == BorderWrapMode::FIT_TO_BOUNDING_BOX) {
                    elementData->boxShape = D2D1::RoundedRect(
                        D2D1::RectF(0.f, 0.f,
                            scaleDPI(imgElement->getDimensions().x), scaleDPI(imgElement->getDimensions().y)
                        ),
                        scaleDPI(imgElement->getRoundedEdgePx()), scaleDPI(imgElement->getRoundedEdgePx())
                    );
                }
                else if (imgElement->getBorderWrapMode() == BorderWrapMode::FIT_TO_IMAGE) {
                    elementData->boxShape = D2D1::RoundedRect(
                        imgrect,
                        scaleDPI(imgElement->getRoundedEdgePx()), scaleDPI(imgElement->getRoundedEdgePx())
                    );
                }

                //building the clip mask
                HRESULT hr = this->d2dFactory->CreatePathGeometry(&elementData->clipMask);
                ID2D1GeometrySink* gmSink = nullptr;
                if (SUCCEEDED(hr)) hr = elementData->clipMask->Open(&gmSink);
                if (SUCCEEDED(hr)) {
                    gmSink->SetFillMode(D2D1_FILL_MODE_WINDING);

                    D2D1_POINT_2F tl = D2D1::Point2F(0.f, 0.f);
                    D2D1_POINT_2F tr = D2D1::Point2F(elementData->boxShape.rect.right, 0.f);
                    D2D1_POINT_2F br = D2D1::Point2F(elementData->boxShape.rect.right, elementData->boxShape.rect.bottom);
                    D2D1_POINT_2F bl = D2D1::Point2F(0.f, elementData->boxShape.rect.bottom);

                    gmSink->BeginFigure(tl, D2D1_FIGURE_BEGIN_FILLED);
                    gmSink->AddLine(tr);
                    gmSink->AddLine(br);
                    gmSink->AddLine(bl);
                    gmSink->EndFigure(D2D1_FIGURE_END_CLOSED);
                    hr = gmSink->Close();
                }
                SafeRelease(&gmSink);
                if (FAILED(hr)) {
                    throw std::runtime_error("[Cpplab Renderer] During Image Clip Mask Generation: Failed to create clip mask!");
                }
            }
            //draw
            //draw image then draw border on top
            
            vec2 imgTruePos = imgElement->getTruePosition();

            this->renderTarget->SetTransform(D2D1::Matrix3x2F::Translation(
                scaleDPI(imgpos.x),
                scaleDPI(imgpos.y)
            ));

            this->renderTarget->PushLayer(
                D2D1::LayerParameters(
                    D2D1::InfiniteRect(),
                    elementData->clipMask,
                    D2D1_ANTIALIAS_MODE_PER_PRIMITIVE,
                    D2D1::Matrix3x2F::Translation(
                        scaleDPI(imgTruePos.x - imgpos.x), scaleDPI(imgTruePos.y - imgpos.y)
                    ),
                    1.0f,
                    nullptr
                ),
                this->clipLayer
            );
            
            
            this->renderTarget->DrawBitmap(
                elementData->image,
                scaleDPI(imgrect),
                1.0,
                D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
            );
            this->renderTarget->PopLayer();

            this->renderTarget->SetTransform(D2D1::Matrix3x2F::Translation(
                scaleDPI(imgElement->getTruePosition().x),
                scaleDPI(imgElement->getTruePosition().y)
            ));
            this->solidColorBrush->SetColor(ToD2D1Color(imgElement->getBorderColor()));
            this->renderTarget->DrawRoundedRectangle(
                D2D1::RoundedRect(
                    elementData->boxShape.rect,
                    elementData->boxShape.radiusX,
                    elementData->boxShape.radiusY
                ),
                this->solidColorBrush,
                imgElement->getBorderWidthPx()
            );
        }
    }

    float WindowD2DRenderer::scaleDPI(float val) {
        return (val * 96.f)/this->dpi;
    }
    D2D1_RECT_F WindowD2DRenderer::scaleDPI(D2D1_RECT_F rect) {
        return D2D1::RectF(
            (rect.left * 96.f) / this->dpi,
            (rect.top * 96.f) / this->dpi,
            (rect.right * 96.f) / this->dpi,
            (rect.bottom * 96.f) / this->dpi
        );
    }
    D2D1_POINT_2F WindowD2DRenderer::scaleDPI(D2D1_POINT_2F point) {
        return D2D1::Point2F(
            scaleDPI(point.x),
            scaleDPI(point.y)
        );
    }
}