#include "sdldevice.hpp"
#include "processrun.hpp"
#include "friendchatboard.hpp"

extern SDLDevice *g_sdlDevice;

FriendChatBoard::FriendItem::FriendItem(dir8_t argDir,
        int argX,
        int argY,

        uint32_t argDBID,
        const char8_t *argNameStr,

        std::function<SDL_Texture *(const ImageBoard *)> argLoadImageFunc,

        std::pair<Widget *, bool> argFuncWidget,

        Widget *argParent,
        bool argAutoDelete)

    : Widget
      {
          argDir,
          argX,
          argY,

          UIPage_WIDTH - UIPage_MARGIN * 2,
          FriendItem::HEIGHT,

          {
              {
                  argFuncWidget.first,
                  DIR_RIGHT,
                  UIPage_WIDTH - UIPage_MARGIN * 2 - FriendItem::FUNC_MARGIN - 1,
                  FriendItem::HEIGHT / 2,
                  argFuncWidget.second,
              },
          },

          argParent,
          argAutoDelete,
      }

    , dbid(argDBID)
    , widgetID(argFuncWidget.first ? argFuncWidget.first->id() : 0)

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
                  g_sdlDevice->fillRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
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

void FriendChatBoard::FriendItem::setFuncWidget(Widget *argFuncWidget, bool argAutoDelete)
{
    clearChild([this](const Widget *widget, bool)
    {
        return this->widgetID == widget->id();
    });

    addChild(argFuncWidget, argAutoDelete);
}

bool FriendChatBoard::FriendItem::processEvent(const SDL_Event &event, bool valid)
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
                if(Widget::processEvent(event, valid)){
                    return consumeFocus(true);
                }
                else if(in(event.button.x, event.button.y)){
                    auto boardPtr = FriendChatBoard::getParentBoard(this);
                    if(this->dbid == boardPtr->m_processRun->getMyHero()->dbid()){
                        boardPtr->setChatPeer(SDChatPeer
                        {
                            .dbid   = boardPtr->m_processRun->getMyHero()->dbid(),
                            .name   = boardPtr->m_processRun->getMyHero()->getName(),
                            .gender = boardPtr->m_processRun->getMyHero()->gender(),
                            .job    = boardPtr->m_processRun->getMyHero()->job(),

                        }, true);
                    }
                    else if(this->dbid == SYS_CHATDBID_SYSTEM){
                        boardPtr->setChatPeer(SDChatPeer
                        {
                            .dbid   = SYS_CHATDBID_SYSTEM,
                            .name   = "系统助手",

                        }, true);
                    }
                    else if(auto peer = boardPtr->findFriend(this->dbid)){
                        boardPtr->setChatPeer(*peer, true);
                    }
                    else{
                        throw fflerror("item is not associated with a friend: dbid %llu", to_llu(this->dbid));
                    }

                    boardPtr->setUIPage(FriendChatBoard::UIPage_CHAT);
                    boardPtr->m_processRun->requestLatestChatMessage({this->dbid}, 50, true, true);
                    return consumeFocus(true);
                }
                else{
                    return false;
                }
            }
        default:
            {
                return Widget::processEvent(event, valid);
            }
    }
}
