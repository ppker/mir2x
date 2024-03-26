#include <initializer_list>
#include "sdldevice.hpp"
#include "pngtexdb.hpp"
#include "processrun.hpp"
#include "friendchatboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

constexpr static int UIPage_WIDTH  = 400; // the area excludes border area, margin included
constexpr static int UIPage_HEIGHT = 400;
constexpr static int UIPage_MARGIN =   4;

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

    FriendItem(dir8_t argDir,
            int argX,
            int argY,

            uint32_t argDBID,
            const char8_t *argNameStr,

            std::function<SDL_Texture *(const ImageBoard *)> argLoadImageFunc,

            Widget *argParent  = nullptr,
            bool argAutoDelete = false)

        : Widget
          {
              argDir,
              argX,
              argY,

              UIPage_WIDTH - UIPage_MARGIN * 2,
              FriendItem::HEIGHT,
              {},

              argParent,
              argAutoDelete,
          }

        , dbid(argDBID)

        , hovered
          {
              DIR_UPLEFT,
              0,
              0,

              this->w(),
              this->h(),

              [this](const Widget *, int drawDstX, int drawDstY)
              {
                  if(const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc(); in(mousePX, mousePY)){
                      g_sdlDevice->fillRectangle(colorf::GREEN              + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
                      g_sdlDevice->drawRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
                  }
                  else{
                      g_sdlDevice->drawRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(32), drawDstX, drawDstY, w(), h());
                  }
              },

              this,
              false,
          }

        , avatar
          {
              DIR_UPLEFT,
              FriendItem::ITEM_MARGIN,
              FriendItem::ITEM_MARGIN,

              FriendItem::AVATAR_WIDTH,
              FriendItem::HEIGHT - FriendItem::ITEM_MARGIN * 2,

              std::move(argLoadImageFunc),

              false,
              false,
              0,

              colorf::WHITE + colorf::A_SHF(0XFF),

              this,
              false,
          }

        , name
          {
              DIR_LEFT,
              FriendItem::ITEM_MARGIN + FriendItem::AVATAR_WIDTH + FriendItem::GAP,
              FriendItem::HEIGHT / 2,

              argNameStr,

              1,
              14,
              0,
              colorf::WHITE + colorf::A_SHF(255),

              this,
              false,
          }
    {}

    bool processEvent(const SDL_Event &event, bool valid) override
    {
        if(!valid){
            return consumeFocus(false);
        }

        if(!show()){
            return consumeFocus(false);
        }

        switch(event.type){
            case SDL_MOUSEBUTTONDOWN:
                {
                    if(in(event.button.x, event.button.y)){
                        dynamic_cast<FriendChatBoard *>(this->parent()->parent()->parent())->setUIPage(FriendChatBoard::UIPage_CHAT, name.getText(true).c_str());
                    }
                    return false;
                }
            default:
                {
                    return false;
                }
        }
    }
};

struct FriendListPage: public Widget
{
    Widget canvas;
    FriendListPage(Widget::VarDir argDir,

            Widget::VarOffset argX,
            Widget::VarOffset argY,

            Widget *argParent     = nullptr,
            bool    argAutoDelete = false)

        : Widget
          {
              std::move(argDir),
              std::move(argX),
              std::move(argY),

              UIPage_WIDTH  - UIPage_MARGIN * 2,
              UIPage_HEIGHT - UIPage_MARGIN * 2,

              {},

              argParent,
              argAutoDelete,
          }

        , canvas
          {
              DIR_UPLEFT,
              0,
              0,

              this->w(),
              {},
              {},

              this,
              false,
          }
    {}

    void append(FriendItem *item, bool autoDelete)
    {
        item->moveAt(DIR_UPLEFT, 0, canvas.h());
        canvas.addChild(item, autoDelete);
    }
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

    const bool showName;
    const bool avatarLeft;
    const uint32_t bgColor;

    ImageBoard avatar;
    LabelBoard name;

    LayoutBoard    message;
    ShapeClipBoard background;

    FriendChatItem(dir8_t argDir,
            int argX,
            int argY,

            const char8_t *argNameStr,
            const char8_t *argMessageStr,

            std::function<SDL_Texture *(const ImageBoard *)> argLoadImageFunc,

            bool argShowName,
            bool argAvatarLeft,
            uint32_t argBGColor,

            Widget *argParent  = nullptr,
            bool argAutoDelete = false)

        : Widget
          {
              argDir,
              argX,
              argY,

              {},
              {},
              {},

              argParent,
              argAutoDelete,
          }

        , showName(argShowName)
        , avatarLeft(argAvatarLeft)
        , bgColor(argBGColor)

        , avatar
          {
              DIR_UPLEFT,
              0,
              0,

              FriendChatItem::AVATAR_WIDTH,
              FriendChatItem::AVATAR_HEIGHT,

              std::move(argLoadImageFunc),
          }

