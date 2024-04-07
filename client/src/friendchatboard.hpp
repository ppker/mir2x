#pragma once
#include <array>
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
        struct FriendItem: public Widget
        {
            //   ITEM_MARGIN                    | ITRM_MARGIN
            // ->| |<-                          v
            //   +--------------------------+ - -
            //   | +-----+                  | ^ -
            //   | |     | +------+         | | ^
            //   | | IMG | | NAME |         | | HEIGHT
            //   | |     | +------+         | |
            //   | +-----+                  | v
            //   +--------------------------+ -
            //         ->| |<-
            //           GAP
            //   |<------------------------>| UIPage_WIDTH - UIPage_MARGIN * 2

            constexpr static int HEIGHT = 40;
            constexpr static int ITEM_MARGIN = 3;
            constexpr static int AVATAR_WIDTH = (HEIGHT - ITEM_MARGIN * 2) * 84 / 94;

            constexpr static int GAP = 5;

            uint32_t dbid;
            ShapeClipBoard hovered;

            ImageBoard avatar;
            LabelBoard name;

            FriendItem(dir8_t,
                    int,
                    int,

                    uint32_t,
                    const char8_t *,
                    std::function<SDL_Texture *(const ImageBoard *)>,

                    Widget * = nullptr,
                    bool     = false);

            bool processEvent(const SDL_Event &, bool) override;
        };

        struct FriendListPage: public Widget
        {
            Widget canvas;
            FriendListPage(Widget::VarDir,
                    Widget::VarOffset,
                    Widget::VarOffset,

                    Widget * = nullptr,
                    bool     = false);

            void append(FriendItem *, bool);
        };

    public:
        enum UIPageType: int
        {
            UIPage_CHAT = 0,
            UIPage_CHATPREVIEW,
            UIPage_FRIENDLIST,
            UIPage_FRIENDSEARCH,
            UIPage_END,
        };

    private:
        struct UIPageWidgetGroup
        {
            LabelBoard * const title   = nullptr;
            Widget     * const control = nullptr;
            Widget     * const page    = nullptr;
        };

    private:
        static constexpr int UIPage_BORDER[4]
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
        int m_uiLastPage = UIPage_CHATPREVIEW;
        int m_uiPage     = UIPage_CHATPREVIEW;
        const std::array<UIPageWidgetGroup, UIPage_END> m_uiPageList; // {buttons, page}

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

    public:
        void setChatPageDBID(uint32_t);
        void setUIPage(int, const char * = nullptr);
};
