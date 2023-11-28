#pragma once
#include "mathf.hpp"
#include "widget.hpp"

class ImageCropBoard: public Widget
{
    private:
        const Widget * const m_widget;

    private:
        const int m_brdCropX;
        const int m_brdCropY;
        const int m_brdCropW;
        const int m_brdCropH;

    private:
        const int m_marginUp;
        const int m_marginDown;
        const int m_marginLeft;
        const int m_marginRight;

    public:
        ImageCropBoard(
                dir8_t argDir,

                int argX,
                int argY,

                const Widget *argWidget,

                int argBrdCropX, // crop on final board, not on original image
                int argBrdCropY, // ...
                int argBrdCropW, // ...
                int argBrdCropH, // ...

                size_t argMarginUp    = 0,
                size_t argMarginDown  = 0,
                size_t argMarginLeft  = 0,
                size_t argMarginRight = 0,

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false,
                bool    argAutoDraw   = true)

            : Widget
              {
                  argDir,
                  argX,
                  argY,
                  0,
                  0,

                  argParent,
                  argAutoDelete,
                  argAutoDraw,
              }

            , m_widget([argWidget]{ fflassert(argWidget); return argWidget; }())

            , m_brdCropX(argBrdCropX)
            , m_brdCropY(argBrdCropY)
            , m_brdCropW([argBrdCropW]{ fflassert(argBrdCropW >= 0); return argBrdCropW; }())
            , m_brdCropH([argBrdCropH]{ fflassert(argBrdCropH >= 0); return argBrdCropH; }())

            , m_marginUp   (argMarginUp   )
            , m_marginDown (argMarginDown )
            , m_marginLeft (argMarginLeft )
            , m_marginRight(argMarginRight)
        {
            setSize(
                    m_brdCropW + m_marginLeft + m_marginRight, // respect blank space by over-cropping
                    m_brdCropH + m_marginUp   + m_marginDown);
        }

    public:
        void drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const override
        {
            if(!show()){
                return;
            }

            int brdCropX = m_brdCropX;
            int brdCropY = m_brdCropY;
            int brdCropW = m_brdCropW;
            int brdCropH = m_brdCropH;

            if(!mathf::rectangleOverlapRegion<int>(0, 0, m_widget->w(), m_widget->h(), brdCropX, brdCropY, brdCropW, brdCropH)){
                return;
            }

            int drawDstX = dstX;
            int drawDstY = dstY;
            int drawSrcX = srcX;
            int drawSrcY = srcY;
            int drawSrcW = srcW;
            int drawSrcH = srcH;

            if(!mathf::cropChildROI(
                        &drawSrcX, &drawSrcY,
                        &drawSrcW, &drawSrcH,
                        &drawDstX, &drawDstY,

                        w(),
                        h(),

                        m_marginLeft + brdCropX - m_brdCropX,
                        m_marginUp   + brdCropY - m_brdCropY,

                        brdCropW,
                        brdCropH)){
                return;
            }

            m_widget->drawEx(drawDstX, drawDstY, drawSrcX + brdCropX, drawSrcY + brdCropY, drawSrcW, drawSrcH);
        }
};
