#include "luaf.hpp"
#include "client.hpp"
#include "imeboard.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "soundeffectdb.hpp"
#include "processrun.hpp"
#include "inventoryboard.hpp"

extern Client *g_client;
extern IMEBoard *g_imeBoard;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

RuntimeConfigBoard::PullMenu::PullMenu(
        dir8_t argDir,
        int argX,
        int argY,

        const char8_t *argLabel,
        int argLabelWidth,

        int argTitleBgWidth,
        int argTitleBgHeight,

        std::initializer_list<std::pair<Widget *, bool>> argMenuList,
        std::function<void(Widget *)> argOnClickMenu,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          argDir,
          argX,
          argY,
          0,
          0,

          {},

          argParent,
          argAutoDelete,
      }

    , m_label
      {
          DIR_UPLEFT,
          0,
          0,

          argLabel,
          1,
          12,
          0,
          colorf::WHITE + colorf::A_SHF(255),
      }

    , m_labelCrop
      {
          DIR_UPLEFT,
          0,
          0,

          &m_label,

          0,
          0,
          [argLabelWidth]{fflassert(argLabelWidth >= 0); return argLabelWidth; }(),
          m_label.h(),

          {},

          this,
          false,
      }

    , m_menuTitleImage
      {
          DIR_UPLEFT,
          0,
          0,
          {},
          {},
          [](const ImageBoard *){ return g_progUseDB->retrieve(0X00000460); },
      }

    , m_menuTitleBackground
      {
          DIR_UPLEFT,
          0,
          0,

          [argTitleBgWidth ]{fflassert(argTitleBgWidth  >= 0); return argTitleBgWidth ; }(),
          [argTitleBgHeight]{fflassert(argTitleBgHeight >= 0); return argTitleBgHeight; }(),

          &m_menuTitleImage,

          3,
          3,
          m_menuTitleImage.w() - 6,
          2,

          this,
          false,
      }

    , m_menuTitle
      {
          DIR_UPLEFT,
          0,
          0,

          u8"NA",
          1,
          12,
          0,
          colorf::WHITE + colorf::A_SHF(255),
      }

    , m_menuTitleCrop
      {
          DIR_UPLEFT,
          0,
          0,

          &m_menuTitle,

          0,
          0,
          m_menuTitleBackground.w() - 6,
          std::min<int>(m_menuTitleBackground.h() - 4, m_menuTitle.h()),

          {},

          this,
          false,
      }

    , m_imgOff {DIR_UPLEFT, 0, 0, 22, 22, [](const ImageBoard *){ return g_progUseDB->retrieve(0X00000301); }, false, false, 1}
    , m_imgOn  {DIR_UPLEFT, 0, 0, 22, 22, [](const ImageBoard *){ return g_progUseDB->retrieve(0X00000300); }, false, false, 1}
    , m_imgDown{DIR_UPLEFT, 0, 0, 22, 22, [](const ImageBoard *){ return g_progUseDB->retrieve(0X00000302); }, false, false, 1}

    , m_button
      {
          DIR_UPLEFT,
          0,
          0,

          {
              &m_imgOff,
              &m_imgOn,
              &m_imgDown,
          },

          {
              std::nullopt,
              std::nullopt,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this]()
          {
              m_menuList.flipShow();
              if(m_menuList.show()){
                  setSize(std::max<int>(w(), m_menuList.dx() + m_menuList.w()),
                          std::max<int>(h(), m_menuList.dy() + m_menuList.h()));
              }
              else{
                  setSize(m_button.dx() + m_button.w(),
                          std::max<int>({m_labelCrop.h(), m_menuTitleBackground.h(), m_button.h()}));
              }
          },

          0,
          0,
          0,
          0,

          true,

          this,
          false,
      }

    , m_menuList
      {
          DIR_UPLEFT,
          0,
          0,

          m_menuTitleBackground.w() - 6,

          0,
          0,

          argMenuList,
          std::move(argOnClickMenu),

          {},

          this,
          false,
      }
{
    m_menuList.setShow(false);
    setSize(m_labelCrop.w() + m_menuTitleBackground.w() + m_button.w(), std::max<int>({m_labelCrop.h(), m_menuTitleBackground.h(), m_button.h()}));

    m_labelCrop          .moveAt(DIR_LEFT, 0                                 , h() / 2);
    m_menuTitleBackground.moveAt(DIR_LEFT, m_labelCrop.dx() + m_labelCrop.w(), h() / 2);

    m_menuTitleCrop.moveAt(DIR_LEFT, m_menuTitleBackground.dx() + 3                        , h() / 2);
    m_button       .moveAt(DIR_LEFT, m_menuTitleBackground.dx() + m_menuTitleBackground.w(), h() / 2);

    m_menuList.moveAt(DIR_UPLEFT, m_menuTitleBackground.dx() + 3, m_menuTitleBackground.dy() + m_menuTitleBackground.h() - 2);
}

