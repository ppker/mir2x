#include <any>
#include <memory>
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

RuntimeConfigBoard::TextInput::TextInput(
        dir8_t argDir,
        int argX,
        int argY,

        const char8_t *argLabelFirst,
        const char8_t *argLabelSecond,

        int argGapFirst,
        int argGapSecond,

        bool argIMEEnabled,

        int argInputW,
        int argInputH,

        std::function<void()> argOnTab,
        std::function<void()> argOnCR,

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

    , m_labelFirst
      {
          DIR_UPLEFT,
          0,
          0,

          argLabelFirst,
          1,
          12,
          0,
          colorf::WHITE + colorf::A_SHF(255),

          this,
          false,
      }

    , m_labelSecond
      {
          DIR_UPLEFT,
          0,
          0,

          argLabelSecond,
          1,
          12,
          0,
          colorf::WHITE + colorf::A_SHF(255),

          this,
          false,
      }

    , m_image
      {
          DIR_UPLEFT,
          0,
          0,
          {},
          {},
          [](const ImageBoard *){ return g_progUseDB->retrieve(0X00000460); },
      }

    , m_imageBg
      {
          DIR_UPLEFT,
          0,
          0,

          argInputW + 6,
          argInputH + 4,

          &m_image,

          3,
          3,
          m_image.w() - 6,
          2,

          this,
          false,
      }

    , m_input
      {
          DIR_UPLEFT,
          0,
          0,

          argInputW,
          argInputH,

          argIMEEnabled,

          1,
          12,
          0,
          colorf::WHITE + colorf::A_SHF(255),

          2,
          colorf::WHITE + colorf::A_SHF(255),

          argOnTab,
          argOnCR,

          this,
          false,
      }
{
    const int maxH = std::max<int>({m_labelFirst.h(), m_imageBg.h(), m_labelSecond.h()});

    m_labelFirst .moveAt(DIR_LEFT,                                                   0, maxH / 2);
    m_imageBg    .moveAt(DIR_LEFT, m_labelFirst.dx() + m_labelFirst.w() + argGapFirst , maxH / 2);
    m_labelSecond.moveAt(DIR_LEFT, m_imageBg   .dx() + m_imageBg   .w() + argGapSecond, maxH / 2);

    m_input.moveAt(DIR_UPLEFT, m_imageBg.dx() + 3, m_imageBg.dy() + 2);
}

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
          {},
          {},

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
          [this](ButtonBase *)
          {
              m_menuList.flipShow();
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

          {},

          10,
          0,

          argMenuList,
          std::move(argOnClickMenu),

          {10, 10, 10, 10},

          this,
          false,
      }
{
    m_menuList.setShow(false);
    const int maxHeight = std::max<int>({m_labelCrop.h(), m_menuTitleBackground.h(), m_button.h()});

    m_labelCrop          .moveAt(DIR_LEFT, 0                                 , maxHeight / 2);
    m_menuTitleBackground.moveAt(DIR_LEFT, m_labelCrop.dx() + m_labelCrop.w(), maxHeight / 2);

    m_menuTitleCrop.moveAt(DIR_LEFT, m_menuTitleBackground.dx() + 3                        , maxHeight / 2);
    m_button       .moveAt(DIR_LEFT, m_menuTitleBackground.dx() + m_menuTitleBackground.w(), maxHeight / 2);

    m_menuList.moveAt(DIR_UPLEFT, m_menuTitleBackground.dx() + 3, m_menuTitleBackground.dy() + m_menuTitleBackground.h() - 2);
}

RuntimeConfigBoard::LabelSliderBar::LabelSliderBar(
        dir8_t argDir,
        int argX,
        int argY,

        const char8_t *argLabel,
        int argLabelWidth,

        int argSliderIndex,
        int argSliderWidth,
        std::function<void(float)> argOnValueChange,

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

    , m_slider
      {
          DIR_UPLEFT,
          0,
          0,
          [argSliderWidth]{fflassert(argSliderWidth >= 0); return argSliderWidth; }(),

          true,
          argSliderIndex,
          std::move(argOnValueChange),

          this,
          false,
      }
{
    setSize(m_labelCrop.w() + m_slider.w(), std::max<int>({m_labelCrop.h(), m_slider.h()}));
    m_labelCrop.moveAt(DIR_LEFT, 0                                 , h() / 2);
    m_slider   .moveAt(DIR_LEFT, m_labelCrop.dx() + m_labelCrop.w(), h() / 2);
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

bool RuntimeConfigBoard::PullMenu::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
    }

    if(Widget::processEvent(event, valid)){
        return true;
    }

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                if(m_menuList.show()){
                    const auto [eventX, eventY] = SDLDeviceHelper::getEventPLoc(event).value();
                    if(!m_menuList.in(eventX, eventY)){
                        m_menuList.setShow(false);
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

RuntimeConfigBoard::MenuPage::TabHeader::TabHeader(
        dir8_t argDir,
        int argX,
        int argY,

        const char8_t *argLabel,
        std::function<void(ButtonBase *)> argOnClick,

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

    , m_label
      {
          DIR_UPLEFT,
          0,
          0,

          argLabel,
          1,
          14,
          0,
          colorf::WHITE + colorf::A_SHF(255),
      }

    , m_button
      {
          DIR_UPLEFT,
          0,
          0,

          {
              &m_label,
              &m_label,
              &m_label,
          },

          {
              std::nullopt,
              std::nullopt,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          std::move(argOnClick),

          0,
          0,
          1,
          1,

          true,

          this,
          false,
      }
{}

RuntimeConfigBoard::MenuPage::MenuPage(
        dir8_t argDir,
        int argX,
        int argY,

        std::optional<int> argSeperatorW,
        int argGap,

        std::initializer_list<std::tuple<const char8_t *, Widget *, bool>> argTabList,

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

    , m_buttonMask
      {
          DIR_UPLEFT,
          0,
          0,
          argSeperatorW,
          0,

          [this](const Widget *self, int drawDstX, int drawDstY)
          {
              if(m_selectedHeader){
                  g_sdlDevice->fillRectangle(colorf::RGBA(231, 231, 189, 100),
                          drawDstX + m_selectedHeader->dx(),
                          drawDstY + m_selectedHeader->dy(),

                          m_selectedHeader->w(),
                          m_selectedHeader->h());

                  g_sdlDevice->drawLine(colorf::RGBA(231, 231, 189, 100),
                          drawDstX,
                          drawDstY + m_selectedHeader->dy() + m_selectedHeader->h(),

                          drawDstX + self->w(),
                          drawDstY + m_selectedHeader->dy() + m_selectedHeader->h());
              }
          },

          this,
          false,
      }
{
    TabHeader *lastHeader = nullptr;
    TabHeader *currHeader = nullptr;

    fflassert(argGap >= 0, argGap);
    fflassert(!std::empty(argTabList));

    for(auto &[tabName, tab, autoDelete]: argTabList){
        fflassert(str_haschar(tabName));
        fflassert(tab);

        addChild(tab, autoDelete);
        addChild(currHeader = new TabHeader
        {
            DIR_UPLEFT,
            lastHeader ? (lastHeader->dx() + lastHeader->w() + 10) : 0,
            0,

            tabName,
            [this, tab = tab](ButtonBase *self)
            {
                if(m_selectedHeader){
                    std::any_cast<Widget *>(m_selectedHeader->data())->setShow(false);
                }

                tab->setShow(true);
                m_selectedHeader = self->parent();
            }
        }, true);

        currHeader->setData(tab);

        tab->setShow(lastHeader == nullptr);
        tab->moveAt(DIR_UPLEFT, 0, currHeader->dy() + currHeader->h() + argGap);

        lastHeader = currHeader;

        if(!m_selectedHeader){
            m_selectedHeader = currHeader;
        }
    }

    m_buttonMask.setSize(argSeperatorW.has_value() ? std::nullopt : std::make_optional<int>(w()), h());
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
                      for(const auto &[label, page]: std::initializer_list<std::tuple<const char8_t *, Widget *>>
                      {
                          {u8"系统", &m_pageSystem},
                          {u8"社交", &m_pageSocial},
                          {u8"游戏", &m_pageGameConfig},
                      }){
                          page->setShow(to_u8sv(id) == label);
                      }
                  }
              }
          },

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
              {(new LabelBoard(DIR_UPLEFT, 0, 0, u8"800×600" , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)))->setData(std::make_any<std::pair<int, int>>( 800, 600)), true},
              {(new LabelBoard(DIR_UPLEFT, 0, 0, u8"960×600" , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)))->setData(std::make_any<std::pair<int, int>>( 960, 600)), true},
              {(new LabelBoard(DIR_UPLEFT, 0, 0, u8"1024×768", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)))->setData(std::make_any<std::pair<int, int>>(1024, 768)), true},
              {(new LabelBoard(DIR_UPLEFT, 0, 0, u8"1280×720", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)))->setData(std::make_any<std::pair<int, int>>(1280, 720)), true},
              {(new LabelBoard(DIR_UPLEFT, 0, 0, u8"1280×768", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)))->setData(std::make_any<std::pair<int, int>>(1280, 768)), true},
              {(new LabelBoard(DIR_UPLEFT, 0, 0, u8"1280×800", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)))->setData(std::make_any<std::pair<int, int>>(1280, 800)), true},
          },

          [this](Widget *widgetPtr)
          {
              m_pageSystem_resolution.getMenuTitle()->setText(to_u8cstr(dynamic_cast<LabelBoard *>(widgetPtr)->getText(true)));
              const auto [w, h] = std::any_cast<std::pair<int, int>>(widgetPtr->data());

              g_sdlDevice->setWindowSize(w, h);
              m_sdRuntimeConfig[RTCFG_WINDOWSIZE] = std::pair<int64_t, int64_t>(w, h);
              reportRuntimeConfig(RTCFG_WINDOWSIZE);
          },
      }

    , m_pageSystem_musicSlider
      {
          DIR_UPLEFT,
          0,
          0,

          u8"音乐音量",
          60,

          1,
          80,

          [this](float val)
          {
              m_sdRuntimeConfig[RTCFG_BGMVALUE] = to_dround(val * 100);
              reportRuntimeConfig(RTCFG_BGMVALUE);
          },
      }

    , m_pageSystem_soundEffectSlider
      {
          DIR_UPLEFT,
          0,
          0,

          u8"声效音量",
          60,

          1,
          80,

          [this](float val)
          {
              m_sdRuntimeConfig[RTCFG_SEFFVALUE] = to_dround(val * 100);
              reportRuntimeConfig(RTCFG_SEFFVALUE);
          },
      }

    , m_pageSystem
      {
          DIR_UPLEFT,
          m_leftMenuBackground.dx() + m_leftMenuBackground.w() + 30,
          m_leftMenuBackground.dy() + 10,

          w() - (m_leftMenuBackground.dx() + m_leftMenuBackground.w() + 30) - 50,
          20,

          {
              {
                  u8"系统",
                  new Widget
                  {
                      DIR_UPLEFT,
                      0,
                      0,
                      {},
                      {},

                      {
                          {&m_pageSystem_resolution, DIR_UPLEFT, 0, 0, false},

                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_FULLSCREEN])), [this](Widget *){ reportRuntimeConfig(RTCFG_FULLSCREEN); }, u8"全屏显示", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)) , DIR_UPLEFT, 0, 40, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_SHOWFPS   ])), [this](Widget *){ reportRuntimeConfig(RTCFG_SHOWFPS   ); }, u8"显示FPS" , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)) , DIR_UPLEFT, 0, 65, true},

                          {new CheckLabel
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              false,
                              8,

                              colorf::RGBA(231, 231, 189, 128),
                              16,
                              16,

                              std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_BGM])),
                              [this](Widget *widgetPtr)
                              {
                                  reportRuntimeConfig(RTCFG_BGM);
                                  m_pageSystem_musicSlider.setActive(dynamic_cast<CheckBox *>(widgetPtr)->checkedValue());
                              },

                              u8"背景音乐",
                              1,
                              12,
                              0,
                              colorf::WHITE + colorf::A_SHF(255),
                          },
                          DIR_UPLEFT, 0, 100, true},

                          {&m_pageSystem_musicSlider, DIR_UPLEFT, 0, 125, false},

                          {new CheckLabel
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              false,
                              8,

                              colorf::RGBA(231, 231, 189, 128),
                              16,
                              16,

                              std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_SEFF])),
                              [this](Widget *widgetPtr)
                              {
                                  reportRuntimeConfig(RTCFG_SEFF);
                                  m_pageSystem_soundEffectSlider.setActive(dynamic_cast<CheckBox *>(widgetPtr)->checkedValue());
                              },

                              u8"动作声效",
                              1,
                              12,
                              0,
                              colorf::WHITE + colorf::A_SHF(255),
                          },
                          DIR_UPLEFT, 0,  160, true},

                          {&m_pageSystem_soundEffectSlider, DIR_UPLEFT, 0, 185, false},
                      },
                  },
                  true,
              },
          },

          this,
          false,
      }

    , m_pageSocial
      {
          DIR_UPLEFT,
          m_leftMenuBackground.dx() + m_leftMenuBackground.w() + 30,
          m_leftMenuBackground.dy() + 10,

          w() - (m_leftMenuBackground.dx() + m_leftMenuBackground.w() + 30) - 50,
          20,

          {
              {
                  u8"社交",
                  new Widget
                  {
                      DIR_UPLEFT,
                      0,
                      0,

                      {},
                      {},

                      {
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"允许私聊"    , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0,   0, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"允许白字聊天", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0,  25, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"允许地图聊天", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0,  50, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"允许行会聊天", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0,  75, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"允许全服聊天", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0, 100, true},

                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"允许加入队伍"    , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 200,   0, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"允许加入行会"    , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 200,  25, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"允许回生术"      , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 200,  50, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"允许天地合一"    , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 200,  75, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"允许交易"        , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 200, 100, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"允许添加好友"    , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 200, 125, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"允许行会召唤"    , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 200, 150, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"允许行会杀人提示", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 200, 175, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"允许拜师"        , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 200, 200, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"允许好友上线提示", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 200, 225, true},
                      },
                  },
                  true,
              },
          },

          this,
          false,
      }

    , m_pageGameConfig
      {
          DIR_UPLEFT,
          m_leftMenuBackground.dx() + m_leftMenuBackground.w() + 30,
          m_leftMenuBackground.dy() + 10,

          w() - (m_leftMenuBackground.dx() + m_leftMenuBackground.w() + 30) - 50,
          20,

          {
              {
                  u8"常用",
                  new Widget
                  {
                      DIR_UPLEFT,
                      0,
                      0,

                      {},
                      {},

                      {
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"强制攻击"    , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0,   0, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"显示体力变化", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0,  25, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"满血不显血"  , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0,  50, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"显示血条"    , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0,  75, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"数字显血"    , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0, 100, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"综合数字显示", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0, 125, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"标记攻击目标", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0, 150, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"单击解除锁定", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0, 175, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"显示BUFF图标", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0, 200, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"显示BUFF计时", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0, 225, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"显示角色名字", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0, 250, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"关闭组队血条", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0, 275, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"队友染色"    , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0, 300, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"显示队友位置", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0, 325, true},
                      },
                  },
                  true,
              },

              {
                  u8"辅助",
                  new Widget
                  {
                      DIR_UPLEFT,
                      0,
                      0,

                      {},
                      {},

                      {
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"持续盾"      , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0,   0, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"持续移花接木", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0,  25, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"持续金刚"    , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0,  50, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"持续破血"    , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0, 100, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"持续铁布衫"  , 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0, 125, true},
                      },
                  },
                  true,
              },

              {
                  u8"保护",
                  new Widget
                  {
                      DIR_UPLEFT,
                      0,
                      0,

                      {},
                      {},

                      {
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"自动喝红", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0,  0, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"保持满血", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0, 25, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"自动喝蓝", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0, 50, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, std::get_if<int64_t>(std::addressof(m_sdRuntimeConfig[RTCFG_IME])), nullptr, u8"保持满蓝", 1, 12, 0, colorf::WHITE + colorf::A_SHF(255)), DIR_UPLEFT, 0, 75, true},

                          {new TextInput(DIR_UPLEFT, 0, 0, u8"等待", u8"秒", 3, 3, true, 50, 20), DIR_UPLEFT, 0, 110, true},
                      },
                  },
                  true,
              },
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

    m_pageSystem_musicSlider      .getSlider()->setValue(0.0, false);
    m_pageSystem_soundEffectSlider.getSlider()->setValue(0.0, false);

    m_pageSystem.setShow(true);
    m_pageSocial.setShow(false);
    m_pageGameConfig.setShow(false);

    setShow(false);
}

