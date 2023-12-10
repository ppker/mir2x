#include <initializer_list>
#include "menubutton.hpp"

MenuButton::MenuButton(dir8_t argDir,
        int argX,
        int argY,

        Widget *argGfxWidget,
        Widget *argMenuBoard,

        std::array<int, 4> argMargin,

        Widget *argParent,
        bool argAutoDelete)
    : ButtonBase
      {
          argDir,
          argX,
          argY,
          0,
          0,

          nullptr,
          nullptr,
          nullptr,

          SYS_U32NIL,
          SYS_U32NIL,
          SYS_U32NIL,

          0,
          0,
          1,
          1,

          false,
          argParent,
          argAutoDelete,
      }

    , m_gfxWidget(argGfxWidget)
    , m_menuBoard(argMenuBoard)
    , m_margin(argMargin)
{
    setSize(m_gfxWidget->w() + argMargin[2] + argMargin[3],
            m_gfxWidget->h() + argMargin[0] + argMargin[1]);
}

void MenuButton::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    if(!show()){
        return;
    }

    for(const auto &[widget, offX, offY]: std::initializer_list<std::tuple<Widget *, int, int>>
    {
        {m_gfxWidget, m_margin[2], m_margin[0]},
        {m_menuBoard, m_margin[2], m_margin[0] + m_gfxWidget->h()},
    }){
        if(!widget->show()){
            continue;
        }

        int srcXCrop = srcX;
        int srcYCrop = srcY;
        int dstXCrop = dstX;
        int dstYCrop = dstY;
        int srcWCrop = srcW;
        int srcHCrop = srcH;

        if(!mathf::cropChildROI(
                    &srcXCrop, &srcYCrop,
                    &srcWCrop, &srcHCrop,
                    &dstXCrop, &dstYCrop,

                    m_margin[2] + std::max<int>(m_gfxWidget->w() + m_margin[3], m_menuBoard->w()),
                    m_margin[0] + m_gfxWidget->h() + std::max<int>(m_margin[1], m_menuBoard->h()),

                    offX,
                    offY,
                    widget->w(),
                    widget->h())){
            continue;
        }
        widget->drawEx(dstXCrop, dstYCrop, srcXCrop, srcYCrop, srcWCrop, srcHCrop);
    }
}
