#pragma once
#include <initializer_list>
#include "mathf.hpp"
#include "widget.hpp"
#include "gfxdupboard.hpp"
#include "gfxcropboard.hpp"

class GfxCropDupBoard: public Widget
{
    private:
        const Widget * const m_gfxWidget;

    private:
        const int m_cropX;
        const int m_cropY;
        const int m_cropW;
        const int m_cropH;

    public:
        GfxCropDupBoard(
                dir8_t argDir,

                int argX,
                int argY,
                int argW,
                int argH,

                const Widget *argWidget,

                int argCropX,
                int argCropY,
                int argCropW,
                int argCropH,

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

            , m_gfxWidget([argWidget]{ fflassert(argWidget); return argWidget; }())

            , m_cropX(argCropX)
            , m_cropY(argCropY)
            , m_cropW([argCropW]{ fflassert(argCropW >= 0); return argCropW; }())
            , m_cropH([argCropH]{ fflassert(argCropH >= 0); return argCropH; }())
        {}

    public:
        void drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const override
        {
            if(!show()){
                return;
            }

            int cropSrcX = m_cropX;
            int cropSrcY = m_cropY;
            int cropSrcW = m_cropW;
            int cropSrcH = m_cropH;

            if(!mathf::rectangleOverlapRegion<int>(0, 0, m_gfxWidget->w(), m_gfxWidget->h(), cropSrcX, cropSrcY, cropSrcW, cropSrcH)){
                return;
            }

            //       w0  w1  w2
            //     +---+---+---+
            //  h0 |   |   |   |
            //     +---+---+---+
            //  h1 |   |   |   |
            //     +---+---+---+
            //  h2 |   |   |   |
            //     +---+---+---+

            const int x0 = 0;
            const int y0 = 0;
            const int w0 = cropSrcX;
            const int h0 = cropSrcY;

            const int x1 = x0 + w0;
            const int y1 = y0 + h0;
            const int w1 = cropSrcW;
            const int h1 = cropSrcH;

            const int x2 = x1 + w1;
            const int y2 = y1 + h1;
            const int w2 = m_gfxWidget->w() - w0 - w1;
            const int h2 = m_gfxWidget->h() - h0 - h1;

            const GfxCropBoard topLeft     {DIR_UPLEFT, 0, 0, m_gfxWidget, x0, y0, w0, h0};
            const GfxCropBoard top         {DIR_UPLEFT, 0, 0, m_gfxWidget, x1, y0, w1, h0};
            const GfxCropBoard topRight    {DIR_UPLEFT, 0, 0, m_gfxWidget, x2, y0, w2, h0};
            const GfxCropBoard left        {DIR_UPLEFT, 0, 0, m_gfxWidget, x0, y1, w0, h1};
            const GfxCropBoard middle      {DIR_UPLEFT, 0, 0, m_gfxWidget, x1, y1, w1, h1};
            const GfxCropBoard right       {DIR_UPLEFT, 0, 0, m_gfxWidget, x2, y1, w2, h1};
            const GfxCropBoard bottomLeft  {DIR_UPLEFT, 0, 0, m_gfxWidget, x0, y2, w0, h2};
            const GfxCropBoard bottom      {DIR_UPLEFT, 0, 0, m_gfxWidget, x1, y2, w1, h2};
            const GfxCropBoard bottomRight {DIR_UPLEFT, 0, 0, m_gfxWidget, x2, y2, w2, h2};

            const GfxDupBoard    topDup{DIR_UPLEFT, 0, 0, w() - w0 - w2,            h0, &top    };
            const GfxDupBoard   leftDup{DIR_UPLEFT, 0, 0,            w0, h() - h0 - h2, &left   };
            const GfxDupBoard middleDup{DIR_UPLEFT, 0, 0, w() - w0 - w2, h() - h0 - h2, &middle };
            const GfxDupBoard  rightDup{DIR_UPLEFT, 0, 0,            w2, h() - h0 - h2, &right  };
            const GfxDupBoard bottomDup{DIR_UPLEFT, 0, 0, w() - w0 - w2,            h2, &bottom };

            for(const auto &[widgetPtr, offX, offY]: std::initializer_list<std::tuple<const Widget *, int, int>>
            {
                {static_cast<const Widget *>(&topLeft    ),        0,        0},
                {static_cast<const Widget *>(&topDup     ),       w0,        0},
                {static_cast<const Widget *>(&topRight   ), w() - w2,        0},

                {static_cast<const Widget *>(&  leftDup  ),        0,       h0},
                {static_cast<const Widget *>(&middleDup  ),       w0,       h0},
                {static_cast<const Widget *>(& rightDup  ), w() - w2,       h0},

                {static_cast<const Widget *>(&bottomLeft ),        0, h() - h2},
                {static_cast<const Widget *>(&bottomDup  ),       w0, h() - h2},
                {static_cast<const Widget *>(&bottomRight), w() - w2, h() - h2},
            }){
                int drawSrcX = srcX;
                int drawSrcY = srcY;
                int drawSrcW = srcW;
                int drawSrcH = srcH;
                int drawDstX = dstX;
                int drawDstY = dstY;

                if(mathf::cropChildROI(
                            &drawSrcX, &drawSrcY,
                            &drawSrcW, &drawSrcH,
                            &drawDstX, &drawDstY,

                            w(),
                            h(),

                            offX,
                            offY,
                            widgetPtr->w(),
                            widgetPtr->h())){
                    widgetPtr->drawEx(drawDstX, drawDstY, drawSrcX, drawSrcY, drawSrcW, drawSrcH);
                }
            }
        }
};