void RuntimeConfigBoard::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    if(!show()){
        return;
    }

    for(const auto p:
    {
        static_cast<const Widget *>(&m_frameBoard),
        static_cast<const Widget *>(&m_leftMenuBackground),
        static_cast<const Widget *>(&m_leftMenu),
        static_cast<const Widget *>(&m_pageSystem),
        static_cast<const Widget *>(&m_pageSocial),
        static_cast<const Widget *>(&m_pageGameConfig),
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
        static_cast<Widget *>(&m_frameBoard),
        static_cast<Widget *>(&m_pageSystem),
        static_cast<Widget *>(&m_pageSocial),
        static_cast<Widget *>(&m_pageGameConfig),
    }){
        if(widgetPtr->processEvent(event, valid)){
            return true;
        }
    }

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
		return false;
            }
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            {
                return consumeFocus(in(event.button.x, event.button.y));
            }
        default:
            {
                return false;
            }
    }
}

void RuntimeConfigBoard::setConfig(const SDRuntimeConfig &config)
{
    m_sdRuntimeConfig = config;

    m_pageSystem_musicSlider      .setActive(std::get<int64_t>(m_sdRuntimeConfig[RTCFG_BGM ]));
    m_pageSystem_soundEffectSlider.setActive(std::get<int64_t>(m_sdRuntimeConfig[RTCFG_SEFF]));

    m_pageSystem_musicSlider      .getSlider()->setValue(std::get<int64_t>(m_sdRuntimeConfig[RTCFG_BGMVALUE ]) / 100.0, false);
    m_pageSystem_soundEffectSlider.getSlider()->setValue(std::get<int64_t>(m_sdRuntimeConfig[RTCFG_SEFFVALUE]) / 100.0, false);
}

