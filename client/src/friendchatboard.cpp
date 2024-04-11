#include <initializer_list>
#include "sdldevice.hpp"
#include "client.hpp"
#include "pngtexdb.hpp"
#include "processrun.hpp"
#include "friendchatboard.hpp"

extern Client *g_client;
extern PNGTexDB *g_progUseDB;
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
                  g_sdlDevice->fillRectangle(colorf::GREEN              + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
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
                    dynamic_cast<FriendChatBoard *>(this->parent(3))->setUIPage(FriendChatBoard::UIPage_CHAT, name.getText(true).c_str());
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

void FriendChatBoard::FriendListPage::append(FriendItem *item, bool autoDelete)
{
    item->moveAt(DIR_UPLEFT, 0, canvas.h());
    canvas.addChild(item, autoDelete);
}

FriendChatBoard::FriendSearchInputLine::FriendSearchInputLine(Widget::VarDir argDir,

        Widget::VarOffset argX,
        Widget::VarOffset argY,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),

          FriendSearchInputLine::WIDTH,
          FriendSearchInputLine::HEIGHT,

          {},

          argParent,
          argAutoDelete,
      }

    , image
      {
          DIR_UPLEFT,
          0,
          0,
          {},
          {},
          [](const ImageBoard *){ return g_progUseDB->retrieve(0X00000460); },
      }

    , inputbg
      {
          DIR_UPLEFT,
          0,
          0,

          this->w(),
          this->h(),

          &image,

          3,
          3,
          image.w() - 6,
          2,

          this,
          false,
      }

    , icon
      {
          DIR_NONE,
          FriendSearchInputLine::ICON_WIDTH / 2 + FriendSearchInputLine::ICON_MARGIN + 3,
          FriendSearchInputLine::HEIGHT     / 2,

          std::min<int>(FriendSearchInputLine::ICON_WIDTH, FriendSearchInputLine::HEIGHT - 3 * 2),
          std::min<int>(FriendSearchInputLine::ICON_WIDTH, FriendSearchInputLine::HEIGHT - 3 * 2),

          [](const ImageBoard *) { return g_progUseDB->retrieve(0X00001200); },

          false,
          false,
          0,

          colorf::WHITE + colorf::A_SHF(0XFF),

          this,
          false,
      }

    , input
      {
          DIR_UPLEFT,
          3 + FriendSearchInputLine::ICON_MARGIN + FriendSearchInputLine::ICON_WIDTH + FriendSearchInputLine::GAP,
          3,

          FriendSearchInputLine::WIDTH  - 3 * 2 - FriendSearchInputLine::ICON_MARGIN - FriendSearchInputLine::ICON_WIDTH - FriendSearchInputLine::GAP,
          FriendSearchInputLine::HEIGHT - 3 * 2,

          true,

          1,
          14,

          0,
          colorf::WHITE + colorf::A_SHF(255),

          2,
          colorf::WHITE + colorf::A_SHF(255),

          nullptr,
          [this]()
          {
              dynamic_cast<FriendSearchPage *>(parent())->candidates.setShow(true);
              dynamic_cast<FriendSearchPage *>(parent())->autocompletes.setShow(false);
          },

          [this](std::string query)
          {
              hint.setShow(query.empty());

              if(query.empty()){
                  dynamic_cast<FriendSearchPage *>(parent())->candidates.clearChild();
                  dynamic_cast<FriendSearchPage *>(parent())->autocompletes.clearChild();

                  dynamic_cast<FriendSearchPage *>(parent())->candidates.setShow(false);
                  dynamic_cast<FriendSearchPage *>(parent())->autocompletes.setShow(true);
              }
              else{
                  CMQueryPlayerCandidates cmQPC;
                  std::memset(&cmQPC, 0, sizeof(cmQPC));

                  cmQPC.input.assign(query);
                  g_client->send({CM_QUERYPLAYERCANDIDATES, cmQPC}, [query = std::move(query), this](uint8_t headCode, const uint8_t *data, size_t size)
                  {
                      switch(headCode){
                          case SM_OK:
                            {
                                dynamic_cast<FriendSearchPage *>(parent())->candidates.clearChild();
                                dynamic_cast<FriendSearchPage *>(parent())->autocompletes.clearChild();

                                for(const auto &candidate: cerealf::deserialize<SDPlayerCandidateList>(data, size)){
                                    dynamic_cast<FriendSearchPage *>(parent())->appendCandidate(candidate);
                                    dynamic_cast<FriendSearchPage *>(parent())->appendAutoCompletionItem(query == std::to_string(candidate.dbid), candidate, [&candidate, &query]
                                    {
                                        if(const auto pos = candidate.name.find(query); pos != std::string::npos){
                                            return str_printf(R"###(<par>%s<t color="red">%s</t>%s（%llu）</par>)###", candidate.name.substr(0, pos).c_str(), query.c_str(), candidate.name.substr(pos + query.size()).c_str(), to_llu(candidate.dbid));
                                        }
                                        else if(std::to_string(candidate.dbid) == query){
                                            return str_printf(R"###(<par>%s（<t color="red">%llu</t>）</par>)###", candidate.name.c_str(), to_llu(candidate.dbid));
                                        }
                                        else{
                                            return str_printf(R"###(<par>%s（%llu）</par>)###", candidate.name.c_str(), to_llu(candidate.dbid));
                                        }
                                    }());
                                }
                                break;
                            }
                        default:
                            {
                                throw fflerror("query failed in server");
                            }
                      }
                  });
              }
          },

          this,
          false,
      }

    , hint
      {
          this->input.dir(),
          this->input.dx(),
          this->input.dy(),

          u8"输入用户ID或角色名",
          1,
          14,
          0,

          colorf::GREY + colorf::A_SHF(255),

          this,
          false,
      }
{}

