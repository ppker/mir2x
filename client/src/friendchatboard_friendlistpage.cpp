#include "hero.hpp"
#include "pngtexdb.hpp"
#include "processrun.hpp"
#include "friendchatboard.hpp"

extern PNGTexDB *g_progUseDB;

FriendChatBoard::FriendListPage::FriendListPage(Widget::VarDir argDir,

        Widget::VarOffset argX,
        Widget::VarOffset argY,

        Widget *argParent,
        bool    argAutoDelete)

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

void FriendChatBoard::FriendListPage::append(const SDChatPeer &peer, std::function<void(FriendChatBoard::FriendItem *)> argOnClick, std::pair<Widget *, bool> argFuncWidget)
{
    auto friendItem = new FriendItem
    {
        DIR_UPLEFT,
        0,
        0,

        peer.dbid,
        to_u8cstr(peer.name),

        [peer](const ImageBoard *)
        {
            if     (peer.group                      ) return g_progUseDB->retrieve(0X00001300);
            else if(peer.dbid == SYS_CHATDBID_SYSTEM) return g_progUseDB->retrieve(0X00001100);
            else                                      return g_progUseDB->retrieve(Hero::faceGfxID(peer.gender, peer.job));
        },

        std::move(argOnClick),
        std::move(argFuncWidget),
    };

    friendItem->moveAt(DIR_UPLEFT, 0, canvas.h());
    canvas.addChild(friendItem, true);
}
