#include "client.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "friendchatboard.hpp"

extern Client *g_client;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

FriendChatBoard::ChatInputContainer::ChatInputContainer(dir8_t argDir,

        int argX,
        int argY,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          argDir,
          argX,
          argY,

          UIPage_WIDTH - UIPage_MARGIN * 2 - ChatPage::INPUT_MARGIN * 2,
          [this](const Widget *)
          {
              return mathf::bound<int>(layout.h(), ChatPage::INPUT_MIN_HEIGHT, ChatPage::INPUT_MAX_HEIGHT);
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

          nullptr,
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
              auto newItem = new ChatItem
              {
                  DIR_UPLEFT,
                  0,
                  0,

                  {},
                  to_u8cstr(FriendChatBoard::getParentBoard(this)->m_processRun->getMyHero()->getName()),
                  to_u8cstr(message),

                  [this](const ImageBoard *)
                  {
                      auto boardPtr = FriendChatBoard::getParentBoard(this);
                      return g_progUseDB->retrieve(Hero::faceGfxID(boardPtr->m_processRun->getMyHero()->gender(), boardPtr->m_processRun->getMyHero()->job()));
                  },

                  true,
                  false,

                  {},
              };

              dynamic_cast<ChatPage *>(parent())->chat.append(newItem, true);
              layout.clear();

              const uint32_t toDBID = dynamic_cast<ChatPage *>(parent())->dbid;

              auto dbidsv = as_sv(toDBID);
              auto msgbuf = cerealf::serialize(message);

              msgbuf.insert(msgbuf.begin(), dbidsv.begin(), dbidsv.end());
              g_client->send({CM_CHATMESSAGE, msgbuf}, [newItem, message, toDBID, this](uint8_t headCode, const uint8_t *buf, size_t bufSize)
              {
                  switch(headCode){
                      case SM_OK:
                          {
                              // TBD allocator may reuse memory
                              // so item with same memory address may not be same item

                              const auto sdCMID = cerealf::deserialize<SDChatMessageID>(buf, bufSize);
                              if(dynamic_cast<ChatPage *>(parent())->chat.hasItem(newItem)){
                                  newItem->idOpt = sdCMID.id;
                              }

                              // create manually here
                              // sever won't echo the sent chat message

                              FriendChatBoard::getParentBoard(this)->addMessage(SDChatMessage
                              {
                                  .id = sdCMID.id,
                                  .from = FriendChatBoard::getParentBoard(this)->m_processRun->getMyHero()->dbid(),
                                  .to = toDBID,
                                  .timestamp = sdCMID.timestamp,
                                  .message = cerealf::serialize(message),
                              });
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