void RuntimeConfigBoard::reportRuntimeConfig(int rtCfg)
{
    CMSetRuntimeConfig cmSRC;
    std::memset(&cmSRC, 0, sizeof(cmSRC));

    fflassert(rtCfg >= RTCFG_BEGIN, rtCfg);
    fflassert(rtCfg <  RTCFG_END  , rtCfg);

    const auto fnAssignBuf = [&cmSRC, rtCfg, this]<typename T>()
    {
        cmSRC.type = rtCfg;
        cmSRC.str  = std::is_same_v<T, std::string>;
        cmSRC.buf.assign(std::get<T>(m_sdRuntimeConfig[rtCfg]));
    };

    switch(rtCfg){
        case RTCFG_BGM       : fnAssignBuf.template operator()<int64_t>(); break;
        case RTCFG_BGMVALUE  : fnAssignBuf.template operator()<int64_t>(); break;
        case RTCFG_SEFF      : fnAssignBuf.template operator()<int64_t>(); break;
        case RTCFG_SEFFVALUE : fnAssignBuf.template operator()<int64_t>(); break;
        case RTCFG_IME       : fnAssignBuf.template operator()<int64_t>(); break;
        case RTCFG_SHOWFPS   : fnAssignBuf.template operator()<int64_t>(); break;
        case RTCFG_FULLSCREEN: fnAssignBuf.template operator()<int64_t>(); break;
        case RTCFG_ATTACKMODE: fnAssignBuf.template operator()<int64_t>(); break;
        case RTCFG_WINDOWSIZE: fnAssignBuf.template operator()<std::pair<int64_t, int64_t>>(); break;
        default:
            {
                throw fflerror("invalid runtime config type: %d", rtCfg);
            }
    }

    g_client->send(CM_SETRUNTIMECONFIG, cmSRC);
}