FriendChatBoard::FriendSearchAutoCompletionItem::FriendSearchAutoCompletionItem(Widget::VarDir argDir,

        Widget::VarOffset argX,
        Widget::VarOffset argY,

        bool argByID,
        SDPlayerCandidate argCandidate,

        const char *argLabelXMLStr,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),

          FriendSearchAutoCompletionItem::WIDTH,
          FriendSearchAutoCompletionItem::HEIGHT,

          {},

          argParent,
          argAutoDelete,
      }

    , byID(argByID)
    , candidate(std::move(argCandidate))

    , background
      {
          DIR_UPLEFT,
          0,
          0,

          this->w(),
          this->h(),

          [this](const Widget *, int drawDstX, int drawDstY)
          {
              if(const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc(); in(mousePX, mousePY)){
                  g_sdlDevice->fillRectangle(colorf::GREEN              + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
                  g_sdlDevice->drawRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
              }
              else{
                  g_sdlDevice->fillRectangle(colorf::GREY               + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
                  g_sdlDevice->drawRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(32), drawDstX, drawDstY, w(), h());
              }
          },

          this,
          false,
      }

    , icon
      {
          DIR_NONE,
          FriendSearchAutoCompletionItem::ICON_WIDTH / 2 + FriendSearchAutoCompletionItem::ICON_MARGIN + 3,
          FriendSearchAutoCompletionItem::HEIGHT     / 2,

          std::min<int>(FriendSearchAutoCompletionItem::ICON_WIDTH, FriendSearchAutoCompletionItem::HEIGHT - 3 * 2),
          std::min<int>(FriendSearchAutoCompletionItem::ICON_WIDTH, FriendSearchAutoCompletionItem::HEIGHT - 3 * 2),

          [](const ImageBoard *) { return g_progUseDB->retrieve(0X00001200); },

          false,
          false,
          0,

          colorf::WHITE + colorf::A_SHF(0XFF),

          this,
          false,
      }

    , label
      {
          DIR_UPLEFT,
          3 + FriendSearchAutoCompletionItem::ICON_MARGIN + FriendSearchAutoCompletionItem::ICON_WIDTH + FriendSearchAutoCompletionItem::GAP,
          3,

          u8"",

          1,
          14,
          0,
          colorf::WHITE + colorf::A_SHF(255),

          this,
          false,
      }
{
    if(str_haschar(argLabelXMLStr)){
        label.loadXML(argLabelXMLStr);
    }
    else{
        label.loadXML(str_printf(R"###(<par>%s（%llu）</par>)###", candidate.name.c_str(), to_llu(candidate.dbid)).c_str());
    }
}

bool FriendChatBoard::FriendSearchAutoCompletionItem::processEvent(const SDL_Event &event, bool valid)
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
                    dynamic_cast<FriendSearchPage *>(this->parent(2))->input.input.setInput(byID ? std::to_string(candidate.dbid).c_str() : candidate.name.c_str());
                    return consumeFocus(true);
                }
                return false;
            }
        default:
            {
                return Widget::processEvent(event, valid);
            }
    }
}