        , name
          {
              DIR_UPLEFT,
              0,
              0,

              argNameStr,

              1,
              10,
          }

        , message
          {
              DIR_UPLEFT,
              0,
              0,
              FriendChatItem::MAX_WIDTH - FriendChatItem::AVATAR_WIDTH - FriendChatItem::GAP - FriendChatItem::TRIANGLE_WIDTH - FriendChatItem::MESSAGE_MARGIN * 2,

              to_cstr(argMessageStr),
              0,

              {},
              false,
              false,
              false,
              false,

              1,
              12,
          }

        , background
          {
              DIR_UPLEFT,
              0,
              0,

              FriendChatItem::MESSAGE_MARGIN * 2 + std::max<int>(message.w(), FriendChatItem::MESSAGE_MIN_WIDTH ) + FriendChatItem::TRIANGLE_WIDTH,
              FriendChatItem::MESSAGE_MARGIN * 2 + std::max<int>(message.h(), FriendChatItem::MESSAGE_MIN_HEIGHT),

              [this](const Widget *, int drawDstX, int drawDstY)
              {
                  g_sdlDevice->fillRectangle(
                          bgColor,

                          drawDstX + (avatarLeft ? FriendChatItem::TRIANGLE_WIDTH : 0),
                          drawDstY,

                          std::max<int>(message.w(), FriendChatItem::MESSAGE_MIN_WIDTH ) + FriendChatItem::MESSAGE_MARGIN * 2,
                          std::max<int>(message.h(), FriendChatItem::MESSAGE_MIN_HEIGHT) + FriendChatItem::MESSAGE_MARGIN * 2,

                          FriendChatItem::MESSAGE_CORNER);

                  const auto triangleX1_avatarLeft = drawDstX;
                  const auto triangleX2_avatarLeft = drawDstX + FriendChatItem::TRIANGLE_WIDTH - 1;
                  const auto triangleX3_avatarLeft = drawDstX + FriendChatItem::TRIANGLE_WIDTH - 1;

                  const auto triangleX1_avatarRight = drawDstX + FriendChatItem::MESSAGE_MARGIN * 2 + std::max<int>(message.w(), FriendChatItem::MESSAGE_MIN_WIDTH) + FriendChatItem::TRIANGLE_WIDTH - 1;
                  const auto triangleX2_avatarRight = drawDstX + FriendChatItem::MESSAGE_MARGIN * 2 + std::max<int>(message.w(), FriendChatItem::MESSAGE_MIN_WIDTH);
                  const auto triangleX3_avatarRight = drawDstX + FriendChatItem::MESSAGE_MARGIN * 2 + std::max<int>(message.w(), FriendChatItem::MESSAGE_MIN_WIDTH);

                  const auto triangleY1_showName = drawDstY + (FriendChatItem::AVATAR_HEIGHT - FriendChatItem::NAME_HEIGHT) / 2;
                  const auto triangleY2_showName = drawDstY + (FriendChatItem::AVATAR_HEIGHT - FriendChatItem::NAME_HEIGHT) / 2 - FriendChatItem::TRIANGLE_HEIGHT / 2;
                  const auto triangleY3_showName = drawDstY + (FriendChatItem::AVATAR_HEIGHT - FriendChatItem::NAME_HEIGHT) / 2 + FriendChatItem::TRIANGLE_HEIGHT / 2;

                  const auto triangleY1_hideName = drawDstY + FriendChatItem::AVATAR_HEIGHT / 2;
                  const auto triangleY2_hideName = drawDstY + FriendChatItem::AVATAR_HEIGHT / 2 - FriendChatItem::TRIANGLE_HEIGHT / 2;
                  const auto triangleY3_hideName = drawDstY + FriendChatItem::AVATAR_HEIGHT / 2 + FriendChatItem::TRIANGLE_HEIGHT / 2;

                  if(avatarLeft){
                      if(showName) g_sdlDevice->fillTriangle(bgColor, triangleX1_avatarLeft, triangleY1_showName, triangleX2_avatarLeft, triangleY2_showName, triangleX3_avatarLeft, triangleY3_showName);
                      else         g_sdlDevice->fillTriangle(bgColor, triangleX1_avatarLeft, triangleY1_hideName, triangleX2_avatarLeft, triangleY2_hideName, triangleX3_avatarLeft, triangleY3_hideName);
                  }
                  else{
                      if(showName) g_sdlDevice->fillTriangle(bgColor, triangleX1_avatarRight, triangleY1_showName, triangleX2_avatarRight, triangleY2_showName, triangleX3_avatarRight, triangleY3_showName);
                      else         g_sdlDevice->fillTriangle(bgColor, triangleX1_avatarRight, triangleY1_hideName, triangleX2_avatarRight, triangleY2_hideName, triangleX3_avatarRight, triangleY3_hideName);
                  }
              },
          }
    {
        const auto fnMoveAdd = [this](Widget *widgetPtr, dir8_t dstDir, int dstX, int dstY)
        {
            widgetPtr->moveAt(dstDir, dstX, dstY);
            addChild(widgetPtr, false);
        };

        if(avatarLeft){
            fnMoveAdd(&avatar, DIR_UPLEFT, 0, 0);
            if(showName){
                fnMoveAdd(&name      , DIR_LEFT  ,                  FriendChatItem::AVATAR_WIDTH + FriendChatItem::GAP + FriendChatItem::TRIANGLE_WIDTH                                 , FriendChatItem::NAME_HEIGHT / 2                             );
                fnMoveAdd(&background, DIR_UPLEFT,                  FriendChatItem::AVATAR_WIDTH + FriendChatItem::GAP                                                                  , FriendChatItem::NAME_HEIGHT                                 );
                fnMoveAdd(&message   , DIR_UPLEFT,                  FriendChatItem::AVATAR_WIDTH + FriendChatItem::GAP + FriendChatItem::TRIANGLE_WIDTH + FriendChatItem::MESSAGE_MARGIN, FriendChatItem::NAME_HEIGHT + FriendChatItem::MESSAGE_MARGIN);
            }
            else{
                fnMoveAdd(&background, DIR_UPLEFT,                  FriendChatItem::AVATAR_WIDTH + FriendChatItem::GAP                                                                  , 0                                                           );
                fnMoveAdd(&message   , DIR_UPLEFT,                  FriendChatItem::AVATAR_WIDTH + FriendChatItem::GAP + FriendChatItem::TRIANGLE_WIDTH + FriendChatItem::MESSAGE_MARGIN, FriendChatItem::MESSAGE_MARGIN                              );
            }
        }
        else{
            const auto realWidth = FriendChatItem::AVATAR_WIDTH + FriendChatItem::GAP + FriendChatItem::TRIANGLE_WIDTH + std::max<int>(message.w(), FriendChatItem::MESSAGE_MIN_WIDTH) + FriendChatItem::MESSAGE_MARGIN * 2;
            fnMoveAdd(&avatar, DIR_UPRIGHT, realWidth - 1, 0);

            if(showName){
                fnMoveAdd(&name      , DIR_RIGHT  , realWidth - 1 - FriendChatItem::AVATAR_WIDTH - FriendChatItem::GAP - FriendChatItem::TRIANGLE_WIDTH                                 , FriendChatItem::NAME_HEIGHT / 2                             );
                fnMoveAdd(&background, DIR_UPRIGHT, realWidth - 1 - FriendChatItem::AVATAR_WIDTH - FriendChatItem::GAP                                                                  , FriendChatItem::NAME_HEIGHT                                 );
                fnMoveAdd(&message   , DIR_UPRIGHT, realWidth - 1 - FriendChatItem::AVATAR_WIDTH - FriendChatItem::GAP - FriendChatItem::TRIANGLE_WIDTH - FriendChatItem::MESSAGE_MARGIN, FriendChatItem::NAME_HEIGHT + FriendChatItem::MESSAGE_MARGIN);
            }
            else{
                fnMoveAdd(&background, DIR_UPRIGHT, realWidth - 1 - FriendChatItem::AVATAR_WIDTH - FriendChatItem::GAP                                                                  , 0                                                           );
                fnMoveAdd(&message   , DIR_UPRIGHT, realWidth - 1 - FriendChatItem::AVATAR_WIDTH - FriendChatItem::GAP - FriendChatItem::TRIANGLE_WIDTH - FriendChatItem::MESSAGE_MARGIN, FriendChatItem::MESSAGE_MARGIN                              );
            }
        }
    }
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
        FriendChatItemContainer(dir8_t argDir,

                int argX,
                int argY,

                Widget::VarSize argH,

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

            : Widget
              {
                  argDir,
                  argX,
                  argY,

                  UIPage_WIDTH - UIPage_MARGIN * 2,
                  std::move(argH),

                  {},

                  argParent,
                  argAutoDelete,
              }

            , canvas
              {
                  DIR_DOWNLEFT,
                  0,
                  [this](const Widget *self)
                  {
                      return std::min<int>(self->h(), this->h()) - 1;
                  },

                  this->w(),
                  {},

                  {},

                  this,
                  false,
              }
        {}

        void append(FriendChatItem *chatItem, bool autoDelete)
        {
            const auto startY = canvas.hasChild() ? (canvas.h() + FriendChatItem::ITEM_SPACE) : 0;
            if(chatItem->avatarLeft){
                chatItem->moveAt(DIR_UPLEFT, 0, startY);
            }
            else{
                chatItem->moveAt(DIR_UPRIGHT, canvas.w() - 1, startY);
            }

            canvas.addChild(chatItem, autoDelete);
        }
    };

