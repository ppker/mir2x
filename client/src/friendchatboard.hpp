#pragma once
#include <array>
#include "widget.hpp"
#include "serdesmsg.hpp"
#include "texslider.hpp"
#include "inputline.hpp"
#include "imageboard.hpp"
#include "tritexbutton.hpp"
#include "gfxcropdupboard.hpp"

class ProcessRun;
class FriendChatBoard: public Widget
{
    constexpr static int UIPage_WIDTH  = 400; // the area excludes border area, margin included
    constexpr static int UIPage_HEIGHT = 400;
    constexpr static int UIPage_MARGIN =   4;

    private:
        struct FriendItem: public Widget
        {
            //   ITEM_MARGIN                    | ITRM_MARGIN
            // ->| |<-                          v
            //   +---------------------------+ - -
            //   | +-----+                   | ^ -
            //   | |     | +------+ +------+ | | ^
            //   | | IMG | | NAME | | FUNC | | | HEIGHT
            //   | |     | +------+ +------+ | |
            //   | +-----+                   | v
            //   +---------------------------+ -
            //         ->| |<-          -->| |<-- FUNC_MARGIN
            //           GAP
            //   |<------------------------->| UIPage_WIDTH - UIPage_MARGIN * 2

            constexpr static int HEIGHT = 40;
            constexpr static int ITEM_MARGIN = 3;
            constexpr static int AVATAR_WIDTH = (HEIGHT - ITEM_MARGIN * 2) * 84 / 94;

            constexpr static int GAP = 5;
            constexpr static int FUNC_MARGIN = 5;

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

                    std::pair<Widget *, bool> argFuncWidget = {},

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

        struct FriendSearchInputLine: public Widget
        {
            // o: (0,0)
            // x: (3,3) : fixed by gfx resource border
            //
            //   o==========================+ -
            //   | x+---+ +---------------+ | ^
            //   | ||O、| |xxxxxxx        | | | HEIGHT
            //   | ++---+ +---------------+ | v
            //   +==========================+ -
            //
            //   |<------- WIDTH --------->|
            //      |<->|
            //   ICON_WIDTH
            //
            //   ->||<- ICON_MARGIN
            //
            //       -->| |<-- GAP

            constexpr static int WIDTH = UIPage_WIDTH - UIPage_MARGIN * 2 - 60;
            constexpr static int HEIGHT = 30;

            constexpr static int ICON_WIDTH = 20;
            constexpr static int ICON_MARGIN = 5;
            constexpr static int GAP = 5;

            ImageBoard image;
            GfxCropDupBoard inputbg;

            ImageBoard icon;
            InputLine  input;
            LabelBoard hint;

            FriendSearchInputLine(Widget::VarDir,

                    Widget::VarOffset,
                    Widget::VarOffset,

                    Widget * = nullptr,
                    bool     = false);
        };

        struct FriendSearchAutoCompletionItem: public Widget
        {
            // o: (0,0)
            // x: (3,3)
            //
            //   o--------------------------+ -
            //   | x+---+ +---------------+ | ^
            //   | ||O、| |label          | | | HEIGHT
            //   | ++---+ +---------------+ | v
            //   +--------------------------+ -
            //   |<-------- WIDTH --------->|
            //      |<->|
            //   ICON_WIDTH
            //
            //   ->||<- ICON_MARGIN
            //
            //       -->| |<-- GAP

            constexpr static int WIDTH = UIPage_WIDTH - UIPage_MARGIN * 2;
            constexpr static int HEIGHT = 30;

            constexpr static int ICON_WIDTH = 20;
            constexpr static int ICON_MARGIN = 5;
            constexpr static int GAP = 5;

            const bool byID; // when clicked, fill input automatically by ID if true, or name if false
            const SDPlayerCandidate candidate;

            ShapeClipBoard background;

            ImageBoard icon;
            LabelBoard label;

            FriendSearchAutoCompletionItem(Widget::VarDir,

                    Widget::VarOffset,
                    Widget::VarOffset,

                    bool,
                    SDPlayerCandidate,

                    const char * = nullptr,

                    Widget * = nullptr,
                    bool     = false);

            bool processEvent(const SDL_Event &, bool) override;
        };

        struct FriendSearchPage: public Widget
        {
            //                  -->| |<-- CLEAR_GAP
            // |<----------WIDTH----------->|
            // +-------------------+ +------+
            // |      INPUT        | | 清空 |
            // +-------------------+ +------+
            // | auto completion item       |
            // +----------------------------+
            // | auto completion item       |
            // +----------------------------+

