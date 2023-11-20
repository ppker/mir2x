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

    const auto fnCropDrawLine = [dstX, dstY, srcX, srcY, srcW, srcH, this](int startX, int startY, int endX, int endY)
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
            g_sdlDevice->drawLine(colorf::RED + colorf::A_SHF(255), drawDstX, drawDstY, drawDstX + drawSrcW - 1, drawDstY + drawSrcH - 1);
        }
    };

    fnCropDrawLine(0  , 0  , w(), 0  );
    fnCropDrawLine(0  , 0  , 0  , h());
    fnCropDrawLine(0  , h(), w(), h());
    fnCropDrawLine(w(), 0  , w(), h());
}

SDL_Texture *CheckBox::loadFunc(const ImageBoard *)
{
    return g_progUseDB->retrieve(0X00000480);
}
