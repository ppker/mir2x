#pragma once
#include <cstdint>
#include "widget.hpp"
#include "sdldevice.hpp"
#include "imageboard.hpp"

class CheckBox: public Widget
{
    private:
        uint32_t m_color;

    private:
        int64_t  m_innVal = 0;
        int64_t *m_valPtr;

    private:
        std::function<void(Widget *)> m_onChange;

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

                int64_t *,
                std::function<void(Widget *)>,

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

    public:
        void toggle()
        {
            *m_valPtr = !(*m_valPtr);
            if(m_onChange){
                m_onChange(this);
            }
        }

    public:
        bool checkedValue() const
        {
            return *m_valPtr;
        }

    private:
        static SDL_Texture *loadFunc(const ImageBoard *);
};