FriendChatBoard::FriendSearchPage::FriendSearchPage(Widget::VarDir argDir,

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

    , input
      {
          DIR_UPLEFT,
          0,
          0,

          this,
          false,
      }

    , clear
      {
          DIR_LEFT,
          input.dx() + input.w() + FriendSearchPage::CLEAR_GAP,
          input.dy() + input.h() / 2,

          -1,

          R"###(<layout><par><event id="clear">清空</event></par></layout>)###",
          0,

          {},

          false,
          false,
          false,
          false,

          1,
          15,

          0,
          colorf::WHITE + colorf::A_SHF(255),
          0,

          LALIGN_LEFT,
          0,
          0,

          0,
          0,

          nullptr,
          nullptr,
          [this](const std::unordered_map<std::string, std::string> &attrList, int oldState, int newState)
          {
              if(oldState == BEVENT_DOWN && newState == BEVENT_ON){
                  const auto fnFindAttrValue = [&attrList](const char *key, const char *valDefault) -> const char *
                  {
                      if(auto p = attrList.find(key); p != attrList.end() && str_haschar(p->second)){
                          return p->second.c_str();
                      }
                      return valDefault;
                  };

                  if(const auto id = fnFindAttrValue("id", nullptr)){
                      input.input.clear();
                  }
              }
          },

          this,
          false,
      }

    , autocompletes
      {
          DIR_UPLEFT,
          0,
          FriendSearchInputLine::HEIGHT,

          FriendSearchPage::WIDTH,
          FriendSearchPage::HEIGHT - FriendSearchInputLine::HEIGHT,

          {},

          this,
          false,
      }

    , candidates
      {
          DIR_UPLEFT,
          0,
          FriendSearchInputLine::HEIGHT,

          FriendSearchPage::WIDTH,
          FriendSearchPage::HEIGHT - FriendSearchInputLine::HEIGHT,

          {},

          this,
          false,
      }
{
    candidates.setShow(false);
}

void FriendChatBoard::FriendSearchPage::appendCandidate(const SDPlayerCandidate &candidate)
{
    int maxY = 0;
    candidates.foreachChild([&maxY](const Widget *widget, bool)
    {
        maxY = std::max<int>(maxY, widget->dy() + widget->h());
    });

    candidates.addChild(new FriendItem
    {
        DIR_UPLEFT,
        0,
        maxY,

        candidate.dbid,
        to_u8cstr(candidate.name),

        [](const ImageBoard *)
        {
            return g_progUseDB->retrieve(0X00001100);
        },

        {
            new LayoutBoard
            {
                DIR_UPLEFT,
                0,
                0,
                0,

                R"###(<layout><par><event id="add">添加</event></par></layout>)###",
                0,

                {},

                false,
                false,
                false,
                false,

                1,
                12,
                0,
                colorf::WHITE + colorf::A_SHF(255),
                0,

                LALIGN_LEFT,
                0,
                0,

                0,
                0,

                nullptr,
                nullptr,
                [dbid = candidate.dbid, name = candidate.name, this](const std::unordered_map<std::string, std::string> &attrList, int oldState, int newState)
                {
                    if(oldState == BEVENT_ON && newState == BEVENT_DOWN){
                        if(const auto id = LayoutBoard::findAttrValue(attrList, "id"); to_sv(id) == "add"){
                            CMAddFriend cmAF;
                            std::memset(&cmAF, 0, sizeof(cmAF));

                            cmAF.dbid = dbid;
                            g_client->send({CM_ADDFRIEND, cmAF}, [dbid, name, this](uint8_t headCode, const uint8_t *buf, size_t bufSize)
                            {
                                switch(headCode){
                                    case SM_OK:
                                        {
                                            switch(const auto sdAFN = cerealf::deserialize<SDAddFriendNotif>(buf, bufSize); sdAFN.notif){
                                                case AF_ACCEPTED:
                                                    {
                                                        dynamic_cast<FriendChatPreviewPage *>(FriendChatBoard::getParentBoard(this)->m_uiPageList[UIPage_CHATPREVIEW].page)->updateChatPreview(dbid, str_printf(R"###(<par><t color="red">%s</t>已经通过你的好友申请，现在可以开始聊天了。</par>)###", to_cstr(name)));
                                                        break;
                                                    }
                                                case AF_EXIST:
                                                    {
                                                        break;
                                                    }
                                                default:
                                                    {
                                                        break;
                                                    }
                                            }
                                            break;
                                        }
                                    default:
                                        {
                                            throw fflerror("failed to add friend: %s", to_cstr(name));
                                        }
                                }
                            });
                        }
                    }
                },
            },

            true,
        },

    }, true);
}

void FriendChatBoard::FriendSearchPage::appendAutoCompletionItem(bool byID, const SDPlayerCandidate &candidate, const std::string &xmlStr)
{
    int maxY = 0;
    autocompletes.foreachChild([&maxY](const Widget *widget, bool)
    {
        maxY = std::max<int>(maxY, widget->dy() + widget->h());
    });

    autocompletes.addChild(new FriendSearchAutoCompletionItem
    {
        DIR_UPLEFT,
        0,
        maxY,

        byID,
        candidate,
        xmlStr.c_str(),

    }, true);
}

FriendChatBoard::FriendChatItem::FriendChatItem(dir8_t argDir,
        int argX,
        int argY,

        const char8_t *argNameStr,
        const char8_t *argMessageStr,

        std::function<SDL_Texture *(const ImageBoard *)> argLoadImageFunc,

        bool argShowName,
        bool argAvatarLeft,
        std::optional<uint32_t> argBGColor,

        Widget *argParent,
        bool argAutoDelete)

    : Widget
      {
          argDir,
          argX,
          argY,

          {},
          {},
          {},

          argParent,
          argAutoDelete,
      }

    , showName(argShowName)
    , avatarLeft(argAvatarLeft)
    , bgColor(std::move(argBGColor))

    , avatar
      {
          DIR_UPLEFT,
          0,
          0,

          FriendChatItem::AVATAR_WIDTH,
          FriendChatItem::AVATAR_HEIGHT,

          std::move(argLoadImageFunc),
      }

    , name
      {
          DIR_UPLEFT,
          0,
          0,

          argNameStr,

          1,
          10,
      }

    , message
      {
          DIR_UPLEFT,
          0,
          0,
          FriendChatItem::MAX_WIDTH - FriendChatItem::AVATAR_WIDTH - FriendChatItem::GAP - FriendChatItem::TRIANGLE_WIDTH - FriendChatItem::MESSAGE_MARGIN * 2,

          to_cstr(argMessageStr),
          0,

          {},
          false,
          false,
          false,
          false,

          1,
          12,
      }

    , background
      {
          DIR_UPLEFT,
          0,
          0,

          FriendChatItem::MESSAGE_MARGIN * 2 + std::max<int>(message.w(), FriendChatItem::MESSAGE_MIN_WIDTH ) + FriendChatItem::TRIANGLE_WIDTH,
          FriendChatItem::MESSAGE_MARGIN * 2 + std::max<int>(message.h(), FriendChatItem::MESSAGE_MIN_HEIGHT),

          [this](const Widget *, int drawDstX, int drawDstY)
          {
              const uint32_t drawBGColor = bgColor.value_or([this]
              {
                  if(avatarLeft){
                      return colorf::RED + colorf::A_SHF(128);
                  }
                  else if(idOpt.has_value()){
                      return colorf::GREEN + colorf::A_SHF(128);
                  }
                  else{
                      return colorf::fadeRGBA(colorf::GREY + colorf::A_SHF(128), colorf::GREEN + colorf::A_SHF(128), std::fabs(std::fmod(accuTime / 1000.0, 2.0) - 1.0));
                  }
              }());

              g_sdlDevice->fillRectangle(
                      drawBGColor,

                      drawDstX + (avatarLeft ? FriendChatItem::TRIANGLE_WIDTH : 0),
                      drawDstY,

                      std::max<int>(message.w(), FriendChatItem::MESSAGE_MIN_WIDTH ) + FriendChatItem::MESSAGE_MARGIN * 2,
                      std::max<int>(message.h(), FriendChatItem::MESSAGE_MIN_HEIGHT) + FriendChatItem::MESSAGE_MARGIN * 2,

                      FriendChatItem::MESSAGE_CORNER);

              const auto triangleX1_avatarLeft = drawDstX;
              const auto triangleX2_avatarLeft = drawDstX + FriendChatItem::TRIANGLE_WIDTH - 1;
              const auto triangleX3_avatarLeft = drawDstX + FriendChatItem::TRIANGLE_WIDTH - 1;

              const auto triangleX1_avatarRight = drawDstX + FriendChatItem::MESSAGE_MARGIN * 2 + std::max<int>(message.w(), FriendChatItem::MESSAGE_MIN_WIDTH) + FriendChatItem::TRIANGLE_WIDTH - 1;
              const auto triangleX2_avatarRight = drawDstX + FriendChatItem::MESSAGE_MARGIN * 2 + std::max<int>(message.w(), FriendChatItem::MESSAGE_MIN_WIDTH);
              const auto triangleX3_avatarRight = drawDstX + FriendChatItem::MESSAGE_MARGIN * 2 + std::max<int>(message.w(), FriendChatItem::MESSAGE_MIN_WIDTH);

              const auto triangleY1_showName = drawDstY + (FriendChatItem::AVATAR_HEIGHT - FriendChatItem::NAME_HEIGHT) / 2;
              const auto triangleY2_showName = drawDstY + (FriendChatItem::AVATAR_HEIGHT - FriendChatItem::NAME_HEIGHT) / 2 - FriendChatItem::TRIANGLE_HEIGHT / 2;
              const auto triangleY3_showName = drawDstY + (FriendChatItem::AVATAR_HEIGHT - FriendChatItem::NAME_HEIGHT) / 2 + FriendChatItem::TRIANGLE_HEIGHT / 2;

              const auto triangleY1_hideName = drawDstY + FriendChatItem::AVATAR_HEIGHT / 2;
              const auto triangleY2_hideName = drawDstY + FriendChatItem::AVATAR_HEIGHT / 2 - FriendChatItem::TRIANGLE_HEIGHT / 2;
              const auto triangleY3_hideName = drawDstY + FriendChatItem::AVATAR_HEIGHT / 2 + FriendChatItem::TRIANGLE_HEIGHT / 2;

              if(avatarLeft){
                  if(showName) g_sdlDevice->fillTriangle(drawBGColor, triangleX1_avatarLeft, triangleY1_showName, triangleX2_avatarLeft, triangleY2_showName, triangleX3_avatarLeft, triangleY3_showName);
                  else         g_sdlDevice->fillTriangle(drawBGColor, triangleX1_avatarLeft, triangleY1_hideName, triangleX2_avatarLeft, triangleY2_hideName, triangleX3_avatarLeft, triangleY3_hideName);
              }
              else{
                  if(showName) g_sdlDevice->fillTriangle(drawBGColor, triangleX1_avatarRight, triangleY1_showName, triangleX2_avatarRight, triangleY2_showName, triangleX3_avatarRight, triangleY3_showName);
                  else         g_sdlDevice->fillTriangle(drawBGColor, triangleX1_avatarRight, triangleY1_hideName, triangleX2_avatarRight, triangleY2_hideName, triangleX3_avatarRight, triangleY3_hideName);
              }
          },
      }
{
    const auto fnMoveAdd = [this](Widget *widgetPtr, dir8_t dstDir, int dstX, int dstY)
    {
        widgetPtr->moveAt(dstDir, dstX, dstY);
        addChild(widgetPtr, false);
    };

    if(avatarLeft){
        fnMoveAdd(&avatar, DIR_UPLEFT, 0, 0);
        if(showName){
            fnMoveAdd(&name      , DIR_LEFT  ,                  FriendChatItem::AVATAR_WIDTH + FriendChatItem::GAP + FriendChatItem::TRIANGLE_WIDTH                                 , FriendChatItem::NAME_HEIGHT / 2                             );
            fnMoveAdd(&background, DIR_UPLEFT,                  FriendChatItem::AVATAR_WIDTH + FriendChatItem::GAP                                                                  , FriendChatItem::NAME_HEIGHT                                 );
            fnMoveAdd(&message   , DIR_UPLEFT,                  FriendChatItem::AVATAR_WIDTH + FriendChatItem::GAP + FriendChatItem::TRIANGLE_WIDTH + FriendChatItem::MESSAGE_MARGIN, FriendChatItem::NAME_HEIGHT + FriendChatItem::MESSAGE_MARGIN);
        }
        else{
            fnMoveAdd(&background, DIR_UPLEFT,                  FriendChatItem::AVATAR_WIDTH + FriendChatItem::GAP                                                                  , 0                                                           );
            fnMoveAdd(&message   , DIR_UPLEFT,                  FriendChatItem::AVATAR_WIDTH + FriendChatItem::GAP + FriendChatItem::TRIANGLE_WIDTH + FriendChatItem::MESSAGE_MARGIN, FriendChatItem::MESSAGE_MARGIN                              );
        }
    }
    else{
        const auto realWidth = FriendChatItem::AVATAR_WIDTH + FriendChatItem::GAP + FriendChatItem::TRIANGLE_WIDTH + std::max<int>(message.w(), FriendChatItem::MESSAGE_MIN_WIDTH) + FriendChatItem::MESSAGE_MARGIN * 2;
        fnMoveAdd(&avatar, DIR_UPRIGHT, realWidth - 1, 0);

        if(showName){
            fnMoveAdd(&name      , DIR_RIGHT  , realWidth - 1 - FriendChatItem::AVATAR_WIDTH - FriendChatItem::GAP - FriendChatItem::TRIANGLE_WIDTH                                 , FriendChatItem::NAME_HEIGHT / 2                             );
            fnMoveAdd(&background, DIR_UPRIGHT, realWidth - 1 - FriendChatItem::AVATAR_WIDTH - FriendChatItem::GAP                                                                  , FriendChatItem::NAME_HEIGHT                                 );
            fnMoveAdd(&message   , DIR_UPRIGHT, realWidth - 1 - FriendChatItem::AVATAR_WIDTH - FriendChatItem::GAP - FriendChatItem::TRIANGLE_WIDTH - FriendChatItem::MESSAGE_MARGIN, FriendChatItem::NAME_HEIGHT + FriendChatItem::MESSAGE_MARGIN);
        }
        else{
            fnMoveAdd(&background, DIR_UPRIGHT, realWidth - 1 - FriendChatItem::AVATAR_WIDTH - FriendChatItem::GAP                                                                  , 0                                                           );
            fnMoveAdd(&message   , DIR_UPRIGHT, realWidth - 1 - FriendChatItem::AVATAR_WIDTH - FriendChatItem::GAP - FriendChatItem::TRIANGLE_WIDTH - FriendChatItem::MESSAGE_MARGIN, FriendChatItem::MESSAGE_MARGIN                              );
        }
    }
}

void FriendChatBoard::FriendChatItem::update(double fUpdateTime)
{
    accuTime += fUpdateTime;
}

FriendChatBoard::FriendChatItemContainer::FriendChatItemContainer(dir8_t argDir,

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

void FriendChatBoard::FriendChatItemContainer::append(FriendChatItem *chatItem, bool autoDelete)
{
    const auto startY = canvas.hasChild() ? (canvas.h() + FriendChatItem::ITEM_SPACE) : 0;
    if(chatItem->avatarLeft){
        chatItem->moveAt(DIR_UPLEFT, 0, startY);
    }
    else{
        chatItem->moveAt(DIR_UPRIGHT, canvas.w() - 1, startY);
    }

    canvas.addChild(chatItem, autoDelete);
}

bool FriendChatBoard::FriendChatItemContainer::hasItem(const Widget *item) const
{
    return canvas.hasChild(item);
}

FriendChatBoard::FriendChatInputContainer::FriendChatInputContainer(dir8_t argDir,

        int argX,
        int argY,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          argDir,
          argX,
          argY,

          UIPage_WIDTH - UIPage_MARGIN * 2 - FriendChatPage::INPUT_MARGIN * 2,
          [this](const Widget *)
          {
              return mathf::bound<int>(layout.h(), FriendChatPage::INPUT_MIN_HEIGHT, FriendChatPage::INPUT_MAX_HEIGHT);
          },

          {},

          argParent,
          argAutoDelete,
      }

    , layout
      {
          DIR_UPLEFT,
          0,
          0,

          this->w(),

          "<layout><par>正在输入的内容。。。</par></layout>",
          0,

          {},
          false,
          true,
          true,
          false,

          1,
          12,
          0,
          colorf::WHITE + colorf::A_SHF(255),
          0,

          LALIGN_JUSTIFY,
          0,
          0,

          2,
          colorf::WHITE + colorf::A_SHF(255),

          nullptr,
          [this]()
          {
              if(!layout.hasToken()){
                  return;
              }

              auto message = layout.getXML();
              auto newItem = new FriendChatItem
              {
                  DIR_UPLEFT,
                  0,
                  0,

                  u8"绝地武士",
                  to_u8cstr(message),

                  [](const ImageBoard *)
                  {
                      return g_progUseDB->retrieve(0X00001100);
                  },

                  true,
                  false,

                  {},
              };

              dynamic_cast<FriendChatPage *>(parent())->chat.append(newItem, true);
              layout.clear();

              const uint32_t toDBID = dynamic_cast<FriendChatPage *>(parent())->dbid;

              auto dbidsv = as_sv(toDBID);
              auto msgbuf = cerealf::serialize(message);

              msgbuf.insert(msgbuf.begin(), dbidsv.begin(), dbidsv.end());
              g_client->send({CM_CHATMESSAGE, msgbuf}, [newItem, this](uint8_t headCode, const uint8_t *buf, size_t bufSize)
              {
                  switch(headCode){
                      case SM_OK:
                          {
                              // TBD allocator may reuse memory
                              // so item with same memory address may not be same item

                              if(dynamic_cast<FriendChatPage *>(parent())->chat.hasItem(newItem)){
                                  newItem->idOpt = cerealf::deserialize<SDChatMessageID>(buf, bufSize);
                              }
                              break;
                          }
                      default:
                          {
                              throw fflerror("failed to send message");
                          }
                  }
              });
          },
          nullptr,

          this,
          false,
      }
{
    // there is mutual dependency
    // height of input container depends on height of layout
    //
    // layout always attach to buttom of input container, so argX needs container height
    // in initialization list we can not call this->h() since initialization of layout is not done yet
    layout.moveAt(DIR_DOWNLEFT, 0, [this](const Widget *){ return this->h() - 1; });
}

FriendChatBoard::FriendChatPage::FriendChatPage(dir8_t argDir,

        int argX,
        int argY,

        Widget *argParent,
        bool argAutoDelete)

    : Widget
      {
          argDir,
          argX,
          argY,

          UIPage_WIDTH,
          UIPage_HEIGHT,

          {},

          argParent,
          argAutoDelete,
      }

    , background
      {
          DIR_UPLEFT,
          0,
          0,
          UIPage_WIDTH,
          UIPage_HEIGHT,

          [this](const Widget *, int drawDstX, int drawDstY)
          {
              g_sdlDevice->drawLine(
                      colorf::RGBA(231, 231, 189, 64),

                      drawDstX,
                      drawDstY + UIPage_HEIGHT - UIPage_MARGIN * 2 - INPUT_MARGIN * 2 - input.h() - 1,

                      drawDstX + UIPage_WIDTH,
                      drawDstY + UIPage_HEIGHT - UIPage_MARGIN * 2 - INPUT_MARGIN * 2 - input.h() - 1);

              g_sdlDevice->fillRectangle(
                      colorf::RGBA(231, 231, 189, 32),

                      drawDstX,
                      drawDstY + UIPage_HEIGHT - UIPage_MARGIN * 2 - INPUT_MARGIN * 2 - input.h(),

                      UIPage_WIDTH,
                      UIPage_MARGIN * 2 + FriendChatPage::INPUT_MARGIN * 2 + input.h());

              g_sdlDevice->fillRectangle(
                      colorf::BLACK + colorf::A_SHF(255),

                      drawDstX + UIPage_MARGIN,
                      drawDstY + UIPage_HEIGHT - UIPage_MARGIN - INPUT_MARGIN * 2 - input.h(),

                      UIPage_WIDTH - UIPage_MARGIN * 2,
                      FriendChatPage::INPUT_MARGIN * 2 + input.h(),

                      FriendChatPage::INPUT_CORNER);

              g_sdlDevice->drawRectangle(
                      colorf::RGBA(231, 231, 189, 96),

                      drawDstX + UIPage_MARGIN,
                      drawDstY + UIPage_HEIGHT - UIPage_MARGIN - INPUT_MARGIN * 2 - input.h(),

                      UIPage_WIDTH - UIPage_MARGIN * 2,
                      FriendChatPage::INPUT_MARGIN * 2 + input.h(),

                      FriendChatPage::INPUT_CORNER);
          },

          this,
          false,
      }

    , input
      {
          DIR_DOWNLEFT,
          UIPage_MARGIN                 + FriendChatPage::INPUT_MARGIN,
          UIPage_HEIGHT - UIPage_MARGIN - FriendChatPage::INPUT_MARGIN - 1,

          this,
          false,
      }

    , chat
      {
          DIR_UPLEFT,
          UIPage_MARGIN,
          UIPage_MARGIN,

          [this](const Widget *)
          {
              return UIPage_HEIGHT - UIPage_MARGIN * 4 - FriendChatPage::INPUT_MARGIN * 2 - input.h() - 1;
          },

          this,
          false,
      }
{}

bool FriendChatBoard::FriendChatPage::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym){
                    case SDLK_RETURN:
                        {
                            if(input.focus()){
                                return Widget::processEvent(event, valid);
                            }
                            else{
                                setFocus(false);
                                return input.consumeFocus(true, std::addressof(input.layout));
                            }
                        }
                    default:
                        {
                            return Widget::processEvent(event, valid);
                        }
                }
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                if(input.in(event.button.x, event.button.y)){
                    setFocus(false);
                    return input.consumeFocus(true, std::addressof(input.layout));
                }

                if(in(event.button.x, event.button.y)){
                    return consumeFocus(true);
                }

                return false;
            }
        default:
            {
                return Widget::processEvent(event, valid);
            }
    }
}

