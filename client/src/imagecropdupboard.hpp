#pragma once
#include "mathf.hpp"
#include "widget.hpp"

class ImageCropDupBoard: public Widget
{
    private:
        const Widget * const m_widget;

    public:
        ImageCropDupBoard(
                dir8_t argDir,

                int argX,
                int argY,
                int argW,
                int argH,

                const Widget *argWidget,

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false,
                bool    argAutoDraw   = true)

            : Widget
              {
                  argDir,
                  argX,
                  argY,
                  argW,
                  argH,

                  argParent,
                  argAutoDelete,
                  argAutoDraw,
              }

            , m_widget([argWidget]{ fflassert(argWidget); return argWidget; }())
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

                            m_widget->w(),
                            m_widget->h(),

                            0, 0, -1, -1, dstX, dstY, srcW, srcH)){
                    m_widget->drawEx(onScreenX, onScreenY, onCropBrdX, onCropBrdY, onCropBrdW, onCropBrdH);
                }
            };

            const auto extendedDstX = dstX - srcX;
            const auto extendedDstY = dstY - srcY;

            int doneDrawWidth = 0;
            while(doneDrawWidth < w()){
                const int drawWidth = std::min<int>(m_widget->w(), w() - doneDrawWidth);
                int doneDrawHeight = 0;
                while(doneDrawHeight < h()){
                    const int drawHeight = std::min<int>(m_widget->h(), h() - doneDrawHeight);
                    fnCropDraw(0, 0, drawWidth, drawHeight, extendedDstX + doneDrawWidth, extendedDstY + doneDrawHeight);
                    doneDrawHeight += drawHeight;
                }
                doneDrawWidth += drawWidth;
            }
        }
};
