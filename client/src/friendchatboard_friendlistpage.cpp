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

void FriendChatBoard::FriendListPage::append(const SDChatPeer &peer)
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

        [this](const FriendChatBoard::FriendItem *item)
        {
            auto boardPtr = FriendChatBoard::getParentBoard(this);
            auto self = boardPtr->m_processRun->getMyHero();

            if(item->dbid == boardPtr->m_processRun->getMyHero()->dbid()){
                boardPtr->setChatPeer(SDChatPeer
                {
                    .dbid   = self->dbid(),
                    .name   = self->getName(),
                    .gender = self->gender(),
                    .job    = self->job(),

                }, true);
            }
            else if(item->dbid == SYS_CHATDBID_SYSTEM){
                boardPtr->setChatPeer(SDChatPeer
                {
                    .dbid   = SYS_CHATDBID_SYSTEM,
                    .name   = "系统助手",

                }, true);
            }
            else if(auto peer = boardPtr->findFriend(item->dbid)){
                boardPtr->setChatPeer(*peer, true);
            }
            else{
                throw fflerror("item is not associated with a friend: dbid %llu", to_llu(item->dbid));
            }

            boardPtr->setUIPage(FriendChatBoard::UIPage_CHAT);
            boardPtr->m_processRun->requestLatestChatMessage({item->dbid}, 50, true, true);
        },
    };

    friendItem->moveAt(DIR_UPLEFT, 0, canvas.h());
    canvas.addChild(friendItem, true);
}
