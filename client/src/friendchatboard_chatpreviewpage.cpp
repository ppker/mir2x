#include "friendchatboard.hpp"

FriendChatBoard::ChatPreviewPage::ChatPreviewPage(Widget::VarDir argDir,

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

void FriendChatBoard::ChatPreviewPage::append(ChatPreviewItem *item, bool autoDelete)
{
    item->moveAt(DIR_UPLEFT, 0, canvas.h());
    canvas.addChild(item, autoDelete);
}

void FriendChatBoard::ChatPreviewPage::updateChatPreview(uint32_t argDBID, const std::string &argMsg)
{
    ChatPreviewItem *child = dynamic_cast<ChatPreviewItem *>(canvas.hasChild([argDBID](const Widget *widgetPtr, bool)
    {
        if(auto preview = dynamic_cast<const ChatPreviewItem *>(widgetPtr); preview && preview->dbid == argDBID){
            return true;
        }
        return false;
    }));

    if(child){
        child->message.loadXML(argMsg.c_str());
    }
    else{
        child = new ChatPreviewItem
        {
            DIR_UPLEFT,
            0,
            0,

            argDBID,
            to_u8cstr(argMsg),

            this, // image load func uses getParentBoard(this)
            true,
        };
        append(child, true);
    }

    canvas.moveFront(child);

    int startY = 0;
    foreachChild([&startY](Widget *widget, bool)
    {
        widget->moveAt(DIR_UPLEFT, 0, startY);
        startY += widget->h();
    });
}
