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

RuntimeConfigExtBoard::RuntimeConfigExtBoard(int argX, int argY, int argW, int argH, ProcessRun *proc, Widget *widgetPtr, bool autoDelete)
    : Widget(DIR_UPLEFT, argX, argY, argW, argH, widgetPtr, autoDelete)
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
          50,
          130,

          colorf::RGBA(231, 231, 189, 128),
          16,
          16,
          m_sdRuntimeConfig.ime,

          8,

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
          50,
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
          50,
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
          280,
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
          280,
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

          colorf::RGBA(231, 231, 189, 128),
          16,
          16,
          m_sdRuntimeConfig.ime,

          8,

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

          {10, 10, 10, 10},
      }

    , m_menuButtonGfx
      {
          DIR_UPLEFT,
          0,
          0,

          u8"点击菜单",
          1,
          12,
          0,
      }

    , m_menuButton
      {
          DIR_UPLEFT,
          200,
          250,

          {&m_menuButtonGfx, false},
          {&m_menuBoard    , false},

          {},

          this,
          false,
      }

    , m_selectBoard
      {
          DIR_UPLEFT,
          20,
          20,
          50,

          false,
          {},

          false,

          1,
          12,
          0,
          colorf::WHITE + colorf::A_SHF(255),
          0,

          LALIGN_LEFT,
          0,
          0,

          [](const std::unordered_map<std::string, std::string> &, int, int)
          {
          },

          this,
          false,
      }

    , m_processRun([proc]()
      {
          fflassert(proc); return proc;
      }())
{
    m_selectBoard.loadXML(
        R"###( <layout>            )###""\n"
        R"###(     <par><event>系统</event></par> )###""\n"
        R"###(     <par><event>社交</event></par> )###""\n"
        R"###(     <par><event>网络</event></par> )###""\n"
        R"###(     <par><event>游戏</event></par> )###""\n"
        R"###(     <par><event>帮助</event></par> )###""\n"
        R"###( </layout>           )###""\n"
    );

    // 1.0f -> SDL_MIX_MAXVOLUME
    // SDL_mixer initial sound/music volume is SDL_MIX_MAXVOLUME

    m_musicSlider.setValue(to_f(SDRuntimeConfig().bgmValue) / 100.0, false);
    m_soundEffectSlider.setValue(to_f(SDRuntimeConfig().soundEffValue) / 100.0, false);

    setShow(false);
}

void RuntimeConfigExtBoard::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    m_frameBoard.drawEx(dstX, dstY, srcX, srcY, srcW, srcH);
    for(auto p:
    {
        static_cast<const Widget *>(&m_selectBoard),
        static_cast<const Widget *>(&m_menuButton),
        static_cast<const Widget *>(&m_checkLabel),
        static_cast<const Widget *>(&m_texSliderBar),
        static_cast<const Widget *>(&m_texSliderBarVertical),
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

    drawEntryTitle(u8"【游戏设置】", 255, 35);

    drawEntryTitle(u8"背景音乐", 345,  97);
    drawEntryTitle(u8"音效",     345, 157);

    m_musicSlider.draw();
    m_soundEffectSlider.draw();
}

bool RuntimeConfigExtBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!show()){
        return consumeFocus(false);
    }

    for(auto widgetPtr:
    {
        static_cast<Widget *>(&m_selectBoard),
        static_cast<Widget *>(&m_menuButton),
        static_cast<Widget *>(&m_checkLabel),
        static_cast<Widget *>(&m_frameBoard),
        static_cast<Widget *>(&m_texSliderBar),
        static_cast<Widget *>(&m_texSliderBarVertical),
    }){
        if(widgetPtr->processEvent(event, valid)){
            return true;
        }
    }

    if(m_musicSlider.processEvent(event, valid)){
        return consumeFocus(true);
    }

    if(m_soundEffectSlider.processEvent(event, valid)){
        return consumeFocus(true);
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

void RuntimeConfigExtBoard::drawEntryTitle(const char8_t *info, int dstCenterX, int dstCenterY) const
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

void RuntimeConfigExtBoard::setConfig(SDRuntimeConfig config)
{
    m_sdRuntimeConfig = std::move(config);

    m_musicSlider.setValue(m_sdRuntimeConfig.bgmValue / 100.0, false);
    m_soundEffectSlider.setValue(m_sdRuntimeConfig.soundEffValue / 100.0, false);
}

void RuntimeConfigExtBoard::reportRuntimeConfig()
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
