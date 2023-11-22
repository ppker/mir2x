#pragma once
#include <cstdint>
#include <SDL2/SDL.h>
#include "totype.hpp"
#include "widget.hpp"
#include "texslider.hpp"

class TexSliderBar: public TexSlider
{
    public:
        TexSliderBar(dir8_t,
                int,
                int,
                int,

                bool,
                int,

                std::function<void(float)>,

                Widget * = nullptr,
                bool  = false);

    public:
        void drawEx(int, int, int, int, int, int) const override;
};
