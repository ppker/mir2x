#include "pngtexdb.hpp"
#include "checkbox.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

CheckBox::CheckBox(dir8_t argDir,
        int argX,
        int argY,

        int &argValRef,

        Widget *argParent,
        bool    argAutoDelete)
    : Widget(argDir, argX, argY, 16, 16, argParent, argAutoDelete)
    , m_valRef(argValRef)
    , m_checkImage
      {
          DIR_NONE,
          w() / 2,
          h() / 2,
          0,
          0,

          loadFunc,
          colorf::WHITE + colorf::A_SHF(0XFF),

          this,
          false,
      }
{}

void CheckBox::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    if(m_valRef){
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

                    m_checkImage.dx(),
                    m_checkImage.dy(),
                    m_checkImage.w(),
                    m_checkImage.h())){
            m_checkImage.drawEx(drawDstX, drawDstY, drawSrcX, drawSrcY, drawSrcW, drawSrcH);
        }
    }

    const auto fnCropDrawLine = [dstX, dstY, srcX, srcY, srcW, srcH, this](uint32_t color, int startX, int startY, int endX, int endY)
    {
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

                    startX,
                    startY,
                    endX - startX + 1,
                    endY - startY + 1)){
            g_sdlDevice->drawLine(color, drawDstX, drawDstY, drawDstX + drawSrcW - 1, drawDstY + drawSrcH - 1);
        }
    };

    // +----1----+
    // |        ||
    // 4        52
    // |        ||
    // |----6---+|
    // +----3----+

    fnCropDrawLine(colorf::RGBA(231, 231, 189, 128),       0,       0, w() - 1,       0); // 1
    fnCropDrawLine(colorf::RGBA(231, 231, 189, 128), w() - 1,       0, w() - 1, h() - 1); // 2
    fnCropDrawLine(colorf::RGBA(231, 231, 189, 128),       0, h() - 1, w() - 1, h() - 1); // 3
    fnCropDrawLine(colorf::RGBA(231, 231, 189, 128),       0,       0,       0, h() - 1); // 4
    fnCropDrawLine(colorf::RGBA(231, 231, 189,  80), w() - 2,       0, w() - 2, h() - 2); // 5
    fnCropDrawLine(colorf::RGBA(231, 231, 189,  80),       0, h() - 2, w() - 2, h() - 2); // 6
}

bool CheckBox::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return false;
    }

    if(!show()){
        return false;
    }

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                if(in(event.button.x, event.button.y)){
                    m_valRef = !m_valRef;
                    return consumeFocus(true);
                }
                return consumeFocus(false);
            }
        default:
            {
                return false;
            }
    }
}

SDL_Texture *CheckBox::loadFunc(const ImageBoard *)
{
    return g_progUseDB->retrieve(0X00000480);
}
