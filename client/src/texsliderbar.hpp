#pragma once
#include <cstdint>
#include "widget.hpp"
#include "texslider.hpp"
#include "imageboard.hpp"
#include "imagecropboard.hpp"
#include "imagecropdupboard.hpp"

class TexSliderBar: public WidgetContainer
{
    private:
        TexSlider m_slider;

    private:
        ImageBoard m_slotImage;
        ImageBoard m_barImage;

    private:
        ImageCropBoard m_barCrop;

    private:
        ImageCropBoard m_slotCropLeft;
        ImageCropBoard m_slotCropMiddle;
        ImageCropBoard m_slotCropRight;

    private:
        ImageCropDupBoard m_barCropDup;
        ImageCropDupBoard m_slotMidCropDup;

    public:
        TexSliderBar(dir8_t,
                int,
                int,
                int,

                bool,
                int,

                std::function<void(float)>,

                Widget * = nullptr,
                bool     = false);
};
