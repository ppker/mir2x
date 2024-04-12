#include "friendchatboard.hpp"

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