FriendChatBoard::FriendChatPreviewItem::FriendChatPreviewItem(dir8_t argDir,
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

          FriendChatPreviewItem::WIDTH,
          FriendChatPreviewItem::HEIGHT,

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

          FriendChatPreviewItem::AVATAR_WIDTH,
          FriendChatPreviewItem::HEIGHT,

          [this](const ImageBoard *) -> SDL_Texture *
          {
              if(this->dbid == SYS_CHATDBID_SYSTEM){
                  return g_progUseDB->retrieve(0X00001100);
              }
              return g_progUseDB->retrieve(0X02000000);
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
          FriendChatPreviewItem::AVATAR_WIDTH + FriendChatPreviewItem::GAP,
          FriendChatPreviewItem::NAME_HEIGHT / 2,

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
          FriendChatPreviewItem::AVATAR_WIDTH + FriendChatPreviewItem::GAP,
          FriendChatPreviewItem::NAME_HEIGHT,

          FriendChatPreviewItem::WIDTH - FriendChatPreviewItem::AVATAR_WIDTH - FriendChatPreviewItem::GAP,
          FriendChatPreviewItem::HEIGHT - FriendChatPreviewItem::NAME_HEIGHT,

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
                  g_sdlDevice->fillRectangle(colorf::GREEN              + colorf::A_SHF(64), drawDstX, drawDstY, w(), h());
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

bool FriendChatBoard::FriendChatPreviewItem::processEvent(const SDL_Event &event, bool valid)
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
                    if(auto chatBoard = dynamic_cast<FriendChatBoard *>(this->parent(3))){
                        chatBoard->setChatPageDBID(this->dbid);
                        chatBoard->setUIPage(FriendChatBoard::UIPage_CHAT, name.getText(true).c_str());
                    }
                }
                return false;
            }
        default:
            {
                return false;
            }
    }
}

