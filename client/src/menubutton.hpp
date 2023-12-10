#pragma once
#include <initializer_list>
#include "widget.hpp"
#include "buttonbase.hpp"

// menu button to expand a menu board
// this gui system does not support menubar and submenu
//
// +--------+
// |  Proj  |             <--- menu button
// +--------+----------+
// |  Open     CTRL+O  |  <--- menu board
// +-------------------+
// |  Save     CTRL+S  |
// +-------------------+

class MenuButton: public ButtonBase
{
    private:
        Widget *m_gfxWidget;
        Widget *m_menuBoard;

    private:
        const std::array<int, 4> m_margin;

    public:
        MenuButton(dir8_t,
                int,
                int,

                std::pair<Widget *, bool>,
                std::pair<Widget *, bool>,

                std::array<int, 4> = {},

                Widget * = nullptr,
                bool     = false);

    public:
        bool processEvent(const SDL_Event&, bool) override;
};
