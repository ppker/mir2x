#include "colorf.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "texsliderbar.hpp"
#include "clientargparser.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

TexSliderBar::TexSliderBar(dir8_t argDir, int argX, int argY, int argSize, bool hslider, int sliderIndex, std::function<void(float)> onChanged, Widget *parent, bool autoDelete)
    : TexSlider
      {
          argDir,
          argX,
          argY,

          /* argW */  hslider ? argSize : SDLDeviceHelper::getTextureHeight(g_progUseDB->retrieve(0X00000460)),
          /* argH */ !hslider ? argSize : SDLDeviceHelper::getTextureHeight(g_progUseDB->retrieve(0X00000460)),

          hslider,
          sliderIndex,

          std::move(onChanged),

          parent,
          autoDelete,
      }
{}

void TexSliderBar::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    if(auto slotTexPtr = g_progUseDB->retrieve(0X00000460)){
        if(hslider()){
            g_sdlDevice->drawTexture(slotTexPtr, dstX, dstY, srcX, srcY, srcW, srcH);
        }
        else{

        }
    }

    if(auto barTexPtr = g_progUseDB->retrieve(0X00000470)){
        if(hslider()){
            g_sdlDevice->drawTexture(barTexPtr, dstX, dstY, srcX, srcY, srcW, srcH);
        }
        else{

        }
    }

    TexSlider::drawEx(dstX, dstY, srcX, srcY, srcW, srcH);
}