    struct FriendChatInputContainer: public Widget
    {
        LayoutBoard layout;
        FriendChatInputContainer(dir8_t argDir,

                int argX,
                int argY,

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

            : Widget
              {
                  argDir,
                  argX,
                  argY,

                  UIPage_WIDTH - UIPage_MARGIN * 2 - FriendChatPage::INPUT_MARGIN * 2,
                  [this](const Widget *)
                  {
                      return mathf::bound<int>(layout.h(), FriendChatPage::INPUT_MIN_HEIGHT, FriendChatPage::INPUT_MAX_HEIGHT);
                  },

                  {},

                  argParent,
                  argAutoDelete,
              }

            , layout
              {
                  DIR_UPLEFT,
                  0,
                  0,

                  this->w(),

                  "<layout><par>正在输入的内容。。。</par></layout>",
                  0,

                  {},
                  false,
                  true,
                  true,
                  false,

                  1,
                  12,
                  0,
                  colorf::WHITE + colorf::A_SHF(255),
                  0,

                  LALIGN_JUSTIFY,
                  0,
                  0,

                  nullptr,
                  nullptr,
                  nullptr,

                  this,
                  false,
              }
        {
            // there is mutual dependency
            // height of input container depends on height of layout
            //
            // layout always attach to buttom of input container, so argX needs container height
            // in initialization list we can not call this->h() since initialization of layout is not done yet
            layout.moveAt(DIR_DOWNLEFT, 0, this->h() - 1);
        }
    };

