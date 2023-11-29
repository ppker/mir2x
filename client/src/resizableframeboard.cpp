#include "colorf.hpp"
#include "totype.hpp"
#include "sdldevice.hpp"
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
    auto texPtr = g_progUseDB->retrieve(m_frameTexID);
    if(!texPtr){
        return;
    }

    const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
    const auto fnCropDraw = [texPtr, texW, texH, dstX, dstY, srcW, srcH](int onTexX, int onTexY, int onTexW, int onTexH, int onScrX, int onScrY)
    {
        if(mathf::cropROI(
                    &onTexX, &onTexY,
                    &onTexW, &onTexH,
                    &onScrX, &onScrY,

                    texW,
                    texH,

                    0, 0, -1, -1, dstX, dstY, srcW, srcH)){
            g_sdlDevice->drawTexture(texPtr, onScrX, onScrY, onTexX, onTexY, onTexW, onTexH);
        }
    };

    const auto extendedDstX = dstX - srcX;
    const auto extendedDstY = dstY - srcY;

    fnCropDraw(0                  , 0                  , m_cornerSize, m_cornerSize, extendedDstX                     , extendedDstY                     );
    fnCropDraw(texW - m_cornerSize, 0                  , m_cornerSize, m_cornerSize, extendedDstX + w() - m_cornerSize, extendedDstY                     );
    fnCropDraw(0                  , texH - m_cornerSize, m_cornerSize, m_cornerSize, extendedDstX                     , extendedDstY + h() - m_cornerSize);
    fnCropDraw(texW - m_cornerSize, texH - m_cornerSize, m_cornerSize, m_cornerSize, extendedDstX + w() - m_cornerSize, extendedDstY + h() - m_cornerSize);

    const int texMidWidth  = texW - m_cornerSize * 2;
    const int texMidHeight = texH - m_cornerSize * 2;

    const int drawMidWidth  = w() - m_cornerSize * 2;
    const int drawMidHeight = h() - m_cornerSize * 2;

    int doneDrawWidth = 0;
    while(doneDrawWidth < drawMidWidth){
        const int drawWidth = std::min<int>(texMidWidth, drawMidWidth - doneDrawWidth);
        fnCropDraw(m_cornerSize, 0                  , drawWidth, m_cornerSize, extendedDstX + m_cornerSize + doneDrawWidth, extendedDstY                     );
        fnCropDraw(m_cornerSize, texH - m_cornerSize, drawWidth, m_cornerSize, extendedDstX + m_cornerSize + doneDrawWidth, extendedDstY + h() - m_cornerSize);
        doneDrawWidth += drawWidth;
    }

    int doneDrawHeight = 0;
    while(doneDrawHeight < drawMidHeight){
        const int drawHeight = std::min<int>(texMidHeight, drawMidHeight - doneDrawHeight);
        fnCropDraw(0                  , m_cornerSize, m_cornerSize, drawHeight, extendedDstX                     , extendedDstY + m_cornerSize + doneDrawHeight);
        fnCropDraw(texW - m_cornerSize, m_cornerSize, m_cornerSize, drawHeight, extendedDstX + w() - m_cornerSize, extendedDstY + m_cornerSize + doneDrawHeight);
        doneDrawHeight += drawHeight;
    }

    doneDrawWidth = 0;
    while(doneDrawWidth < drawMidWidth){
        const int drawWidth = std::min<int>(texMidWidth, drawMidWidth - doneDrawWidth);
        doneDrawHeight = 0;
        while(doneDrawHeight < drawMidHeight){
            const int drawHeight = std::min<int>(texMidHeight, drawMidHeight - doneDrawHeight);
            fnCropDraw(m_cornerSize, m_cornerSize, drawWidth, drawHeight, extendedDstX + m_cornerSize + doneDrawWidth, extendedDstY + m_cornerSize + doneDrawHeight);
            doneDrawHeight += drawHeight;
        }
        doneDrawWidth += drawWidth;
    }

    int drawButtonSrcX = srcX;
    int drawButtonSrcY = srcY;
    int drawButtonSrcW = srcW;
    int drawButtonSrcH = srcH;
    int drawButtonDstX = dstX;
    int drawButtonDstY = dstY;

    if(mathf::cropChildROI(
                &drawButtonSrcX, &drawButtonSrcY,
                &drawButtonSrcW, &drawButtonSrcH,
                &drawButtonDstX, &drawButtonDstY,

                w(),
                h(),

                m_close.dx(), m_close.dy(), m_close.w(), m_close.h())){
        m_close.drawEx(drawButtonDstX, drawButtonDstY, drawButtonSrcX, drawButtonSrcY, drawButtonSrcW, drawButtonSrcH);
    }
}

void ResizableFrameBoard::update(double fUpdateTime)
{
    m_close.update(fUpdateTime);
}

bool ResizableFrameBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return false;
    }

    if(!show()){
        return false;
    }

    if(!m_close.processEvent(event, valid)){
        return false;
    }

    return true;
}
