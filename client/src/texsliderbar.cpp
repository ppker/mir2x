#include "pngtexdb.hpp"
#include "texsliderbar.hpp"

extern PNGTexDB *g_progUseDB;
TexSliderBar::TexSliderBar(
        dir8_t argDir,

        int argX,
        int argY,
        int argSize,

        bool argHSlider,
        int  argSliderIndex,

        std::function<void(float)> argOnChanged,

        Widget *argParent,
        bool    argAutoDelete)

    : WidgetContainer
      {
          argDir,
          argX,
          argY,

          /* argW */  argHSlider ? [argSize]{ fflassert(argSize >= 6, argSize); return argSize; }() : SDLDeviceHelper::getTextureHeight(g_progUseDB->retrieve(0X00000460)),
          /* argH */ !argHSlider ? [argSize]{ fflassert(argSize >= 6, argSize); return argSize; }() : SDLDeviceHelper::getTextureHeight(g_progUseDB->retrieve(0X00000460)),

          argParent,
          argAutoDelete,
      }

    , m_slotImage
      {
          DIR_UPLEFT,
          0,
          0,

          {},
          {},

          [](const ImageBoard *){ return g_progUseDB->retrieve(0X00000460); },

          false,
          false,
          argHSlider ? 0 : 1,
      }

    , m_barImage
      {
          DIR_UPLEFT,
          0,
          0,

          {},
          {},

          [](const ImageBoard *){ return g_progUseDB->retrieve(0X00000470); },

          false,
          false,

          argHSlider ? 0 : 1,
      }

    , m_barCrop
      {
          DIR_UPLEFT,
          0,
          0,

          &m_barImage,

          0,
          0,
          m_barImage.w(),
          m_barImage.h(),
      }

    , m_slotCropFirst
      {
          argHSlider ? DIR_LEFT : DIR_UP,

          /* x */  argHSlider ? 0 : w() / 2,
          /* y */ !argHSlider ? 0 : h() / 2,

          &m_slotImage,

          0,
          0,

          /* brdCropW */  argHSlider ? 3 : m_slotImage.w(),
          /* brdCropW */ !argHSlider ? 3 : m_slotImage.h(),

          0,
          0,
          0,
          0,

          this,
          false,
      }

    , m_slotCropSecond
      {
          argHSlider ? DIR_RIGHT : DIR_DOWN,

          /* x */  argHSlider ? w() : w() / 2,
          /* y */ !argHSlider ? h() : h() / 2,

          &m_slotImage,

          /* brdCropX */  argHSlider ? m_slotImage.w() - 3 : 0,
          /* brdCropY */ !argHSlider ? m_slotImage.h() - 3 : 0,

          /* brdCropW */  argHSlider ? 3 : m_slotImage.w(),
          /* brdCropW */ !argHSlider ? 3 : m_slotImage.h(),

          0,
          0,
          0,
          0,

          this,
          false,
      }

    , m_barCropDup
      {
          argHSlider ? DIR_LEFT : DIR_UP,

          /* x */  argHSlider ? 3 : 0,
          /* y */ !argHSlider ? 3 : 0,

          /* w */  argHSlider ? m_barCrop.w() - 6 : m_barCrop.w(),
          /* h */ !argHSlider ? m_barCrop.h() - 6 : m_barCrop.h(),

          &m_barCrop,

          this,
          false,
      }

    , m_slotMidCropDup
      {
          argHSlider ? DIR_LEFT : DIR_UP,

          /* x */  argHSlider ? 3 : 0,
          /* y */ !argHSlider ? 3 : 0,

          /* w */  argHSlider ? m_slotImage.w() - 6 : m_slotImage.w(),
          /* h */ !argHSlider ? m_slotImage.h() - 6 : m_slotImage.h(),

          &m_barCrop,

          this,
          false,
      }

    , m_slider
      {
          DIR_NONE,
          w() / 2,
          h() / 2,

          w(),
          h(),

          argHSlider,
          argSliderIndex,

          std::move(argOnChanged),

          this,
          false,
      }
{}