    ShapeClipBoard background;

    FriendChatInputContainer input;
    FriendChatItemContainer  chat;

    FriendChatPage(dir8_t argDir,

            int argX,
            int argY,

            Widget *argParent,
            bool argAutoDelete)

        : Widget
          {
              argDir,
              argX,
              argY,

              UIPage_WIDTH,
              UIPage_HEIGHT,

              {},

              argParent,
              argAutoDelete,
          }

        , background
          {
              DIR_UPLEFT,
              0,
              0,
              UIPage_WIDTH,
              UIPage_HEIGHT,

              [this](const Widget *, int drawDstX, int drawDstY)
              {
                  g_sdlDevice->drawLine(
                          colorf::RGBA(231, 231, 189, 64),

                          drawDstX,
                          drawDstY + UIPage_HEIGHT - UIPage_MARGIN * 2 - INPUT_MARGIN * 2 - input.h() - 1,

                          drawDstX + UIPage_WIDTH,
                          drawDstY + UIPage_HEIGHT - UIPage_MARGIN * 2 - INPUT_MARGIN * 2 - input.h() - 1);

                  g_sdlDevice->fillRectangle(
                          colorf::RGBA(231, 231, 189, 32),

                          drawDstX,
                          drawDstY + UIPage_HEIGHT - UIPage_MARGIN * 2 - INPUT_MARGIN * 2 - input.h(),

                          UIPage_WIDTH,
                          UIPage_MARGIN * 2 + FriendChatPage::INPUT_MARGIN * 2 + input.h());

                  g_sdlDevice->fillRectangle(
                          colorf::BLACK + colorf::A_SHF(255),

                          drawDstX + UIPage_MARGIN,
                          drawDstY + UIPage_HEIGHT - UIPage_MARGIN - INPUT_MARGIN * 2 - input.h(),

                          UIPage_WIDTH - UIPage_MARGIN * 2,
                          FriendChatPage::INPUT_MARGIN * 2 + input.h(),

                          FriendChatPage::INPUT_CORNER);

                  g_sdlDevice->drawRectangle(
                          colorf::RGBA(231, 231, 189, 96),

                          drawDstX + UIPage_MARGIN,
                          drawDstY + UIPage_HEIGHT - UIPage_MARGIN - INPUT_MARGIN * 2 - input.h(),

                          UIPage_WIDTH - UIPage_MARGIN * 2,
                          FriendChatPage::INPUT_MARGIN * 2 + input.h(),

                          FriendChatPage::INPUT_CORNER);
              },

              this,
              false,
          }

        , input
          {
              DIR_DOWNLEFT,
              UIPage_MARGIN                 + FriendChatPage::INPUT_MARGIN,
              UIPage_HEIGHT - UIPage_MARGIN - FriendChatPage::INPUT_MARGIN - 1,

              this,
              false,
          }

