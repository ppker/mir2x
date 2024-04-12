#include "hero.hpp"
#include "pngtexdb.hpp"
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

void FriendChatBoard::ChatItemContainer::append(uint32_t argDBID, const std::string &argMsg)
{
    if(const auto chatPage = dynamic_cast<ChatPage *>(parent()); chatPage->dbid != argDBID){
        throw fflerror("append chat message from %llu to chat page which is used for another friend %llu", to_llu(argDBID), to_llu(chatPage->dbid));
    }

    append(new ChatItem
    {
        DIR_UPLEFT,
        0,
        0,

        [argDBID, this]() -> const char8_t *
        {
            if(argDBID == SYS_CHATDBID_SYSTEM){
                return u8"系统消息";
            }

            const auto boardPtr = FriendChatBoard::getParentBoard(this);
            const auto p = std::find_if(boardPtr->m_sdFriendList.begin(), boardPtr->m_sdFriendList.end(), [argDBID](const auto &x)
            {
                return argDBID == x.dbid;
            });

            if(p == boardPtr->m_sdFriendList.end()){
                return u8"未知";
            }

            return to_u8cstr(p->name);
        }(),

        to_u8cstr(argMsg),

        [argDBID, this](const ImageBoard *)
        {
            if(argDBID == SYS_CHATDBID_SYSTEM){
                return g_progUseDB->retrieve(0X00001100);
            }

            const auto boardPtr = FriendChatBoard::getParentBoard(this);
            const auto p = std::find_if(boardPtr->m_sdFriendList.begin(), boardPtr->m_sdFriendList.end(), [argDBID](const auto &x)
            {
                return argDBID == x.dbid;
            });

            if(p == boardPtr->m_sdFriendList.end()){
                return g_progUseDB->retrieve(0X010007CF);
            }

            return g_progUseDB->retrieve(Hero::faceGfxID(p->gender, p->job));
        },

        true,
        true,

        {},
    }, true);
}

void FriendChatBoard::ChatItemContainer::append(ChatItem *chatItem, bool autoDelete)
{
    const auto startY = canvas.hasChild() ? (canvas.h() + ChatItem::ITEM_SPACE) : 0;
    if(chatItem->avatarLeft){
        chatItem->moveAt(DIR_UPLEFT, 0, startY);
    }
    else{
        chatItem->moveAt(DIR_UPRIGHT, canvas.w() - 1, startY);
    }

    canvas.addChild(chatItem, autoDelete);
    dynamic_cast<ChatPage *>(parent())->placeholder.setShow(false);
}

bool FriendChatBoard::ChatItemContainer::hasItem(const Widget *item) const
{
    return canvas.hasChild(item);
}
