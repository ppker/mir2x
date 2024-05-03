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

void FriendChatBoard::ChatPreviewPage::updateChatPreview(bool argGroup, uint32_t argDBID, const std::string &argMsg)
{
    ChatPreviewItem *child = dynamic_cast<ChatPreviewItem *>(canvas.hasChild([argGroup, argDBID](const Widget *widgetPtr, bool)
    {
        if(auto preview = dynamic_cast<const ChatPreviewItem *>(widgetPtr); preview && preview->group == argGroup && preview->dbid == argDBID){
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

            argGroup,
            argDBID,
            to_u8cstr(argMsg),

            &canvas, // image load func uses getParentBoard(this)
            true,
        };
    }

    canvas.moveFront(child);

    int startY = 0;
    canvas.foreachChild([&startY](Widget *widget, bool)
    {
        widget->moveAt(DIR_UPLEFT, 0, startY);
        startY += widget->h();
    });
}
