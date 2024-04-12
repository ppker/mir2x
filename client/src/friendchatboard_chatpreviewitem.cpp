#include "hero.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "friendchatboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

FriendChatBoard::ChatPreviewItem::ChatPreviewItem(dir8_t argDir,
        int argX,
        int argY,

        uint32_t argDBID,
        const char8_t *argChatXMLStr,

        Widget *argParent,
        bool   argAutoDelete)

    : Widget
      {
          argDir,
          argX,
          argY,

          ChatPreviewItem::WIDTH,
          ChatPreviewItem::HEIGHT,

          {},

          argParent,
          argAutoDelete,
      }

    , dbid(argDBID)
    , avatar
      {
          DIR_UPLEFT,
          0,
          0,

          ChatPreviewItem::AVATAR_WIDTH,
          ChatPreviewItem::HEIGHT,

          [this](const ImageBoard *) -> SDL_Texture *
          {
              if(this->dbid == SYS_CHATDBID_SYSTEM){
                  return g_progUseDB->retrieve(0X00001100);
              }

              auto boardPtr = FriendChatBoard::getParentBoard(this);
              const auto p = std::find_if(boardPtr->m_sdFriendList.begin(), boardPtr->m_sdFriendList.end(), [this](const auto &x)
              {
                  return this->dbid == x.dbid;
              });

              if(p == boardPtr->m_sdFriendList.end()){
                  return g_progUseDB->retrieve(0X010007CF);
              }

              return g_progUseDB->retrieve(Hero::faceGfxID(p->gender, p->job));
          },

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
          ChatPreviewItem::AVATAR_WIDTH + ChatPreviewItem::GAP,
          ChatPreviewItem::NAME_HEIGHT / 2,

          [this]() -> const char8_t *
          {
              if(this->dbid == SYS_CHATDBID_SYSTEM){
                  return u8"系统消息";
              }
              return u8"绝地武士";
          }(),

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
          ChatPreviewItem::AVATAR_WIDTH + ChatPreviewItem::GAP,
          ChatPreviewItem::NAME_HEIGHT,

          ChatPreviewItem::WIDTH - ChatPreviewItem::AVATAR_WIDTH - ChatPreviewItem::GAP,
          ChatPreviewItem::HEIGHT - ChatPreviewItem::NAME_HEIGHT,

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
{}

bool FriendChatBoard::ChatPreviewItem::processEvent(const SDL_Event &event, bool valid)
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
                    FriendChatBoard::getParentBoard(this)->setChatPageDBID(this->dbid);
                    FriendChatBoard::getParentBoard(this)->setUIPage(FriendChatBoard::UIPage_CHAT, name.getText(true).c_str());
                }
                return false;
            }
        default:
            {
                return false;
            }
    }
}
