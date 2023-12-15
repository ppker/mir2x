#pragma once
#include <tuple>
#include <string>
#include "mathf.hpp"
#include "widget.hpp"
#include "menuboard.hpp"
#include "imageboard.hpp"
#include "gfxcropdupboard.hpp"
#include "menubutton.hpp"
#include "checklabel.hpp"
#include "labelboard.hpp"
#include "texslider.hpp"
#include "tritexbutton.hpp"
#include "texsliderbar.hpp"
#include "shapeclipboard.hpp"
#include "resizableframeboard.hpp"

class ProcessRun;
class RuntimeConfigBoard: public Widget
{
    private:
        SDRuntimeConfig m_sdRuntimeConfig;

    private:
        ResizableFrameBoard m_frameBoard;

    private:
        CheckLabel m_checkLabel;
        TexSliderBar m_texSliderBar;
        TexSliderBar m_texSliderBarVertical;

    private:
        TexSliderBar m_musicSlider;
        TexSliderBar m_soundEffectSlider;

    private:
        LabelBoard m_item1;
        LabelBoard m_item2;
        CheckLabel m_itemCheckLabel;

        MenuBoard m_menuBoard;

    private:
        TritexButton   m_menuExpandButton;
        ShapeClipBoard m_menuExpandButtonBox;

    private:
        ImageBoard      m_menuImage;
        GfxCropDupBoard m_menuImageCropDup;

        LabelBoard m_menuButtonTitle;
        Widget     m_menuButtonTitleGfx;
        MenuButton m_menuButton;

    private:
        ShapeClipBoard m_leftMenuBackground;
        LayoutBoard    m_leftMenu;

    private:
        ProcessRun *m_processRun;

    public:
        RuntimeConfigBoard(int, int, int, int, ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;

    protected:
        void drawEntryTitle(const char8_t *, int, int) const;

    private:
        void reportRuntimeConfig();

    public:
        const SDRuntimeConfig &getConfig() const
        {
            return m_sdRuntimeConfig;
        }

    public:
        void setConfig(SDRuntimeConfig);
};