void RuntimeConfigBoard::PullMenu::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    if(!show()){
        return;
    }

    for(const auto p:
    {
        static_cast<const Widget *>(&m_menuTitleBackground),
        static_cast<const Widget *>(&m_labelCrop),
        static_cast<const Widget *>(&m_menuTitleCrop),
        static_cast<const Widget *>(&m_button),
        static_cast<const Widget *>(&m_menuList),
    }){
        if(!p->show()){
            continue;
        }

        int srcXCrop = srcX;
        int srcYCrop = srcY;
        int dstXCrop = dstX;
        int dstYCrop = dstY;
        int srcWCrop = srcW;
        int srcHCrop = srcH;

        if(!mathf::cropChildROI(
                    &srcXCrop, &srcYCrop,
                    &srcWCrop, &srcHCrop,
                    &dstXCrop, &dstYCrop,

                    w(),
                    h(),

                    p->dx(),
                    p->dy(),
                    p-> w(),
                    p-> h())){
            continue;
        }
        p->drawEx(dstXCrop, dstYCrop, srcXCrop, srcYCrop, srcWCrop, srcHCrop);
    }
}

RuntimeConfigBoard::RuntimeConfigBoard(int argX, int argY, int argW, int argH, ProcessRun *proc, Widget *widgetPtr, bool autoDelete)
    : Widget(DIR_UPLEFT, argX, argY, argW, argH, {}, widgetPtr, autoDelete)
    , m_frameBoard
      {
          DIR_UPLEFT,
          0,
          0,
          argW,
          argH,

          this,
          false,
      }

    , m_checkLabel
      {
          DIR_UPLEFT,
          350,
          40,

          true,
          8,

          colorf::RGBA(231, 231, 189, 128),
          16,
          16,
          m_sdRuntimeConfig.ime,

          u8"拼音输入法",
          1,
          12,
          0,
          colorf::WHITE + colorf::A_SHF(255),

          this,
          false,
      }

    , m_texSliderBar
      {
          DIR_UPLEFT,
          250,
          200,

          100,
          true,

          1,
          [](float){},

          this,
          false,
      }

    , m_texSliderBarVertical
      {
          DIR_UPLEFT,
          350,
          250,

          100,
          false,

          1,
          [](float){},

          this,
          false,
      }

    , m_musicSlider
      {
          DIR_UPLEFT,
          380,
          124,
          194,

          true,
          1,
          [this](float)
          {
          },
          this,
      }

    , m_soundEffectSlider
      {
          DIR_UPLEFT,
          380,
          184,
          194,

          true,
          1,
          [this](float)
          {
          },
          this,
      }

    , m_item1
      {
          DIR_UPLEFT,
          0,
          0,
          u8"你好",
      }

    , m_item2
      {
          DIR_UPLEFT,
          0,
          0,
          u8"天气很好",
      }

    , m_itemCheckLabel
      {
          DIR_UPLEFT,
          0,
          0,

          true,
          8,

          colorf::RGBA(231, 231, 189, 128),
          16,
          16,
          m_sdRuntimeConfig.ime,

          u8"拼音输入法",
          1,
          12,
          0,
          colorf::WHITE + colorf::A_SHF(255),
      }

    , m_menuBoard
      {
          DIR_UPLEFT,
          0,
          0,

          {},

          10,
          50,

          {
              {&m_item1, false},
              {&m_item2, false},
              {&m_itemCheckLabel, false},
          },

          nullptr,

          {10, 10, 10, 10},
      }

    , m_menuExpandButton
      {
          DIR_UPLEFT,
          180,
          130,

          {0X00000055, 0X00000055, 0X00000056},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this]()
          {
          },

          0,
          0,
          0,
          0,

          true,
          true,
          this,
      }

    , m_menuExpandButtonBox
      {
          DIR_UPLEFT,
          m_menuExpandButton.dx() - 1,
          m_menuExpandButton.dy(),
          m_menuExpandButton. w() - 1,
          m_menuExpandButton. h(),

          [](const Widget *widgetPtr, int drawDstX, int drawDstY)
          {
              g_sdlDevice->drawRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(100), drawDstX, drawDstY, widgetPtr->w(), widgetPtr->h(), 2);
          },

          this,
          false,
      }

    , m_menuImage
      {
          DIR_UPLEFT,
          0,
          0,
          {},
          {},
          [](const ImageBoard *){ return g_progUseDB->retrieve(0X00000460); },
      }

    , m_menuImageCropDup
      {
          DIR_UPLEFT,
          m_menuExpandButton.dx() + m_menuExpandButton.w(),
          m_menuExpandButton.dy() + m_menuExpandButton.h(),
          50,
          20,

          &m_menuImage,

          3,
          3,
          m_menuImage.w() - 6,
          2,

          this,
          false,
      }

    , m_menuButtonTitle
      {
          DIR_UPLEFT,
          0,
          0,

          u8"点击菜单",
          1,
          12,
          0,
      }

    , m_menuButtonTitleGfx
      {
          DIR_UPLEFT,
          0,
          0,

          200,
          250,

          {
              // {&m_menuImageCropDup, DIR_UPLEFT, 0, 0, false},
              // {&m_menuButtonTitle,  DIR_UPLEFT, 0, 0, false},
          },
      }

    , m_menuButton
      {
          DIR_UPLEFT,
          200,
          250,

          {&m_menuButtonTitle, false},
          {&m_menuBoard      , false},

          {},

          this,
          false,
      }

    , m_menuImageCropDup2
      {
          DIR_UPLEFT,
          0,
          0,
          22,
          20,

          &m_menuImage,

          3,
          3,
          m_menuImage.w() - 6,
          2,
      }

    , m_menuExpandButton2
      {
          DIR_UPLEFT,
          130,
          100,

          {0X00000055, 0X00000055, 0X00000056},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this]()
          {
          },

          0,
          0,
          0,
          0,

          true,
          true,
          this,
      }

    , m_menuExpandButtonBox2
      {
          DIR_UPLEFT,
          130,
          99,

          &m_menuImageCropDup2,

          3,
          2,
          m_menuImageCropDup2.w() - 6,
          m_menuImageCropDup2.h() - 4,

          this,
          false,
      }

    , m_leftMenuBackground
      {
          DIR_UPLEFT,
          30,
          30,
          80,
          argH - 30 * 2,

          [](const Widget *widgetPtr, int drawDstX, int drawDstY)
          {
              g_sdlDevice->fillRectangle(                             colorf::A_SHF(128), drawDstX, drawDstY, widgetPtr->w(), widgetPtr->h(), 10);
              g_sdlDevice->drawRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(100), drawDstX, drawDstY, widgetPtr->w(), widgetPtr->h(), 10);
          },

          this,
          false,
      }

    , m_leftMenu
      {
          DIR_NONE,
          m_leftMenuBackground.dx() + m_leftMenuBackground.w() / 2,
          m_leftMenuBackground.dy() + m_leftMenuBackground.h() / 2,

          to_dround(m_leftMenuBackground.w() * 0.7),

          false,
          {},

          false,

          1,
          14,
          0,
          colorf::WHITE + colorf::A_SHF(255),
          0,

          LALIGN_LEFT,
          0,
          0,

          [](const std::unordered_map<std::string, std::string> &attrList, int oldState, int newState)
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
                  }
              }
          },

          this,
          false,
      }

    , m_resizeImg0{DIR_UPLEFT, 0, 0, 22, 22, [](const ImageBoard *){ return g_progUseDB->retrieve(0X00000300); }, false, false, 1}
    , m_resizeImg1{DIR_UPLEFT, 0, 0, 22, 22, [](const ImageBoard *){ return g_progUseDB->retrieve(0X00000301); }, false, false, 1}
    , m_resizeImg2{DIR_UPLEFT, 0, 0, 22, 22, [](const ImageBoard *){ return g_progUseDB->retrieve(0X00000302); }, false, false, 1}

    , m_resizeButton
      {
          DIR_UPLEFT,
          130,
          140,

          {
              &m_resizeImg1,
              &m_resizeImg0,
              &m_resizeImg2,
          },

          {
              std::nullopt,
              std::nullopt,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this]()
          {
          },

          0,
          0,
          0,
          0,

          true,

          this,
          false,
      }

    , m_pageSystem_resolution
      {
          DIR_UPLEFT,
          0,
          0,

          u8"分辨率",
          40,

          80,
          24,

          {
              {new LabelBoard(DIR_UPLEFT, 0, 0, u8"800×600" , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), true},
              {new LabelBoard(DIR_UPLEFT, 0, 0, u8"960×600" , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), true},
              {new LabelBoard(DIR_UPLEFT, 0, 0, u8"1024×768", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), true},
              {new LabelBoard(DIR_UPLEFT, 0, 0, u8"1280×720", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), true},
              {new LabelBoard(DIR_UPLEFT, 0, 0, u8"1280×768", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), true},
              {new LabelBoard(DIR_UPLEFT, 0, 0, u8"1280×800", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), true},
          },

          [this](Widget *widgetPtr)
          {
              m_pageSystem_resolution.getMenuTitle()->setText(to_u8cstr(dynamic_cast<LabelBoard *>(widgetPtr)->getText(true)));
          },
      }

    , m_pageSystem_fullScreen
      {
          DIR_UPLEFT,
          0,
          0,

          true,
          8,

          colorf::RGBA(231, 231, 189, 128),
          16,
          16,
          m_sdRuntimeConfig.ime,

          u8"全屏显示",
          1,
          12,
          0,
          colorf::WHITE + colorf::A_SHF(255),
      }

    , m_pageSystem_showFPS
      {
          DIR_UPLEFT,
          0,
          0,

          true,
          8,

          colorf::RGBA(231, 231, 189, 128),
          16,
          16,
          m_sdRuntimeConfig.ime,

          u8"显示FPS",
          1,
          12,
          0,
          colorf::WHITE + colorf::A_SHF(255),
      }

    , m_pageSystem
      {
          DIR_UPLEFT,
          150,
          100,

          400,
          400,

          {
              {&m_pageSystem_resolution, DIR_UPLEFT, 0,   0, false},
              {&m_pageSystem_fullScreen, DIR_UPLEFT, 0,  50, false},
              {&m_pageSystem_showFPS   , DIR_UPLEFT, 0, 100, false},
          },

          this,
          false,
      }

    , m_processRun([proc]()
      {
          fflassert(proc); return proc;
      }())
{
    m_leftMenu.loadXML(
        R"###( <layout>            )###""\n"
        R"###(     <par><event id="系统">系 统</event></par> )###""\n"
        R"###(     <par>                              </par> )###""\n"
        R"###(     <par><event id="社交">社 交</event></par> )###""\n"
        R"###(     <par>                              </par> )###""\n"
        R"###(     <par><event id="网络">网 络</event></par> )###""\n"
        R"###(     <par>                              </par> )###""\n"
        R"###(     <par><event id="游戏">游 戏</event></par> )###""\n"
        R"###(     <par>                              </par> )###""\n"
        R"###(     <par><event id="帮助">帮 助</event></par> )###""\n"
        R"###( </layout>                           )###""\n"
    );

    // 1.0f -> SDL_MIX_MAXVOLUME
    // SDL_mixer initial sound/music volume is SDL_MIX_MAXVOLUME

    m_musicSlider.setValue(to_f(SDRuntimeConfig().bgmValue) / 100.0, false);
    m_soundEffectSlider.setValue(to_f(SDRuntimeConfig().soundEffValue) / 100.0, false);

    setShow(false);
}

