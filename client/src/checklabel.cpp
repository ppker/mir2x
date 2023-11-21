#include "widget.hpp"
#include "sdldevice.hpp"
#include "checklabel.hpp"

CheckLabel::CheckLabel(
        dir8_t argDir,
        int argX,
        int argY,

        int &argRef,
        const char8_t *argLabel,

        Widget *argParent,
        bool argAutoDelete)
    : Widget(argDir, argX, argY, 0, 0, argParent, argAutoDelete)
    , m_checkBox
      {
          DIR_UPLEFT,
          0,
          0,

          argRef,

          this,
          false,
      }

    , m_labelBoard
      {
          DIR_UPLEFT,
          0,
          0,

          argLabel,

          1,
          12,

          0,
          colorf::WHITE + colorf::A_SHF(255),

          this,
          false,
      }
{
    constexpr int gap = 5;

    m_w = m_checkBox.w() + gap + m_labelBoard.w();
    m_h = std::max<int>(m_checkBox.h(), m_labelBoard.h());

    m_checkBox.moveTo(0, (m_h - m_checkBox.h()) / 2);
    m_labelBoard.moveTo(m_checkBox.w() + gap, (m_h - m_labelBoard.h()) / 2);
}

bool CheckLabel::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return false;
    }

    if(!show()){
        return false;
    }

    if(!m_checkBox.show()){
        return false;
    }

    if(!m_checkBox.processEvent(event, valid)){
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
