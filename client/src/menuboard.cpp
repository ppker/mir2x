#include "colorf.hpp"
#include "sdldevice.hpp"
#include "menuboard.hpp"

extern SDLDevice *g_sdlDevice;

MenuBoard::MenuBoard(dir8_t argDir,
        int argX,
        int argY,

        std::optional<int> argWOpt,

        int argItemSpace,
        int argSeperatorSpace,

        std::initializer_list<std::pair<Widget *, bool>> argMenuItemList,

        std::array<int, 4> argMargin,

        Widget *argParent,
        bool    argAutoDelete)

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

    , m_wOpt(argWOpt)
    , m_itemSpace(argItemSpace)
    , m_seperatorSpace(argSeperatorSpace)
    , m_margin(argMargin)
{
    if(m_wOpt.has_value()){
        fflassert(m_wOpt.value() > 0, m_wOpt);
        m_w = m_wOpt.value() + m_margin[0] + m_margin[2];
    }
    else{
        m_w = 0;
    }

    m_h = m_margin[1] + m_margin[3];

    for(auto p: argMenuItemList){
        addChild(p.first, p.second);
    }
}

void MenuBoard::addChild(Widget *widget, bool autoDelete)
{
    if(widget){
        if(widget->parent()){
            widget->parent()->removeChild(widget, false);
        }

        widget->moveAt(DIR_UPLEFT, 0, h() + m_itemSpace);
        Widget::addChild(widget, autoDelete);

        m_h = widget->y() + widget->h();

        if(!m_wOpt.has_value()){
            m_w = std::max<int>(m_w, widget->w());
        }
    }
}

void MenuBoard::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    g_sdlDevice->fillRectangle(colorf::BLACK + colorf::A_SHF(128), dstX, dstY, srcW, srcH);
    Widget::drawEx(dstX, dstY, srcX, srcY, srcW, srcH);
    g_sdlDevice->drawRectangle(colorf::WHITE + colorf::A_SHF(128), dstX, dstY, srcW, srcH);
}

Widget *MenuBoard::getSeparator()
{
    static Widget separator
    {
        DIR_UPLEFT,
        0,
        0,
    };
    return &separator;
}