FriendChatBoard::FriendChatPreviewPage::FriendChatPreviewPage(Widget::VarDir argDir,

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

void FriendChatBoard::FriendChatPreviewPage::append(FriendChatPreviewItem *item, bool autoDelete)
{
    item->moveAt(DIR_UPLEFT, 0, canvas.h());
    canvas.addChild(item, autoDelete);
}

void FriendChatBoard::FriendChatPreviewPage::updateChatPreview(uint32_t argDBID, const std::string &argMsg)
{
    FriendChatPreviewItem *child = dynamic_cast<FriendChatPreviewItem *>(canvas.hasChild([argDBID](const Widget *widgetPtr, bool)
    {
        if(auto preview = dynamic_cast<const FriendChatPreviewItem *>(widgetPtr); preview && preview->dbid == argDBID){
            return true;
        }
        return false;
    }));

    if(child){
        child->message.loadXML(argMsg.c_str());
    }
    else{
        child = new FriendChatPreviewItem
        {
            DIR_UPLEFT,
            0,
            0,

            argDBID,
            to_u8cstr(argMsg),
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

FriendChatBoard::PageControl::PageControl(dir8_t argDir,

        int argX,
        int argY,

        int argSpace,

        std::initializer_list<std::pair<Widget *, bool>> argChildList,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          argDir,
          argX,
          argY,

          {},
          {},
          {},

          argParent,
          argAutoDelete,
      }
{
    int offX = 0;
    for(auto &[widgetPtr, autoDelete]: argChildList){
        addChild(widgetPtr, autoDelete);
        widgetPtr->moveAt(DIR_UPLEFT, offX, (h() - widgetPtr->h()) / 2);

        offX += widgetPtr->w();
        offX += argSpace;
    }
}

FriendChatBoard::FriendChatBoard(int argX, int argY, ProcessRun *runPtr, Widget *widgetPtr, bool autoDelete)
    : Widget
      {
          DIR_UPLEFT,
          argX,
          argY,

          UIPage_BORDER[2] + UIPage_WIDTH  + UIPage_BORDER[3],
          UIPage_BORDER[0] + UIPage_HEIGHT + UIPage_BORDER[1],

          {},

          widgetPtr,
          autoDelete,
      }

    , m_processRun(runPtr)
    , m_frame
      {
          DIR_UPLEFT,
          0,
          0,
          {},
          {},
          [](const ImageBoard *){ return g_progUseDB->retrieve(0X00000800); },
      }

    , m_frameCropDup
      {
          DIR_UPLEFT,
          0,
          0,
          this->w(),
          this->h(),

          &m_frame,

          55,
          95,
          285 - 55,
          345 - 95,

          this,
          false,
      }

    , m_background
      {
          DIR_UPLEFT,
          0,
          0,
          {},
          {},
          [](const ImageBoard *){ return g_progUseDB->retrieve(0X00000810); },

          false,
          false,
          0,
          colorf::RGBA(160, 160, 160, 255),
      }

    , m_backgroundCropDup
      {
          DIR_UPLEFT,
          0,
          0,
          m_frameCropDup.w(),
          m_frameCropDup.h(),

          &m_background,

          0,
          0,
          510,
          187,

          this,
          false,
      }

    , m_slider
      {
          DIR_UPLEFT,
          m_frameCropDup.w() - 30,
          70,
          9,
          m_frameCropDup.h() - 140,

          false,
          3,
          nullptr,

          this,
          false,
      }

    , m_close
      {
          DIR_UPLEFT,
          m_frameCropDup.w() - 38,
          m_frameCropDup.h() - 40,
          {SYS_U32NIL, 0X0000001C, 0X0000001D},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this](ButtonBase *)
          {
              setShow(false);
          },

          0,
          0,
          0,
          0,

          true,
          true,
          this,
      }

    , m_uiPageList
      {
          UIPageWidgetGroup // UIPage_CHAT
          {
              .title = new LabelBoard
              {
                  DIR_NONE,
                  45 + (m_frameCropDup.w() - 45 - 190) / 2,
                  29,

                  u8"好友名称",
                  1,
                  14,
                  0,colorf::WHITE + colorf::A_SHF(255),

                  this,
                  true,
              },

              .control = new PageControl
              {
                  DIR_RIGHT,
                  m_frameCropDup.w() - 42,
                  29,
                  2,

                  {
                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X000008F0, 0X000008F0, 0X000008F1},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              [this](ButtonBase *)
                              {
                                  setUIPage(UIPage_CHATPREVIEW);
                              },
                          },

                          true,
                      },

                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X00000023, 0X00000023, 0X00000024},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              [this](ButtonBase *)
                              {
                              },
                          },

                          true,
                      },

                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X000008B0, 0X000008B0, 0X000008B1},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              [this](ButtonBase *)
                              {
                              },
                          },

                          true,
                      },

                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X00000590, 0X00000590, 0X00000591},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              [this](ButtonBase *)
                              {
                              },
                          },

                          true,
                      },

                  },

                  this,
                  true,
              },

              .page = new FriendChatPage
              {
                  DIR_UPLEFT,
                  UIPage_BORDER[2],
                  UIPage_BORDER[0],

                  this,
                  true,
              },
          },

          UIPageWidgetGroup // UIPage_CHATPREVIEW
          {
              .title = new LabelBoard
              {
                  DIR_NONE,
                  45 + (m_frameCropDup.w() - 45 - 190) / 2,
                  29,

                  u8"【聊天记录】",
                  1,
                  14,
                  0,colorf::WHITE + colorf::A_SHF(255),

                  this,
                  true,
              },

              .control = new PageControl
              {
                  DIR_RIGHT,
                  m_frameCropDup.w() - 42,
                  28,
                  2,

                  {
                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X00000160, 0X00000160, 0X00000161},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              [this](ButtonBase *)
                              {
                                  setUIPage(UIPage_FRIENDLIST);
                              },
                          },

                          true,
                      },
                  },

                  this,
                  true,
              },

              .page = new FriendChatPreviewPage
              {
                  DIR_UPLEFT,
                  UIPage_BORDER[2] + UIPage_MARGIN,
                  UIPage_BORDER[0] + UIPage_MARGIN,

                  this,
                  true,
              },
          },

          UIPageWidgetGroup // UIPage_FRIENDLIST
          {
              .title = new LabelBoard
              {
                  DIR_NONE,
                  45 + (m_frameCropDup.w() - 45 - 190) / 2,
                  29,

                  u8"【好友列表】",
                  1,
                  14,
                  0,colorf::WHITE + colorf::A_SHF(255),

                  this,
                  true,
              },

              .control = new PageControl
              {
                  DIR_RIGHT,
                  m_frameCropDup.w() - 42,
                  28,
                  2,

                  {
                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X000008F0, 0X000008F0, 0X000008F1},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              [this](ButtonBase *)
                              {
                                  setUIPage(UIPage_CHATPREVIEW);
                              },
                          },

                          true,
                      },

                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X00000170, 0X00000170, 0X00000171},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              [this](ButtonBase *)
                              {
                                  setUIPage(UIPage_FRIENDSEARCH);
                              },
                          },

                          true,
                      },
                  },

                  this,
                  true,
              },

              .page = new FriendListPage
              {
                  DIR_UPLEFT,
                  UIPage_BORDER[2] + UIPage_MARGIN,
                  UIPage_BORDER[0] + UIPage_MARGIN,

                  this,
                  true,
              },
          },

          UIPageWidgetGroup // UIPage_FRIENDSEARCH
          {
              .title = new LabelBoard
              {
                  DIR_NONE,
                  45 + (m_frameCropDup.w() - 45 - 190) / 2,
                  29,

                  u8"【查找用户】",
                  1,
                  14,
                  0,colorf::WHITE + colorf::A_SHF(255),

                  this,
                  true,
              },

              .control = new PageControl
              {
                  DIR_RIGHT,
                  m_frameCropDup.w() - 42,
                  28,
                  2,

                  {
                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X000008F0, 0X000008F0, 0X000008F1},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              [this](ButtonBase *)
                              {
                                  setUIPage(UIPage_CHATPREVIEW);
                              },
                          },

                          true,
                      },
                  },

                  this,
                  true,
              },

              .page = new FriendSearchPage
              {
                  DIR_UPLEFT,
                  UIPage_BORDER[2] + UIPage_MARGIN,
                  UIPage_BORDER[0] + UIPage_MARGIN,

                  this,
                  true,
              },
          },
      }
{
    setShow(false);
}

