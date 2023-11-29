#include "colorf.hpp"
#include "totype.hpp"
#include "sdldevice.hpp"
#include "imageboard.hpp"
#include "imagecropboard.hpp"
#include "imagecropdupboard.hpp"
#include "resizableframeboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

ResizableFrameBoard::ResizableFrameBoard(
        dir8_t argDir,
        int argX,
        int argY,
        int argW,
        int argH,

        Widget *parent     = nullptr,
        bool    autoDelete = false)
    : Widget(argDir, argX, argY, argW, argH, parent, autoDelete)
    , m_close
      {
          DIR_UPLEFT,
          argW - 51,
          argH - 53,

          {SYS_U32NIL, 0X0000001C, 0X0000001D},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this]()
          {
              this->parent()->setShow(false);
          },

          0,
          0,
          0,
          0,

          true,
          true,
          this,
      }
{
    fflassert(argW >= m_cornerSize * 2);
    fflassert(argH >= m_cornerSize * 2);
}

void ResizableFrameBoard::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    if(g_progUseDB->retrieve(m_frameTexID)){
        const ImageBoard frame
        {
            DIR_UPLEFT,
            0,
            0,

            {},
            {},

            [](const ImageBoard *){ return g_progUseDB->retrieve(m_frameTexID); },
        };

        const ImageCropBoard topLeft     {DIR_UPLEFT,                  0,                  0, &frame,                        0,                        0,                 m_cornerSize,                 m_cornerSize};
        const ImageCropBoard top         {DIR_UPLEFT,       m_cornerSize,                  0, &frame,             m_cornerSize,                        0, frame.w() - 2 * m_cornerSize,                 m_cornerSize};
        const ImageCropBoard topRight    {DIR_UPLEFT, w() - m_cornerSize,                  0, &frame, frame.w() - m_cornerSize,                        0,                 m_cornerSize,                 m_cornerSize};
        const ImageCropBoard left        {DIR_UPLEFT,                  0,       m_cornerSize, &frame,                        0,             m_cornerSize,                 m_cornerSize, frame.h() - 2 * m_cornerSize};
        const ImageCropBoard middle      {DIR_UPLEFT,       m_cornerSize,       m_cornerSize, &frame,             m_cornerSize,             m_cornerSize, frame.w() - 2 * m_cornerSize, frame.h() - 2 * m_cornerSize};
        const ImageCropBoard right       {DIR_UPLEFT, w() - m_cornerSize,       m_cornerSize, &frame, frame.w() - m_cornerSize,             m_cornerSize,                 m_cornerSize, frame.h() - 2 * m_cornerSize};
        const ImageCropBoard bottomLeft  {DIR_UPLEFT,                  0, h() - m_cornerSize, &frame,                        0, frame.h() - m_cornerSize,                 m_cornerSize,                 m_cornerSize};
        const ImageCropBoard bottom      {DIR_UPLEFT,       m_cornerSize, h() - m_cornerSize, &frame,             m_cornerSize, frame.h() - m_cornerSize, frame.w() - 2 * m_cornerSize,                 m_cornerSize};
        const ImageCropBoard bottomRight {DIR_UPLEFT, w() - m_cornerSize, h() - m_cornerSize, &frame, frame.w() - m_cornerSize, frame.h() - m_cornerSize,                 m_cornerSize,                 m_cornerSize};

        const ImageCropDupBoard    topDup{DIR_UPLEFT,       m_cornerSize,                  0, w() - 2 * m_cornerSize,           m_cornerSize, &top    };
        const ImageCropDupBoard   leftDup{DIR_UPLEFT,                  0,       m_cornerSize,           m_cornerSize, h() - 2 * m_cornerSize, &left   };
        const ImageCropDupBoard middleDup{DIR_UPLEFT,       m_cornerSize,       m_cornerSize, w() - 2 * m_cornerSize, h() - 2 * m_cornerSize, &middle };
        const ImageCropDupBoard  rightDup{DIR_UPLEFT, w() - m_cornerSize,       m_cornerSize,           m_cornerSize, h() - 2 * m_cornerSize, &right  };
        const ImageCropDupBoard bottomDup{DIR_UPLEFT,       m_cornerSize, h() - m_cornerSize, w() - 2 * m_cornerSize,           m_cornerSize, &bottom };

        for(const auto &p:
        {
            static_cast<const Widget *>(&topLeft ),
            static_cast<const Widget *>(&topDup  ),
            static_cast<const Widget *>(&topRight),

            static_cast<const Widget *>(&  leftDup),
            static_cast<const Widget *>(&middleDup),
            static_cast<const Widget *>(& rightDup),

            static_cast<const Widget *>(&bottomLeft ),
            static_cast<const Widget *>(&bottomDup  ),
            static_cast<const Widget *>(&bottomRight),
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

                        p->dx(),
                        p->dy(),
                        p-> w(),
                        p-> h())){
                p->drawEx(drawDstX, drawDstY, drawSrcX, drawSrcY, drawSrcW, drawSrcH);
            }
        }
    }
    Widget::drawEx(dstX, dstY, srcX, srcY, srcW, srcH);
}
