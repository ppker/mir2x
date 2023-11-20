#pragma once
#include "widget.hpp"
#include "sdldevice.hpp"
#include "checkbox.hpp"
#include "labelboard.hpp"

class CheckLabel: public Widget
{
    private:
        CheckBox m_checkBox;
        LabelBoard m_labelBoard;

    public:
        CheckLabel(
                dir8_t,
                int,
                int,

                int &,
                const char8_t *,

                Widget * = nullptr, // parent
                bool     = false);  // auto-delete

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;
};