void RuntimeConfigBoard::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    if(!show()){
        return;
    }

    for(const auto p:
    {
        static_cast<const Widget *>(&m_frameBoard          ),
        // static_cast<const Widget *>(&m_menuExpandButton    ),
        // static_cast<const Widget *>(&m_menuExpandButtonBox ),
        // static_cast<const Widget *>(&m_menuExpandButton2   ),
        // static_cast<const Widget *>(&m_menuExpandButtonBox2),
        // static_cast<const Widget *>(&m_menuImageCropDup    ),
        static_cast<const Widget *>(&m_leftMenuBackground  ),
        static_cast<const Widget *>(&m_leftMenu            ),
        static_cast<const Widget *>(&m_pageSystem          ),
        // static_cast<const Widget *>(&m_resizeButton        ),
        // static_cast<const Widget *>(&m_menuButton          ),
        // static_cast<const Widget *>(&m_checkLabel          ),
        // static_cast<const Widget *>(&m_texSliderBar        ),
        // static_cast<const Widget *>(&m_texSliderBarVertical),
    }){
        auto drawSrcX = srcX;
        auto drawSrcY = srcY;
        auto drawSrcW = srcW;
        auto drawSrcH = srcH;
        auto drawDstX = dstX;
        auto drawDstY = dstY;

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

    drawEntryTitle(u8"【游戏设置】", w() / 2, 35);

    // drawEntryTitle(u8"背景音乐", 345,  97);
    // drawEntryTitle(u8"音效",     345, 157);

    // m_musicSlider.draw();
    // m_soundEffectSlider.draw();
}

