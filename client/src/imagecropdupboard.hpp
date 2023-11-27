#pragma once
#include "imagecropboard.hpp"

class ImageCropDupBoard: public Widget
{
    private:
        const ImageCropBoard * const m_imgCropBoard;

    public:
        ImageCropDupBoard(
                dir8_t argDir,

                int argX,
                int argY,
                int argW,
                int argH,

                const ImageCropBoard *argImgCropBoard,

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

            : Widget
              {
                  argDir,
                  argX,
                  argY,
                  argW,
                  argH,

                  argParent,
                  argAutoDelete,
              }

            , m_imgCropBoard([argImgCropBoard]{ fflassert(argImgCropBoard); return argImgCropBoard; }())
        {}

    public:
        void drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const override
        {
            if(!show()){
                return;
            }

            const auto fnCropDraw = [dstX, dstY, srcW, srcH, this](int onCropBrdX, int onCropBrdY, int onCropBrdW, int onCropBrdH, int onScreenX, int onScreenY)
            {
                if(mathf::cropROI(
                            &onCropBrdX, &onCropBrdY,
                            &onCropBrdW, &onCropBrdH,

                            &onScreenX,
                            &onScreenY,

                            m_imgCropBoard->w(),
                            m_imgCropBoard->h(),

                            0, 0, -1, -1, dstX, dstY, srcW, srcH)){
                    m_imgCropBoard->drawEx(onScreenX, onScreenY, onCropBrdX, onCropBrdY, onCropBrdW, onCropBrdH);
                }
            };

            const auto extendedDstX = dstX - srcX;
            const auto extendedDstY = dstY - srcY;

            int doneDrawWidth = 0;
            while(doneDrawWidth < w()){
                const int drawWidth = std::min<int>(m_imgCropBoard->w(), w() - doneDrawWidth);
                int doneDrawHeight = 0;
                while(doneDrawHeight < h()){
                    const int drawHeight = std::min<int>(m_imgCropBoard->h(), h() - doneDrawHeight);
                    fnCropDraw(0, 0, drawWidth, drawHeight, extendedDstX + doneDrawWidth, extendedDstY + doneDrawHeight);
                    doneDrawHeight += drawHeight;
                }
                doneDrawWidth += drawWidth;
            }
        }
};