            constexpr static int WIDTH  = UIPage_WIDTH  - UIPage_MARGIN * 2;
            constexpr static int HEIGHT = UIPage_HEIGHT - UIPage_MARGIN * 2;

            constexpr static int CLEAR_GAP = 10;

            FriendSearchInputLine input;
            LayoutBoard clear;

            Widget autocompletes;
            Widget candidates;

            FriendSearchPage(Widget::VarDir,

                    Widget::VarOffset,
                    Widget::VarOffset,

                    Widget * = nullptr,
                    bool     = false);

            void appendCandidate(const SDPlayerCandidate &);
            void appendAutoCompletionItem(bool, const SDPlayerCandidate &, const std::string &);
        };

        struct FriendChatItem: public Widget
        {
            //          WIDTH
            // |<------------------->|
            //       GAP
            //     ->| |<-
            // +-----+     +--------+      -
            // |     |     |  name  |      | NAME_HEIGHT
            // |     |     +--------+      -
            // |     |     /-------------\             <----+
            // | IMG |     | ........... |                  |
            // |     |     | ........... |                  |
            // |     |    /  ........... |                  |
            // |     |  <    ........... |                  |
            // |     |    \  ........... |                  |
            // |     |  ^  | ........... |                  | background includes messsage round-corner background box and the triangle area
            // +-----+  |  | ........... |                  |
            //          |  | ........... |                  |
            //          |  \-------------/<- MESSAGE_CORNER |
            //          |            ->| |<-                |
            //          |             MESSAGE_MARGIN        |
            //          +-----------------------------------+
            //
            //
            //            -->|  |<-- TRIANGLE_WIDTH
            //                2 +                + 2                    -
            //      -----+     /|                |\     +-----          ^
            //           |    / |                | \    |               |
            //    avatar | 1 +  |                |  + 1 | avatar        | TRIANGLE_HEIGHT
            //           |    \ |                | /    |               |
            //      -----+     \|                |/     +-----          v
            //                3 +                + 3                    -
            //           |<---->|                |<---->|
            //           ^  GAP                     GAP ^
            //           |                              |
            //           +-- startX of background       +-- endX of background

            constexpr static int AVATAR_WIDTH  = 35;
            constexpr static int AVATAR_HEIGHT = AVATAR_WIDTH * 94 / 84;

            constexpr static int GAP = 5;
            constexpr static int ITEM_SPACE = 5;  // space between two items
            constexpr static int NAME_HEIGHT = 20;

            constexpr static int TRIANGLE_WIDTH  = 4;
            constexpr static int TRIANGLE_HEIGHT = 6;

            constexpr static int MAX_WIDTH = UIPage_WIDTH - UIPage_MARGIN * 2 - FriendChatItem::TRIANGLE_WIDTH - FriendChatItem::GAP - FriendChatItem::AVATAR_WIDTH;

            constexpr static int MESSAGE_MARGIN = 5;
            constexpr static int MESSAGE_CORNER = 3;

            constexpr static int MESSAGE_MIN_WIDTH  = 10; // handling small size message
            constexpr static int MESSAGE_MIN_HEIGHT = 10;

            double accuTime = 0.0;
            std::optional<uint64_t> idOpt {};

            const bool showName;
            const bool avatarLeft;
            const std::optional<uint32_t> bgColor;

            ImageBoard avatar;
            LabelBoard name;

            LayoutBoard    message;
            ShapeClipBoard background;

            FriendChatItem(dir8_t,
                    int,
                    int,

                    const char8_t *,
                    const char8_t *,

                    std::function<SDL_Texture *(const ImageBoard *)>,

                    bool,
                    bool,
                    std::optional<uint32_t>,

                    Widget * = nullptr,
                    bool     = false);

            void update(double) override;
        };

        struct FriendChatItemContainer: public Widget
        {
            // use canvas to hold all chat item
            // then we can align canvas always to buttom when needed
            //
            // when scroll we can only move canvas inside this container
            // no need to move chat item only by one
            //
            // canvas height is flexible
            // ShapeClipBoard can achieve this on drawing, but prefer ShapeClipBoard when drawing primitives

            Widget canvas;
            FriendChatItemContainer(dir8_t,

                    int,
                    int,

                    Widget::VarSize,

                    Widget * = nullptr,
                    bool     = false);

            void append(FriendChatItem *, bool);
            bool hasItem(const Widget *) const;
        };

        struct FriendChatInputContainer: public Widget
        {
            LayoutBoard layout;
            FriendChatInputContainer(dir8_t,

                    int,
                    int,

                    Widget * = nullptr,
                    bool     = false);
        };

