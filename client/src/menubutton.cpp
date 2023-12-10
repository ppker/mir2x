#include <initializer_list>
#include "menubutton.hpp"

MenuButton::MenuButton(dir8_t argDir,
        int argX,
        int argY,

        std::pair<Widget *, bool> argGfxWidget,
        std::pair<Widget *, bool> argMenuBoard,

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

    , m_gfxWidget(argGfxWidget.first)
    , m_menuBoard(argMenuBoard.first)
    , m_margin(argMargin)
{
    m_gfxWidget->moveAt(DIR_UPLEFT, m_margin[2], m_margin[0]);
    m_menuBoard->moveAt(DIR_UPLEFT, m_margin[2], m_margin[0] + m_gfxWidget->h());

    addChild(argGfxWidget.first, argGfxWidget.second);
    addChild(argMenuBoard.first, argMenuBoard.second);

    setSize(m_margin[2] + std::max<int>(m_gfxWidget->w() + m_margin[3], m_menuBoard->w()),
            m_margin[0] + m_gfxWidget->h() + std::max<int>(m_margin[1], m_menuBoard->h()));
}
