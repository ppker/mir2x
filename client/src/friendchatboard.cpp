#include <initializer_list>
#include "sdldevice.hpp"
#include "client.hpp"
#include "hero.hpp"
#include "pngtexdb.hpp"
#include "processrun.hpp"
#include "friendchatboard.hpp"

extern Client *g_client;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

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

              .page = new ChatPage
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

              .page = new ChatPreviewPage
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

              .page = new SearchPage
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

void FriendChatBoard::setFriendList(const SDFriendList &sdFL)
{
    m_sdFriendList = sdFL;
    std::unordered_set<uint32_t> seenDBIDList;

    seenDBIDList.insert(SYS_CHATDBID_SYSTEM);
    dynamic_cast<FriendListPage *>(m_uiPageList[UIPage_FRIENDLIST].page)->append(SDChatPeer
    {
        .dbid = SYS_CHATDBID_SYSTEM,
        .name = "系统助手",
    });

    seenDBIDList.insert(m_processRun->getMyHero()->dbid());
    dynamic_cast<FriendListPage *>(m_uiPageList[UIPage_FRIENDLIST].page)->append(SDChatPeer
    {
        .dbid   = m_processRun->getMyHero()->dbid(),
        .name   = m_processRun->getMyHero()->getName(),
        .gender = m_processRun->getMyHero()->gender(),
        .job    = m_processRun->getMyHero()->job(),
    });

    for(const auto &sdPC: sdFL){
        if(seenDBIDList.contains(sdPC.dbid)){
            continue;
        }

        seenDBIDList.insert(sdPC.dbid);
        dynamic_cast<FriendListPage *>(m_uiPageList[UIPage_FRIENDLIST].page)->append(sdPC);
    }
}

const SDChatPeer *FriendChatBoard::findFriend(uint32_t argDBID) const
{
    if(auto p = std::find_if(m_sdFriendList.begin(), m_sdFriendList.end(), [argDBID](const auto &x) { return argDBID == x.dbid; }); p != m_sdFriendList.end()){
        return std::addressof(*p);
    }
    return nullptr;
}

void FriendChatBoard::queryChatPeer(bool argGroup, uint32_t argDBID, std::function<void(const SDChatPeer *)> argOp)
{
    if(argDBID == SYS_CHATDBID_SYSTEM){
        if(argOp){
            const SDChatPeer sysPeer
            {
                .dbid = SYS_CHATDBID_SYSTEM,
                .name = "系统助手",
            };
            argOp(std::addressof(sysPeer));
        }
    }

    else if(argDBID == SYS_CHATDBID_GROUP){
        if(argOp){
            const SDChatPeer groupPeer
            {
                .dbid = SYS_CHATDBID_GROUP,
                .name = "群管理助手",
            };
            argOp(std::addressof(groupPeer));
        }
    }

    else if(!argGroup && argDBID == m_processRun->getMyHero()->dbid()){
        if(argOp){
            const SDChatPeer groupPeer
            {
                .dbid   = m_processRun->getMyHero()->dbid(),
                .name   = m_processRun->getMyHero()->getName(),
                .gender = m_processRun->getMyHero()->gender(),
                .job    = m_processRun->getMyHero()->job(),
            };
            argOp(std::addressof(groupPeer));
        }
    }

    else if(auto p = argGroup ? nullptr : findFriend(argDBID)){
        if(argOp){
            argOp(p);
        }
    }

    else if(auto strangerIter = std::find_if(m_strangerList.begin(), m_strangerList.end(), [argGroup, argDBID](const auto &x)
    {
        return x.group == argGroup && x.dbid == argDBID;

    }); strangerIter != m_strangerList.end()){
        if(argOp){
            argOp(std::addressof(*strangerIter));
        }
    }

    else{
        CMQueryChatPeerList cmQCPL;
        std::memset(&cmQCPL, 0, sizeof(cmQCPL));

        cmQCPL.input.assign(std::to_string(argDBID));
        g_client->send({CM_QUERYCHATPEERLIST, cmQCPL}, [argGroup, argDBID, argOp = std::move(argOp), this](uint8_t headCode, const uint8_t *data, size_t size)
        {
            switch(headCode){
                case SM_OK:
                  {
                      if(const auto sdPCL = cerealf::deserialize<SDChatPeerList>(data, size); sdPCL.empty()){
                          if(argOp){
                              argOp(nullptr);
                          }
                          return;
                      }
                      else{
                          for(const auto &peer: sdPCL){
                              if(peer.group == argGroup && peer.dbid == argDBID){
                                  if(argOp){
                                      argOp(&peer);
                                  }
                                  return;
                              }
                          }

                          if(argOp){
                              argOp(nullptr);
                          }
                          return;
                      }
                  }
              default:
                  {
                      throw fflerror("query failed in server");
                  }
            }
        });
    }
}