bool RuntimeConfigBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
    }

    for(auto widgetPtr:
    {
        static_cast<Widget *>(&m_leftMenu),
        // static_cast<Widget *>(&m_resizeButton),
        // static_cast<Widget *>(&m_menuExpandButton),
        // static_cast<Widget *>(&m_menuExpandButton2),
        // static_cast<Widget *>(&m_menuButton),
        // static_cast<Widget *>(&m_checkLabel),
        static_cast<Widget *>(&m_frameBoard),
        static_cast<Widget *>(&m_pageSystem),
        // static_cast<Widget *>(&m_texSliderBar),
        // static_cast<Widget *>(&m_texSliderBarVertical),
    }){
        if(widgetPtr->processEvent(event, valid)){
            return true;
        }
    }

    // if(m_musicSlider.processEvent(event, valid)){
    //     return consumeFocus(true);
    // }
    //
    // if(m_soundEffectSlider.processEvent(event, valid)){
    //     return consumeFocus(true);
    // }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                if(event.key.keysym.sym == SDLK_ESCAPE){
                    setShow(false);
                    return consumeFocus(false);
                }
                return consumeFocus(true);
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
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            {
                return consumeFocus(in(event.button.x, event.button.y));
            }
        default:
            {
                return consumeFocus(false);
            }
    }
}

