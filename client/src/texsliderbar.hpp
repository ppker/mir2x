#pragma once
#include <cstdint>
#include "widget.hpp"
#include "texslider.hpp"
#include "imageboard.hpp"
#include "imagecropboard.hpp"
#include "imagecropdupboard.hpp"

class TexSliderBar: public Widget
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

    public:
        float getValue() const
        {
            return m_slider.getValue();
        }

        void setValue(float val, bool triggerCallback)
        {
            m_slider.setValue(val, triggerCallback);
            m_barCropDup.setSize(
                    /* w */  m_slider.hslider() ? std::make_optional<int>(std::lround(val * (w() - 6))) : std::nullopt,
                    /* h */ !m_slider.hslider() ? std::make_optional<int>(std::lround(val * (h() - 6))) : std::nullopt);
        }
};
