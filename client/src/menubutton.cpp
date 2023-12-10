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
