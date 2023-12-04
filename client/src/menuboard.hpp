#pragma once
#include <initializer_list>
#include "widget.hpp"

class MenuBoard: public Widget
{
    private:
        const std::optional<int> m_wOpt;

    private:
        const int m_itemSpace;
        const int m_seperatorSpace;

    private:
        const std::array<int, 4> m_margin;

    public:
        MenuBoard(dir8_t,
                int,
                int,

                std::optional<int>,

                int,
                int,

                std::initializer_list<std::pair<Widget *, bool>>,

                std::array<int, 4>,

                Widget *,
                bool);

    public:
        void addChild(Widget *, bool) override;
};
