#include "sdldevice.hpp"
#include "pngtexdb.hpp"
#include "processrun.hpp"
#include "friendchatboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

struct FriendChatPreviewItem: public Widget
{
    static constexpr int WIDTH  = 400;
    static constexpr int HEIGHT = 50;

    static constexpr int GAP = 10;
    static constexpr int AVATAR_WIDTH = HEIGHT * 84 / 94; // original avatar size: 84 x 94

    //        GAP
    //       |<->|
    // +-+---+  +------+         -
    // |1|   |  | name |         ^
    // +-+   |  +------+         | HEIGHT
    // | IMG |  +--------------+ |
    // |     |  |latest message| v
    // +-----+  +--------------+ -
    //
    // |<--->|
    // AVATAR_WIDTH
    //
    // |<--------------------->|
    //           WIDTH

    ImageBoard     avatar;
    LabelBoard     name;
    LayoutBoard    message;
    ShapeClipBoard selected;

    FriendChatPreviewItem(dir8_t argDir, int argX, int argY, const char8_t *nameStr, std::function<SDL_Texture *(const ImageBoard *)> argLoadImageFunc, Widget *argParent, bool argAutoDelete)
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

        , avatar
          {
              DIR_UPLEFT,
              0,
              0,

              FriendChatPreviewItem::AVATAR_WIDTH,
              FriendChatPreviewItem::HEIGHT,

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
              DIR_UPLEFT,
              FriendChatPreviewItem::AVATAR_WIDTH + FriendChatPreviewItem::GAP,
              0,

              nameStr,

              1,
              12,
              0,
              colorf::WHITE + colorf::A_SHF(255),

              this,
              false,
          }

        , message
          {
              DIR_DOWNRIGHT,
              FriendChatPreviewItem::WIDTH  - 1,
              FriendChatPreviewItem::HEIGHT - 1,

              FriendChatPreviewItem::WIDTH - FriendChatPreviewItem::AVATAR_WIDTH - FriendChatPreviewItem::GAP,

              false,
              {0, 0, 0, 0},

              false,

              1,
              12,
              0,
              colorf::WHITE + colorf::A_SHF(255),
              0,

              LALIGN_LEFT,
              0,
              0,

              nullptr,

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
                  if(focus()){
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
};

struct FriendChatPreviewPage: public Widget
{
    using Widget::Widget;
    bool processEvent(const SDL_Event &event, bool valid) override
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
                    if(focus()){
                        switch(event.key.keysym.sym){
                            case SDLK_RETURN:
                            default:
                                {
                                    return consumeFocus(true);
                                }
                        }
                    }
                    return false;
                }
            case SDL_MOUSEBUTTONDOWN:
                {
                    Widget *clicked = nullptr;
                    for(auto &child: m_childList){
                        if(child.widget->in(event.button.x, event.button.y)){
                            clicked = child.widget;
                            break;
                        }
                    }

                    if(clicked){
                        return consumeFocus(true, clicked);
                    }

                    return consumeFocus(in(event.button.x, event.button.y));
                }
            default:
                {
                    return false;
                }
        }
    }
};

FriendChatBoard::FriendChatBoard(int argX, int argY, ProcessRun *runPtr, Widget *widgetPtr, bool autoDelete)
    : Widget
      {
          DIR_UPLEFT,
          argX,
          argY,

          UIPage_MARGIN[2] + FriendChatPreviewItem::WIDTH      + UIPage_MARGIN[3],
          UIPage_MARGIN[0] + FriendChatPreviewItem::HEIGHT * 8 + UIPage_MARGIN[1],

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

    , m_UIPage_CHATPREVIEW(new FriendChatPreviewPage
      {
          DIR_UPLEFT,
          UIPage_MARGIN[2],
          UIPage_MARGIN[0],

          m_frameCropDup.w() - UIPage_MARGIN[2] - UIPage_MARGIN[3],
          m_frameCropDup.h() - UIPage_MARGIN[0] - UIPage_MARGIN[1],

          {},

          this,
          true,
      })
{
    setShow(false);
    m_UIPage_CHATPREVIEW->addChild(new FriendChatPreviewItem
    {
        DIR_UPLEFT,
        0,
        0,

        u8"绝地武士",
        [](const ImageBoard *)
        {
            return g_progUseDB->retrieve(0X02000000);
        },

        nullptr,
        false,

    }, true);


    m_UIPage_CHATPREVIEW->addChild(new FriendChatPreviewItem
    {
        DIR_UPLEFT,
        0,
        FriendChatPreviewItem::HEIGHT,

        u8"恭喜发财",
        [](const ImageBoard *)
        {
            return g_progUseDB->retrieve(0X02000001);
        },

        nullptr,
        false,

    }, true);
}

void FriendChatBoard::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    for(const auto &p:
    {
        static_cast<const Widget *>(&m_backgroundCropDup),
        static_cast<const Widget *>( m_UIPage_CHATPREVIEW),
        static_cast<const Widget *>(&m_frameCropDup),
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

    if(m_close .processEvent(event, valid)){ return true; }
    if(m_slider.processEvent(event, valid)){ return true; }

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
                if(m_uiPage == UIPage_CHATPREVIEW && m_UIPage_CHATPREVIEW->in(event.button.x, event.button.y)){
                    if(m_UIPage_CHATPREVIEW->processEvent(event, true)){
                        return consumeFocus(true, m_UIPage_CHATPREVIEW);
                    }
                }
                return consumeFocus(in(event.button.x, event.button.y));
            }
        case SDL_MOUSEWHEEL:
            {
                if(m_UIPage_CHATPREVIEW->focus()){
                    m_UIPage_CHATPREVIEW->processEvent(event, true);
                    return consumeFocus(true, m_UIPage_CHATPREVIEW);
                }
                return consumeFocus(focus());
            }
        default:
            {
                return false;
            }
    }
}

void FriendChatBoard::setFriendList(const SDFriendList &)
{}

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
    }
}