        struct FriendChatPage: public Widget
        {
            // chat page is different, it uses the UIPage_MARGIN area
            // because we fill different color to chat area and input area
            //
            //         |<----- UIPage_WIDTH ------>|
            //       ->||<---- UIPage_MARGIN                     v
            //       - +---------------------------+             -
            //       ^ |+-------------------------+|           - -
            //       | || +------+                ||           ^ ^
            //       | || |******|                ||           | |
            //       | || +------+                ||           | + UIPage_MARGIN
            //       | ||                +------+ ||           |
            //  U    | ||                |******| ||           |
            //  I    | ||                +------+ ||           |
            //  P    | || +------------+          ||           +-- UIPage_HEIGHT - UIPage_MARGIN * 4 - INPUT_MARGIN * 2 - input.h() - 1
            //  a    | || |************|          ||           |
            //  g ---+ || |*****       |          ||           |
            //  e    | || +------------+          ||           |
            //  |    | ||                         ||           |
            //  H    | ||       chat area         ||         | |
            //  E    | ||                         ||         v v
            //  I    | |+-------------------------+|       | - -
            //  G    | +===========================+       v   UIPage_MARGIN * 2 + 1
            //  H    | |  +---------------------+  |       - -
            //  T    | | / +-------------------+ \ |     - -<- INPUT_MARGIN
            //       | ||  |*******************|  ||     ^ ^
            //       | ||  |****input area*****|  ||   | +---- input.h()
            //       | ||  |*******************|  || | v v
            //       | | \ +-------------------+ / | v - -
            //       v |  +---------------------+  | - -
            //       - +---------------------------+ - ^
            //       ->||<---- UIPage_MARGIN         ^ |
            //       -->| |<--  INPUT_CORNER         | +------  INPUT_MARGIN
            //       -->|  |<-  INPUT_MARGIN         +-------- UIPage_MARGIN
            //             |<--- input.w() --->|

            constexpr static int INPUT_CORNER = 8;
            constexpr static int INPUT_MARGIN = 8;

            constexpr static int INPUT_MIN_HEIGHT =  10;
            constexpr static int INPUT_MAX_HEIGHT = 200;

            uint32_t dbid = 0;
            ShapeClipBoard background;

            FriendChatInputContainer input;
            FriendChatItemContainer  chat;

            FriendChatPage(dir8_t,

                    int,
                    int,

                    Widget * = nullptr,
                    bool     = false);

            bool processEvent(const SDL_Event &, bool) override;
        };

        struct FriendChatPreviewItem: public Widget
        {
            constexpr static int WIDTH  = UIPage_WIDTH - UIPage_MARGIN * 2;
            constexpr static int HEIGHT = 50;

            constexpr static int GAP = 10;
            constexpr static int NAME_HEIGHT = 30;
            constexpr static int AVATAR_WIDTH = HEIGHT * 84 / 94; // original avatar size: 84 x 94

            //        GAP
            //       |<->|
            // +-+---+  +------+          -             -
            // |1|   |  | name |          | NAME_HEIGHT ^
            // +-+   |  +------+          -             | HEIGHT
            // | IMG |  +--------------+                |
            // |     |  |latest message|                v
            // +-----+  +--------------+                -
            //
            // |<--->|
            // AVATAR_WIDTH
            //
            // |<--------------------->|
            //           WIDTH

            const uint32_t dbid;

            ImageBoard  avatar;
            LabelBoard  name;
            LayoutBoard message;

            ShapeClipBoard preview;
            ShapeClipBoard selected;

            FriendChatPreviewItem(dir8_t,
                    int,
                    int,

                    uint32_t,
                    const char8_t *,

                    Widget * = nullptr,
                    bool     = false);

            bool processEvent(const SDL_Event &, bool) override;
        };

        struct FriendChatPreviewPage: public Widget
        {
            Widget canvas;
            FriendChatPreviewPage(Widget::VarDir,

                    Widget::VarOffset,
                    Widget::VarOffset,

                    Widget * = nullptr,
                    bool     = false);

            void append(FriendChatPreviewItem *, bool);
            void updateChatPreview(uint32_t, const std::string &);
        };

    private:
        struct PageControl: public Widget
        {
            PageControl(dir8_t,

                    int,
                    int,

                    int,

                    std::initializer_list<std::pair<Widget *, bool>>,

                    Widget * = nullptr,
                    bool     = false);
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

            std::function<void(int, Widget *)> enter = nullptr;
            std::function<void(int, Widget *)> exit  = nullptr;
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

    public:
        static       FriendChatBoard *getParentBoard(      Widget *);
        static const FriendChatBoard *getParentBoard(const Widget *);
};