void FriendChatBoard::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    for(const auto &p:
    {
        static_cast<const Widget *>(&m_backgroundCropDup),
        static_cast<const Widget *>( m_uiPageList[m_uiPage].page),
        static_cast<const Widget *>(&m_frameCropDup),
        static_cast<const Widget *>( m_uiPageList[m_uiPage].title),
        static_cast<const Widget *>( m_uiPageList[m_uiPage].control),
        static_cast<const Widget *>(&m_slider),
        static_cast<const Widget *>(&m_close),
    }){
        int drawSrcX = srcX;
        int drawSrcY = srcY;
        int drawSrcW = srcW;
        int drawSrcH = srcH;
        int drawDstX = dstX;
        int drawDstY = dstY;

        if(mathf::cropChildROI(
                    &drawSrcX, &drawSrcY,
                    &drawSrcW, &drawSrcH,
                    &drawDstX, &drawDstY,

                    w(),
                    h(),

                    p->dx(),
                    p->dy(),
                    p-> w(),
                    p-> h())){
            p->drawEx(drawDstX, drawDstY, drawSrcX, drawSrcY, drawSrcW, drawSrcH);
        }
    }
}

bool FriendChatBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
    }

    if(m_close                        .processEvent(event, valid)){ return true; }
    if(m_slider                       .processEvent(event, valid)){ return true; }
    if(m_uiPageList[m_uiPage].page   ->processEvent(event, valid)){ return true; }
    if(m_uiPageList[m_uiPage].control->processEvent(event, valid)){ return true; }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                if(focus()){
                    switch(event.key.keysym.sym){
                        case SDLK_ESCAPE:
                            {
                                setShow(false);
                                setFocus(false);
                                return true;
                            }
                        default:
                            {
                                return false;
                            }
                    }
                }
                return false;
            }
        case SDL_MOUSEMOTION:
            {
                if((event.motion.state & SDL_BUTTON_LMASK) && (in(event.motion.x, event.motion.y) || focus())){
                    const auto [rendererW, rendererH] = g_sdlDevice->getRendererSize();
                    const int maxX = rendererW - w();
                    const int maxY = rendererH - h();

                    const int newX = std::max<int>(0, std::min<int>(maxX, x() + event.motion.xrel));
                    const int newY = std::max<int>(0, std::min<int>(maxY, y() + event.motion.yrel));
                    moveBy(newX - x(), newY - y());
                    return consumeFocus(true);
                }
                return consumeFocus(false);
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                if(m_uiPageList[m_uiPage].page->in(event.button.x, event.button.y)){
                    if(m_uiPageList[m_uiPage].page->processEvent(event, true)){
                        return consumeFocus(true, m_uiPageList[m_uiPage].page);
                    }
                }
                return consumeFocus(in(event.button.x, event.button.y));
            }
        case SDL_MOUSEWHEEL:
            {
                if(m_uiPageList[m_uiPage].page->focus()){
                    if(m_uiPageList[m_uiPage].page->processEvent(event, true)){
                        return consumeFocus(true, m_uiPageList[m_uiPage].page);
                    }
                }
                return false;
            }
        default:
            {
                return false;
            }
    }
}