        , chat
          {
              DIR_UPLEFT,
              UIPage_MARGIN,
              UIPage_MARGIN,

              [this](const Widget *)
              {
                  return UIPage_HEIGHT - UIPage_MARGIN * 4 - FriendChatPage::INPUT_MARGIN * 2 - input.h() - 1;
              },

              this,
              false,
          }
    {
        chat.append(new FriendChatItem
        {
            DIR_UPLEFT,
            0,
            0,

            u8"绝地武士",
            u8"<layout><par>你好呀，好久没你消息了！</par></layout>",

            [](const ImageBoard *)
            {
                return g_progUseDB->retrieve(0X02000000);
            },

            true,
            true,

            colorf::RED + colorf::A_SHF(128),
        }, true);

        chat.append(new FriendChatItem
        {
            DIR_UPLEFT,
            0,
            100,

            u8"恭喜发财",
            u8"<layout><par>今天是大年初一，祝你新年发财！</par><par>老家的人都很想念你。</par><par>祝好！</par></layout>",

            [](const ImageBoard *)
            {
                return g_progUseDB->retrieve(0X02000001);
            },

            false,
            false,

            colorf::GREEN + colorf::A_SHF(128),
        }, true);

        chat.append(new FriendChatItem
        {
            DIR_UPLEFT,
            0,
            0,

            u8"绝地武士",
            u8"<layout><par>好的，谢谢！</par></layout>",

            [](const ImageBoard *)
            {
                return g_progUseDB->retrieve(0X02000000);
            },

            true,
            true,

            colorf::RED + colorf::A_SHF(128),
        }, true);
    }
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

    ImageBoard  avatar;
    LabelBoard  name;
    LayoutBoard message;

    ShapeClipBoard preview;
    ShapeClipBoard selected;

    FriendChatPreviewItem(dir8_t argDir,
            int argX,
            int argY,

            const char8_t *argNameStr,
            const char8_t *argChatXMLStr,

            std::function<SDL_Texture *(const ImageBoard *)> argLoadImageFunc,

            Widget *argParent    = nullptr,
            bool   argAutoDelete = false)

        : Widget
          {
              argDir,
              argX,
              argY,

              FriendChatPreviewItem::WIDTH,
              FriendChatPreviewItem::HEIGHT,

              {},

              argParent,
              argAutoDelete,
          }

        , avatar
          {
              DIR_UPLEFT,
              0,
              0,

              FriendChatPreviewItem::AVATAR_WIDTH,
              FriendChatPreviewItem::HEIGHT,

              std::move(argLoadImageFunc),

              false,
              false,
              0,

              colorf::WHITE + colorf::A_SHF(0XFF),

              this,
              false,
          }

        , name
          {
              DIR_LEFT,
              FriendChatPreviewItem::AVATAR_WIDTH + FriendChatPreviewItem::GAP,
              FriendChatPreviewItem::NAME_HEIGHT / 2,

              argNameStr,

              1,
              14,
              0,
              colorf::WHITE + colorf::A_SHF(255),

              this,
              false,
          }

        , message
          {
              DIR_UPLEFT,
              0,
              0,
              0, // line width

              to_cstr(argChatXMLStr),
              1,

              {},
              false,
              false,
              false,
              false,

              1,
              12,
              0,
              colorf::GREY + colorf::A_SHF(255),
          }

        , preview
          {
              DIR_UPLEFT,
              FriendChatPreviewItem::AVATAR_WIDTH + FriendChatPreviewItem::GAP,
              FriendChatPreviewItem::NAME_HEIGHT,

              FriendChatPreviewItem::WIDTH - FriendChatPreviewItem::AVATAR_WIDTH - FriendChatPreviewItem::GAP,
              FriendChatPreviewItem::HEIGHT - FriendChatPreviewItem::NAME_HEIGHT,

              [this](const Widget *, int drawDstX, int drawDstY)
              {
                  message.drawEx(drawDstX, drawDstY, 0, 0, message.w(), message.h());
              },

              this,
              false,
          }

        , selected
          {
              DIR_UPLEFT,
              0,
              0,

              this->w(),
              this->h(),

              [this](const Widget *, int drawDstX, int drawDstY)
              {
                  if(const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc(); in(mousePX, mousePY)){
                      g_sdlDevice->fillRectangle(colorf::GREEN              + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
                      g_sdlDevice->drawRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
                  }
                  else{
                      g_sdlDevice->drawRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(32), drawDstX, drawDstY, w(), h());
                  }
              },

              this,
              false,
          }
    {}

