#pragma once
#include <functional>
#include <SDL2/SDL.h>
#include "widget.hpp"
#include "colorf.hpp"
#include "tritexbutton.hpp"

// ProgUse 0X00000450.PNG
// for corners are identical squares of size 58 x 58
// left side squares can be smaller but use same size for simplicity
//
//                       +------+
//                       |      |
//                       v      |
// +=--+---------------+--=+    |
// |   |               |   |<---+--- 58 x 58
// +---+---------------+---+
// |   |               |   |
// |   |               |   |
// |   |               |   |
// |   |               |   |
// +---+---------------+---+
// |   |               | O |
// +=--+---------------+--=+ (510 x 468)

class ResizableFrameBoard: public Widget
{
    private:
        static constexpr      int m_cornerSize = 58;
        static constexpr uint32_t m_frameTexID = 0X00000450;

    private:
        TritexButton m_close;

    public:
        ResizableFrameBoard(dir8_t, int, int, int, int, Widget *, bool);

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        void update(double) override;
        bool processEvent(const SDL_Event &, bool);

    public:
        void setSize(std::optional<int>, std::optional<int>);
};