void FriendChatBoard::setFriendList(const SDFriendList &)
{
    dynamic_cast<FriendListPage *>(m_uiPageList[UIPage_FRIENDLIST].page)->append(new FriendItem
    {
        DIR_UPLEFT,
        0,
        0,

        SYS_CHATDBID_SYSTEM,
        u8"系统消息",

        [](const ImageBoard *)
        {
            return g_progUseDB->retrieve(0X00001100);
        },

    }, true);

    dynamic_cast<FriendListPage *>(m_uiPageList[UIPage_FRIENDLIST].page)->append(new FriendItem
    {
        DIR_UPLEFT,
        0,
        0,

        m_processRun->getMyHero()->dbid(),
        to_u8cstr(m_processRun->getMyHero()->getName()),

        [this](const ImageBoard *)
        {
            return g_progUseDB->retrieve(to_u32(0X02000000) + [this]() -> uint32_t
            {
                if(m_processRun->getMyHero()->job() & JOB_WARRIOR){
                    return 0;
                }
                else if(m_processRun->getMyHero()->job() & JOB_TAOIST){
                    return 2;
                }
                else{
                    return 4;
                }
            }() + (m_processRun->getMyHero()->gender() ? 0 : 1));
        },
    }, true);
}

void FriendChatBoard::addMessage(const SDChatMessage &newmsg)
{
    const auto saveInDBID = [&newmsg, this]
    {
        if(newmsg.from == m_processRun->getMyHero()->dbid()){
            return newmsg.to;
        }
        else if(newmsg.to == m_processRun->getMyHero()->dbid()){
            return newmsg.from;
        }
        else{
            return m_processRun->getMyHero()->dbid();
        }
    }();

    auto p = std::find_if(m_friendMessageList.begin(), m_friendMessageList.end(), [saveInDBID](const auto &item)
    {
        return item.dbid == saveInDBID;
    });

    if(p == m_friendMessageList.end()){
        m_friendMessageList.emplace_front(saveInDBID);
    }
    else if(p != m_friendMessageList.begin()){
        m_friendMessageList.splice(m_friendMessageList.begin(), m_friendMessageList, p, std::next(p));
    }

    p = m_friendMessageList.begin();
    if(std::find_if(p->list.begin(), p->list.end(), [&newmsg](const auto &msg){ return msg.id == newmsg.id; }) == p->list.end()){
        p->unread++;
        p->list.push_back(newmsg);
        std::sort(p->list.begin(), p->list.end(), [](const auto &x, const auto &y)
        {
            return x.timestamp < y.timestamp;
        });
        dynamic_cast<FriendChatPreviewPage *>(m_uiPageList[UIPage_CHATPREVIEW].page)->updateChatPreview(saveInDBID, cerealf::deserialize<std::string>(p->list.back().message));
    }
}