    bool processEvent(const SDL_Event &event, bool valid) override
    {
        if(!valid){
            return consumeFocus(false);
        }

        if(!show()){
            return consumeFocus(false);
        }

        switch(event.type){
            case SDL_MOUSEBUTTONDOWN:
                {
                    if(in(event.button.x, event.button.y)){
                        dynamic_cast<FriendChatBoard *>(this->parent()->parent()->parent())->setUIPage(FriendChatBoard::UIPage_CHAT, name.getText(true).c_str());
                    }
                    return false;
                }
            default:
                {
                    return false;
                }
        }
    }
};

struct FriendChatPreviewPage: public Widget
{
    Widget canvas;
    FriendChatPreviewPage(Widget::VarDir argDir,

            Widget::VarOffset argX,
            Widget::VarOffset argY,

            Widget *argParent     = nullptr,
            bool    argAutoDelete = false)

        : Widget
          {
              std::move(argDir),
              std::move(argX),
              std::move(argY),

              UIPage_WIDTH  - UIPage_MARGIN * 2,
              UIPage_HEIGHT - UIPage_MARGIN * 2,

              {},

              argParent,
              argAutoDelete,
          }

        , canvas
          {
              DIR_UPLEFT,
              0,
              0,

              this->w(),
              {},
              {},

              this,
              false,
          }
    {}

    void append(FriendChatPreviewItem *item, bool autoDelete)
    {
        item->moveAt(DIR_UPLEFT, 0, canvas.h());
        canvas.addChild(item, autoDelete);
    }
};

struct PageControl: public Widget
{
    PageControl(dir8_t argDir,

            int argX,
            int argY,

            int argSpace,

            std::initializer_list<std::pair<Widget *, bool>> argChildList,

            Widget *argParent     = nullptr,
            bool    argAutoDelete = false)

        : Widget
          {
              argDir,
              argX,
              argY,

              {},
              {},
              {},

              argParent,
              argAutoDelete,
          }
    {
        int offX = 0;
        for(auto &[widgetPtr, autoDelete]: argChildList){
            addChild(widgetPtr, autoDelete);
            widgetPtr->moveAt(DIR_UPLEFT, offX, (h() - widgetPtr->h()) / 2);

            offX += widgetPtr->w();
            offX += argSpace;
        }
    }
};

FriendChatBoard::FriendChatBoard(int argX, int argY, ProcessRun *runPtr, Widget *widgetPtr, bool autoDelete)
    : Widget
      {
          DIR_UPLEFT,
          argX,
          argY,

          UIPage_BORDER[2] + UIPage_WIDTH  + UIPage_BORDER[3],
          UIPage_BORDER[0] + UIPage_HEIGHT + UIPage_BORDER[1],

          {},

          widgetPtr,
          autoDelete,
      }

    , m_processRun(runPtr)
    , m_frame
      {
          DIR_UPLEFT,
          0,
          0,
          {},
          {},
          [](const ImageBoard *){ return g_progUseDB->retrieve(0X00000800); },
      }

    , m_frameCropDup
      {
          DIR_UPLEFT,
          0,
          0,
          this->w(),
          this->h(),

          &m_frame,

          55,
          95,
          285 - 55,
          345 - 95,

          this,
          false,
      }

    , m_background
      {
          DIR_UPLEFT,
          0,
          0,
          {},
          {},
          [](const ImageBoard *){ return g_progUseDB->retrieve(0X00000810); },

          false,
          false,
          0,
          colorf::RGBA(160, 160, 160, 255),
      }

    , m_backgroundCropDup
      {
          DIR_UPLEFT,
          0,
          0,
          m_frameCropDup.w(),
          m_frameCropDup.h(),

          &m_background,

          0,
          0,
          510,
          187,

          this,
          false,
      }

    , m_slider
      {
          DIR_UPLEFT,
          m_frameCropDup.w() - 30,
          70,
          9,
          m_frameCropDup.h() - 140,

          false,
          3,
          nullptr,

          this,
          false,
      }

    , m_close
      {
          DIR_UPLEFT,
          m_frameCropDup.w() - 38,
          m_frameCropDup.h() - 40,
          {SYS_U32NIL, 0X0000001C, 0X0000001D},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this](ButtonBase *)
          {
              setShow(false);
          },

          0,
          0,
          0,
          0,

