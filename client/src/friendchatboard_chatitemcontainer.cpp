#include "hero.hpp"
#include "pngtexdb.hpp"
#include "processrun.hpp"
#include "friendchatboard.hpp"

extern PNGTexDB *g_progUseDB;

FriendChatBoard::ChatItemContainer::ChatItemContainer(dir8_t argDir,

        int argX,
        int argY,

        Widget::VarSize argH,

        Widget *argParent,
        bool    argAutoDelete)

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

void FriendChatBoard::ChatItemContainer::append(const SDChatMessage &sdCM, std::function<void(const FriendChatBoard::ChatItem *)> fnOp)
{
    FriendChatBoard::getParentBoard(this)->queryChatPeer(sdCM.group, sdCM.from, [sdCM, fnOp = std::move(fnOp), this](const SDChatPeer *peer)
    {
        if(!peer){
            return;
        }

        const auto chatPage = dynamic_cast<ChatPage *>(parent());
        const auto self = FriendChatBoard::getParentBoard(this)->m_processRun->getMyHero();

        if(sdCM.group && chatPage->peer.group && sdCM.to == chatPage->peer.dbid){
            // group chat
        }
        else if(!sdCM.group && !chatPage->peer.group && (sdCM.from == chatPage->peer.dbid || sdCM.to == chatPage->peer.dbid)){
            // personal chat
        }
        else{
            return;
        }

        auto chatItem = new ChatItem
        {
            DIR_UPLEFT,
            0,
            0,

            !sdCM.seq.has_value(),

            to_u8cstr(peer->name),
            to_u8cstr(cerealf::deserialize<std::string>(sdCM.message)),

            [from = sdCM.from, gender = peer->gender, job = peer->job](const ImageBoard *)
            {
                if     (from == SYS_CHATDBID_SYSTEM) return g_progUseDB->retrieve(0X00001100);
                else if(from == SYS_CHATDBID_GROUP ) return g_progUseDB->retrieve(0X00001300);
                else                                 return g_progUseDB->retrieve(Hero::faceGfxID(gender, job));
            },

            peer->dbid != self->dbid(),
            peer->dbid != self->dbid(),

            {},
        };

        const auto startY = canvas.hasChild() ? (canvas.h() + ChatItem::ITEM_SPACE) : 0;

        if(chatItem->avatarLeft){
            chatItem->moveAt(DIR_UPLEFT, 0, startY);
        }
        else{
            chatItem->moveAt(DIR_UPRIGHT, canvas.w() - 1, startY);
        }

        canvas.addChild(chatItem, true);
        if(fnOp){
            fnOp(chatItem);
        }

        dynamic_cast<ChatPage *>(parent())->placeholder.setShow(false);
    });
}