void FriendChatBoard::setChatPageDBID(uint32_t argDBID)
{
    auto chatPage = dynamic_cast<FriendChatPage *>(m_uiPageList[UIPage_CHAT].page);
    if(chatPage->dbid != argDBID){
        chatPage->dbid = argDBID;
        chatPage->chat.canvas.clearChild();

        for(const auto &node: m_friendMessageList){
            if(node.dbid == argDBID){
                for(const auto &msg: node.list){
                    chatPage->chat.append(new FriendChatItem
                    {
                        DIR_UPLEFT,
                        0,
                        0,

                        [chatPage]() -> const char8_t *
                        {
                            if(chatPage->dbid == SYS_CHATDBID_SYSTEM){
                                return u8"系统消息";
                            }
                            return u8"绝地武士";
                        }(),

                        to_u8cstr(cerealf::deserialize<std::string>(msg.message)),

                        [chatPage](const ImageBoard *)
                        {
                            if(chatPage->dbid == SYS_CHATDBID_SYSTEM){
                                return g_progUseDB->retrieve(0X00001100);
                            }
                            return g_progUseDB->retrieve(0X02000000);
                        },

                        true,
                        true,

                        {},
                    }, true);
                }
            }
        }
    }
}

void FriendChatBoard::setUIPage(int uiPage, const char *titleStr)
{
    fflassert(uiPage >= 0, uiPage);
    fflassert(uiPage < UIPage_END, uiPage);

    if(m_uiPage != uiPage){
        m_uiLastPage = m_uiPage;
        m_uiPage     = uiPage;

        m_uiPageList[m_uiLastPage].page->setFocus(false);
        m_uiPageList[m_uiPage    ].page->setFocus(true );


        if(titleStr){
            m_uiPageList[m_uiPage].title->setText(to_u8cstr(titleStr));
        }
    }
}

FriendChatBoard *FriendChatBoard::getParentBoard(Widget *widget)
{
    fflassert(widget);
    while(widget){
        if(auto p = dynamic_cast<FriendChatBoard *>(widget)){
            return p;
        }
        else{
            widget = widget->parent();
        }
    }
    throw fflerror("widget is not a decedent of FriendChatBoard");
}

const FriendChatBoard *FriendChatBoard::getParentBoard(const Widget *widget)
{
    fflassert(widget);
    while(widget){
        if(auto p = dynamic_cast<const FriendChatBoard *>(widget)){
            return p;
        }
        else{
            widget = widget->parent();
        }
    }
    throw fflerror("widget is not a decedent of FriendChatBoard");
}