          true,
          true,
          this,
      }

    , m_uiPageList
      {
          UIPageWidgetGroup // UIPage_CHAT
          {
              .title = new LabelBoard
              {
                  DIR_NONE,
                  45 + (m_frameCropDup.w() - 45 - 190) / 2,
                  29,

                  u8"好友名称",
                  1,
                  14,
                  0,colorf::WHITE + colorf::A_SHF(255),

                  this,
                  true,
              },

              .control = new PageControl
              {
                  DIR_RIGHT,
                  m_frameCropDup.w() - 42,
                  29,
                  2,

                  {
                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X000008F0, 0X000008F0, 0X000008F1},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              [this](ButtonBase *)
                              {
                                  setUIPage(UIPage_CHATPREVIEW);
                              },
                          },

                          true,
                      },

                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X00000023, 0X00000023, 0X00000024},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              [this](ButtonBase *)
                              {
                              },
                          },

                          true,
                      },

                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X000008B0, 0X000008B0, 0X000008B1},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              [this](ButtonBase *)
                              {
                              },
                          },

                          true,
                      },

                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X00000590, 0X00000590, 0X00000591},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              [this](ButtonBase *)
                              {
                              },
                          },

                          true,
                      },

                  },

                  this,
                  true,
              },

              .page = new FriendChatPage
              {
                  DIR_UPLEFT,
                  UIPage_BORDER[2],
                  UIPage_BORDER[0],

                  this,
                  true,
              },
          },

          UIPageWidgetGroup // UIPage_CHATPREVIEW
          {
              .title = new LabelBoard
              {
                  DIR_NONE,
                  45 + (m_frameCropDup.w() - 45 - 190) / 2,
                  29,

                  u8"【聊天记录】",
                  1,
                  14,
                  0,colorf::WHITE + colorf::A_SHF(255),

                  this,
                  true,
              },

              .control = new PageControl
              {
                  DIR_RIGHT,
                  m_frameCropDup.w() - 42,
                  28,
                  2,

                  {
                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X00000160, 0X00000160, 0X00000161},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              [this](ButtonBase *)
                              {
                                  setUIPage(UIPage_FRIENDLIST);
                              },
                          },

                          true,
                      },
                  },

                  this,
                  true,
              },

              .page = new FriendChatPreviewPage
              {
                  DIR_UPLEFT,
                  UIPage_BORDER[2] + UIPage_MARGIN,
                  UIPage_BORDER[0] + UIPage_MARGIN,

                  this,
                  true,
              },
          },

          UIPageWidgetGroup // UIPage_FRIENDLIST
          {
              .title = new LabelBoard
              {
                  DIR_NONE,
                  45 + (m_frameCropDup.w() - 45 - 190) / 2,
                  29,

                  u8"【好友列表】",
                  1,
                  14,
                  0,colorf::WHITE + colorf::A_SHF(255),

                  this,
                  true,
              },

              .control = new PageControl
              {
                  DIR_RIGHT,
                  m_frameCropDup.w() - 42,
                  28,
                  2,

                  {
                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X000008F0, 0X000008F0, 0X000008F1},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              [this](ButtonBase *)
                              {
                                  setUIPage(UIPage_CHATPREVIEW);
                              },
                          },

                          true,
                      },

                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X00000170, 0X00000170, 0X00000171},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              [this](ButtonBase *)
                              {
                              },
                          },

                          true,
                      },
                  },

                  this,
                  true,
              },

              .page = new FriendListPage
              {
                  DIR_UPLEFT,
                  UIPage_BORDER[2] + UIPage_MARGIN,
                  UIPage_BORDER[0] + UIPage_MARGIN,

                  this,
                  true,
              },
          },
      }
{
    setShow(false);

    dynamic_cast<FriendListPage *>(m_uiPageList[UIPage_FRIENDLIST].page)->append(new FriendItem
    {
        DIR_UPLEFT,
        0,
        0,

        123,
        u8"绝地武士",

        [](const ImageBoard *)
        {
            return g_progUseDB->retrieve(0X02000000);
        },
    }, true);

    dynamic_cast<FriendListPage *>(m_uiPageList[UIPage_FRIENDLIST].page)->append(new FriendItem
    {
        DIR_UPLEFT,
        0,
        0,

        123,
        u8"尤达大师的妈妈",

        [](const ImageBoard *)
        {
            return g_progUseDB->retrieve(0X02000001);
        },
    }, true);

    dynamic_cast<FriendChatPreviewPage *>(m_uiPageList[UIPage_CHATPREVIEW].page)->append(new FriendChatPreviewItem
    {
        DIR_UPLEFT,
        0,
        0,

        u8"绝地武士",
        u8"<layout><par>你好呀！</par></layout>",

        [](const ImageBoard *)
        {
            return g_progUseDB->retrieve(0X02000000);
        },
    }, true);

    dynamic_cast<FriendChatPreviewPage *>(m_uiPageList[UIPage_CHATPREVIEW].page)->append(new FriendChatPreviewItem
    {
        DIR_UPLEFT,
        0,
        0,

        u8"尤达大师的妈妈",
        u8"<layout><par>祝你发财！</par><par>收到请回复，谢谢！</par></layout>",

        [](const ImageBoard *)
        {
            return g_progUseDB->retrieve(0X02000001);
        },
    }, true);
}

