#pragma once
#include "widget.hpp"
#include "sdldevice.hpp"
#include "imageboard.hpp"

class CheckBox: public Widget
{
    private:
        uint32_t m_color;

    private:
        int &m_valRef;

    private:
        ImageBoard m_checkImage;

    public:
        CheckBox(
                dir8_t,
                int,
                int,
                int,
                int,

                uint32_t,
                int &,

                Widget * = nullptr, // parent
                bool     = false);  // auto-delete

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;

    public:
        void setColor(uint32_t color)
        {
            m_color = color;
        }

    private:
        static SDL_Texture *loadFunc(const ImageBoard *);
};