void FriendChatBoard::addMessage(std::optional<uint64_t> localPendingID, const SDChatMessage &sdCM)
{
    fflassert(sdCM.seq.has_value());
    const auto peerDBID = [&sdCM, this]
    {
        if(sdCM.from == m_processRun->getMyHero()->dbid()){
            return sdCM.to;
        }
        else if(sdCM.to == m_processRun->getMyHero()->dbid()){
            return sdCM.from;
        }
        else{
            throw fflerror("received invalid chat message: from %llu, to %llu, self %llu", to_llu(sdCM.from), to_llu(sdCM.to), to_llu(m_processRun->getMyHero()->dbid()));
        }
    }();

    auto peerIter = std::find_if(m_friendMessageList.begin(), m_friendMessageList.end(), [peerDBID, &sdCM](const auto &item)
    {
        return item.dbid == peerDBID && item.group == sdCM.group;
    });

    if(peerIter == m_friendMessageList.end()){
        m_friendMessageList.emplace_front(sdCM.group, peerDBID);
    }
    else if(peerIter != m_friendMessageList.begin()){
        m_friendMessageList.splice(m_friendMessageList.begin(), m_friendMessageList, peerIter, std::next(peerIter));
    }

    peerIter = m_friendMessageList.begin();
    if(std::find_if(peerIter->list.begin(), peerIter->list.end(), [&sdCM](const auto &msg){ return msg.seq.value().id == sdCM.seq.value().id; }) == peerIter->list.end()){
        peerIter->unread++;
        peerIter->list.push_back(sdCM);

        if(peerIter->list.size() >= 2 && peerIter->list.back().seq.value().timestamp < peerIter->list.rbegin()[1].seq.value().timestamp){
            std::sort(peerIter->list.begin(), peerIter->list.end(), [](const auto &x, const auto &y)
            {
                if(x.seq.value().timestamp != y.seq.value().timestamp){
                    return x.seq.value().timestamp < y.seq.value().timestamp;
                }
                else{
                    return x.seq.value().id < y.seq.value().id;
                }
            });

            if(dynamic_cast<ChatPage *>(m_uiPageList[UIPage_CHAT].page)->peer.dbid == peerIter->dbid){
                loadChatPage();
            }
        }
        else{
            if(auto chatPage = dynamic_cast<ChatPage *>(m_uiPageList[UIPage_CHAT].page); chatPage->peer.dbid == peerIter->dbid){
                if(localPendingID.has_value()){
                    if(auto p = chatPage->chat.canvas.hasChild(localPendingID.value())){
                        dynamic_cast<FriendChatBoard::ChatItem *>(p)->pending = false;
                    }
                }
                else{
                    chatPage->chat.append(peerIter->list.back(), nullptr);
                }
            }
        }
        dynamic_cast<ChatPreviewPage *>(m_uiPageList[UIPage_CHATPREVIEW].page)->updateChatPreview(peerDBID, cerealf::deserialize<std::string>(peerIter->list.back().message));
    }
}

void FriendChatBoard::addMessagePending(uint64_t localPendingID, const SDChatMessage &sdCM)
{
    fflassert(!sdCM.seq.has_value());
    if(!m_localMessageList.emplace(localPendingID, sdCM).second){
        throw fflerror("adding a pending message with local pending id which has already been used: %llu", to_llu(localPendingID));
    }
}

void FriendChatBoard::finishMessagePending(size_t localPendingID, const SDChatMessageDBSeq &sdCMDBS)
{
    if(auto p = m_localMessageList.find(localPendingID); p != m_localMessageList.end()){
        auto chatMessage = std::move(p->second);
        m_localMessageList.erase(p);

        chatMessage.seq = sdCMDBS;
        addMessage(localPendingID, chatMessage);
    }
    else{
        throw fflerror("invalid local pending message id: %zu", localPendingID);
    }
}

void FriendChatBoard::setChatPeer(const SDChatPeer &sdCP, bool forceReload)
{
    if(auto chatPage = dynamic_cast<ChatPage *>(m_uiPageList[UIPage_CHAT].page); (chatPage->peer.dbid != sdCP.dbid) || forceReload){
        chatPage->peer = sdCP;
        loadChatPage();
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

void FriendChatBoard::loadChatPage()
{
    auto chatPage = dynamic_cast<ChatPage *>(m_uiPageList[UIPage_CHAT].page);
    chatPage->chat.canvas.clearChild();

    for(const auto &elem: m_friendMessageList){
        if(elem.group == chatPage->peer.group && elem.dbid == chatPage->peer.dbid){
            for(const auto &msg: elem.list){
                chatPage->chat.append(msg, nullptr);
            }
            break;
        }
    }

    for(const auto &[localID, sdCM]: m_localMessageList){
        if(sdCM.group == chatPage->peer.group && sdCM.to == chatPage->peer.dbid){
            chatPage->chat.append(sdCM, nullptr);
        }
    }

    chatPage->placeholder.setShow(!chatPage->chat.canvas.hasChild());
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