void FriendChatBoard::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    for(const auto &p:
    {
        static_cast<const Widget *>(&m_backgroundCropDup),
        static_cast<const Widget *>( m_uiPageList[m_uiPage].page),
        static_cast<const Widget *>(&m_frameCropDup),
        static_cast<const Widget *>( m_uiPageList[m_uiPage].title),
        static_cast<const Widget *>( m_uiPageList[m_uiPage].control),
        static_cast<const Widget *>(&m_slider),
        static_cast<const Widget *>(&m_close),
    }){
        int drawSrcX = srcX;
        int drawSrcY = srcY;
        int drawSrcW = srcW;
        int drawSrcH = srcH;
        int drawDstX = dstX;
        int drawDstY = dstY;

        if(mathf::cropChildROI(
                    &drawSrcX, &drawSrcY,
                    &drawSrcW, &drawSrcH,
                    &drawDstX, &drawDstY,

                    w(),
                    h(),

                    p->dx(),
                    p->dy(),
                    p-> w(),
                    p-> h())){
            p->drawEx(drawDstX, drawDstY, drawSrcX, drawSrcY, drawSrcW, drawSrcH);
        }
    }
}

bool FriendChatBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
    }

    if(m_close                        .processEvent(event, valid)){ return true; }
    if(m_slider                       .processEvent(event, valid)){ return true; }
    if(m_uiPageList[m_uiPage].control->processEvent(event, valid)){ return true; }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                if(focus()){
                    switch(event.key.keysym.sym){
                        case SDLK_ESCAPE:
                            {
                                setShow(false);
                                setFocus(false);
                                return true;
                            }
                        default:
                            {
                                return false;
                            }
                    }
                }
                return false;
            }
        case SDL_MOUSEMOTION:
            {
                if((event.motion.state & SDL_BUTTON_LMASK) && (in(event.motion.x, event.motion.y) || focus())){
                    const auto [rendererW, rendererH] = g_sdlDevice->getRendererSize();
                    const int maxX = rendererW - w();
                    const int maxY = rendererH - h();

                    const int newX = std::max<int>(0, std::min<int>(maxX, x() + event.motion.xrel));
                    const int newY = std::max<int>(0, std::min<int>(maxY, y() + event.motion.yrel));
                    moveBy(newX - x(), newY - y());
                    return consumeFocus(true);
                }
                return consumeFocus(false);
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                if(m_uiPageList[m_uiPage].page->in(event.button.x, event.button.y)){
                    if(m_uiPageList[m_uiPage].page->processEvent(event, true)){
                        return consumeFocus(true, m_uiPageList[m_uiPage].page);
                    }
                }
                return consumeFocus(in(event.button.x, event.button.y));
            }
        case SDL_MOUSEWHEEL:
            {
                if(m_uiPageList[m_uiPage].page->focus()){
                    if(m_uiPageList[m_uiPage].page->processEvent(event, true)){
                        return consumeFocus(true, m_uiPageList[m_uiPage].page);
                    }
                }
                return false;
            }
        default:
            {
                return false;
            }
    }
}

void FriendChatBoard::setFriendList(const SDFriendList &)
{}

void FriendChatBoard::addMessage(const SDChatMessage &newmsg)
{
    const auto saveInDBID = [&newmsg, this]
    {
        if(newmsg.from == m_processRun->getMyHero()->dbid()){
            return newmsg.to;
        }
        else if(newmsg.to == m_processRun->getMyHero()->dbid()){
            return newmsg.from;
        }
        else{
            return m_processRun->getMyHero()->dbid();
        }
    }();

    auto p = std::find_if(m_friendMessageList.begin(), m_friendMessageList.end(), [saveInDBID](const auto &item)
    {
        return item.dbid == saveInDBID;
    });

    if(p == m_friendMessageList.end()){
        m_friendMessageList.emplace_front(saveInDBID);
    }
    else if(p != m_friendMessageList.begin()){
        m_friendMessageList.splice(m_friendMessageList.begin(), m_friendMessageList, p, std::next(p));
    }

    p = m_friendMessageList.begin();
    if(std::find_if(p->list.begin(), p->list.end(), [&newmsg](const auto &msg){ return msg.id == newmsg.id; }) == p->list.end()){
        p->unread++;
        p->list.push_back(newmsg);
        std::sort(p->list.begin(), p->list.end(), [](const auto &x, const auto &y)
        {
            return x.timestamp < y.timestamp;
        });
    }
}

void FriendChatBoard::setUIPage(int uiPage, const char *titleStr)
{
    fflassert(uiPage >= 0, uiPage);
    fflassert(uiPage < UIPage_END, uiPage);

    if(m_uiPage != uiPage){
        m_uiLastPage = m_uiPage;
        m_uiPage     = uiPage;

        m_uiPageList[m_uiLastPage].page->setFocus(false);
        m_uiPageList[m_uiPage    ].page->setFocus(true );


        if(titleStr){
            m_uiPageList[m_uiPage].title->setText(to_u8cstr(titleStr));
        }
    }
}
