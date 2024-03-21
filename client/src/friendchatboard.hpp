#pragma once
#include "widget.hpp"
#include "serdesmsg.hpp"
#include "texslider.hpp"
#include "imageboard.hpp"
#include "tritexbutton.hpp"
#include "gfxcropdupboard.hpp"

class ProcessRun;
class FriendChatBoard: public Widget
{
    private:
        static constexpr int UIPage_CHAT        = 1;
        static constexpr int UIPage_CHATPREVIEW = 2;
        static constexpr int UIPage_FRIENDLIST  = 3;

    private:
        static constexpr int UIPage_MARGIN[4]
        {
            54,
            10,
            13,
            38,
        };

    private:
        struct FriendMessage
        {
            uint32_t dbid   = 0;
            size_t   unread = 0;

            std::vector<SDChatMessage> list;
        };

    private:
        int m_uiPage = UIPage_CHATPREVIEW;

    private:
        ProcessRun *m_processRun;

    private:
        ImageBoard m_frame;
        GfxCropDupBoard m_frameCropDup;

    private:
        ImageBoard m_background;
        GfxCropDupBoard m_backgroundCropDup;

    private:
        TexSlider m_slider;

    private:
        TritexButton m_close;

    private:
        Widget *m_UIPage_CHATPREVIEW;

    private:
        std::list<FriendMessage> m_friendMessageList;

    public:
        FriendChatBoard(int, int, ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;

    public:
        void addMessage(const SDChatMessage &);
        void setFriendList(const SDFriendList &);
};
