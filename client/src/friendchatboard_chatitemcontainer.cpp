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

void FriendChatBoard::ChatItemContainer::append(uint32_t argDBID, std::optional<uint64_t> argIDOpt, bool argAvatarLeft, const std::string &argMsg)
{
    if(argDBID == SYS_CHATDBID_SYSTEM){
        append(new ChatItem
        {
            DIR_UPLEFT,
            0,
            0,

            argIDOpt,
            u8"系统消息",
            to_u8cstr(argMsg),

            [](const ImageBoard *)
            {
                return g_progUseDB->retrieve(0X00001100);
            },

            true,
            argAvatarLeft,

            {},
        }, true);
    }
    else{
        FriendChatBoard::getParentBoard(this)->queryPlayerCandidate(argDBID, [argIDOpt, argAvatarLeft, argMsg, this](const SDPlayerCandidate *candidate)
        {
            if(!candidate){
                return;
            }

            this->append(new ChatItem
            {
                DIR_UPLEFT,
                0,
                0,

                argIDOpt,
                to_u8cstr(candidate->name),
                to_u8cstr(argMsg),

                [gender = candidate->gender, job = candidate->job, this](const ImageBoard *)
                {
                    return g_progUseDB->retrieve(Hero::faceGfxID(gender, job));
                },

                true,
                argAvatarLeft,

                {},
            }, true);
        });
    }
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
