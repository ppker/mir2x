#include "widget.hpp"
#include "sdldevice.hpp"
#include "checklabel.hpp"

CheckLabel::CheckLabel(
        dir8_t argDir,
        int argX,
        int argY,

        uint32_t argBoxColor,
        int argBoxW,
        int argBoxH,
        int &argBoxRef,

        int argGap,

        const char8_t *argLabel,
        uint8_t  argFont,
        uint8_t  argFontSize,
        uint8_t  argFontStyle,
        uint32_t argFontColor,

        Widget *argParent,
        bool argAutoDelete)
    : Widget(argDir, argX, argY, 0, 0, argParent, argAutoDelete)
    , m_checkBoxColor(argBoxColor)
    , m_labelBoardColor(argFontColor)

    , m_checkBox
      {
          DIR_UPLEFT,
          0,
          0,
          argBoxW,
          argBoxH,

          argBoxColor,
          argBoxRef,

          this,
          false,
      }

    , m_labelBoard
      {
          DIR_UPLEFT,
          0,
          0,

          argLabel,
          argFont,
          argFontSize,
          argFontStyle,
          argFontColor,

          this,
          false,
      }
{
    m_w = m_checkBox.w() + std::max<int>(argGap, 0) + m_labelBoard.w();
    m_h = std::max<int>(m_checkBox.h(), m_labelBoard.h());

    m_checkBox  .moveAt(DIR_LEFT ,   0, m_h / 2);
    m_labelBoard.moveAt(DIR_RIGHT, m_w, m_h / 2);
}

bool CheckLabel::processEvent(const SDL_Event &event, bool valid)
{
    const auto fnOnColor = [this](bool on)
    {
        if(on){
            m_checkBox.setColor(colorf::modRGBA(m_checkBoxColor, colorf::RGBA(0XFF, 0, 0, 0XFF)));
            m_labelBoard.setFontColor(colorf::modRGBA(m_labelBoardColor, colorf::RGBA(0XFF, 0, 0, 0XFF)));
        }
        else{
            m_checkBox.setColor(m_checkBoxColor);
            m_labelBoard.setFontColor(m_labelBoardColor);
        }
    };

    if(!valid){
        fnOnColor(false);
        return consumeFocus(false);
    }

    if(!show()){
        return false;
    }

    if(event.type == SDL_MOUSEMOTION){
        fnOnColor(in(event.motion.x, event.motion.y));
    }

    if(!m_checkBox.processEvent(event, valid)){
        if(event.type == SDL_MOUSEBUTTONDOWN && in(event.button.x, event.button.y)){
            m_checkBox.toggle();
            return consumeFocus(true);
        }
        return false;
    }

    return consumeFocus(true);
}

void CheckLabel::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    if(!show()){
        return;
    }

    for(auto p:
    {
        static_cast<const Widget *>(&m_checkBox),
        static_cast<const Widget *>(&m_labelBoard),
    }){
        if(!p->show()){
            continue;
        }

        int srcXCrop = srcX;
        int srcYCrop = srcY;
        int dstXCrop = dstX;
        int dstYCrop = dstY;
        int srcWCrop = srcW;
        int srcHCrop = srcH;

        if(mathf::cropChildROI(
                    &srcXCrop, &srcYCrop,
                    &srcWCrop, &srcHCrop,
                    &dstXCrop, &dstYCrop,

                    w(),
                    h(),

                    p->dx(),
                    p->dy(),
                    p-> w(),
                    p-> h())){
            p->drawEx(dstXCrop, dstYCrop, srcXCrop, srcYCrop, srcWCrop, srcHCrop);
        }
    }
}