void RuntimeConfigBoard::drawEntryTitle(const char8_t *info, int dstCenterX, int dstCenterY) const
{
    const LabelBoard titleBoard
    {
        DIR_NONE,
        0, // reset by new width
        0,
        to_u8cstr(info),

        1,
        12,
        0,

        colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
    };
    titleBoard.drawAt(DIR_NONE, x() + dstCenterX, y() + dstCenterY);
}

void RuntimeConfigBoard::setConfig(SDRuntimeConfig config)
{
    m_sdRuntimeConfig = std::move(config);

    m_musicSlider.setValue(m_sdRuntimeConfig.bgmValue / 100.0, false);
    m_soundEffectSlider.setValue(m_sdRuntimeConfig.soundEffValue / 100.0, false);
}

void RuntimeConfigBoard::reportRuntimeConfig()
{
    CMSetRuntimeConfig cmSRC;
    std::memset(&cmSRC, 0, sizeof(cmSRC));

    cmSRC.bgm = getConfig().bgm;
    cmSRC.bgmValue = getConfig().bgmValue;

    cmSRC.soundEff = getConfig().soundEff;
    cmSRC.soundEffValue = getConfig().soundEffValue;

    cmSRC.ime = getConfig().ime;
    cmSRC.attackMode = getConfig().attackMode;

    g_client->send(CM_SETRUNTIMECONFIG, cmSRC);
}
