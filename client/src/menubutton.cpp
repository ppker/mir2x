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
    : Widget
      {
          argDir,
          argX,
          argY,
          0,
          0,

          argParent,
          argAutoDelete,
      }

    , m_margin(argMargin)

    , m_gfxWidget(argGfxWidget.first)
    , m_menuBoard(argMenuBoard.first)

    , m_button
      {
          DIR_UPLEFT,
          m_margin[2],
          m_margin[0],
          m_gfxWidget->w() + m_margin[2] + m_margin[3],
          m_gfxWidget->h() + m_margin[0] + m_margin[1],

          nullptr,
          nullptr,
          [this](){
              m_menuBoard->flipShow();
              updateMenuButtonSize();
          },

          SYS_U32NIL,
          SYS_U32NIL,
          SYS_U32NIL,

          0,
          0,
          1,
          1,

          false,

          this,
          false,
      }
{
    m_gfxWidget->moveAt(DIR_UPLEFT, m_margin[2], m_margin[0]);
    m_menuBoard->moveAt(DIR_UPLEFT, m_margin[2], m_margin[0] + m_gfxWidget->h());

    addChild(argGfxWidget.first, argGfxWidget.second);
    addChild(argMenuBoard.first, argMenuBoard.second);

    m_menuBoard->setShow(false);
    updateMenuButtonSize();
}

void MenuButton::updateMenuButtonSize()
{
    setSize(m_margin[2] + std::max<int>(m_gfxWidget->w() + m_margin[3], m_menuBoard->show() ? m_menuBoard->w() : 0),
            m_margin[0] + m_gfxWidget->h() + std::max<int>(m_margin[1], m_menuBoard->show() ? m_menuBoard->h() : 0));
}
