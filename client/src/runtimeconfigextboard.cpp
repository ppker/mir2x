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

    , m_checkBox
      {
          DIR_UPLEFT,
          50,
          50,

          m_sdRuntimeConfig.ime,

          this,
          false,
      }

    , m_musicSwitch
      {
          DIR_UPLEFT,
          431,
          85,

          m_sdRuntimeConfig.bgm,
          0,
          2,

          [this](int)
          {
              g_sdlDevice->setBGMVolume(getMusicVolume().value_or(0.0f));
          },
          this
      }

    , m_soundEffectSwitch
      {
          DIR_UPLEFT,
          431,
          145,

          m_sdRuntimeConfig.soundEff,
          0,
          2,

          [this](int)
          {
              g_sdlDevice->setSoundEffectVolume(getSoundEffectVolume().value_or(0.0f));
          },
          this
      }

    , m_musicSlider
      {
          DIR_UPLEFT,
          280,
          124,
          194,
          6,

          true,
          1,
          [this](float val)
          {
              m_sdRuntimeConfig.bgmValue = std::lround(val * 100.0);
              g_sdlDevice->setBGMVolume(getMusicVolume().value_or(0.0f));
              reportRuntimeConfig();
          },
          this,
      }

    , m_soundEffectSlider
      {
          DIR_UPLEFT,
          280,
          184,
          194,
          6,

          true,
          1,
          [this](float val)
          {
              m_sdRuntimeConfig.soundEffValue = std::lround(val * 100.0);
              g_sdlDevice->setSoundEffectVolume(getSoundEffectVolume().value_or(0.0f));
              reportRuntimeConfig();
          },
          this,
      }

    , m_entryProtoList
      {
          {{u8"和平攻击", u8"组队攻击", u8"行会攻击", u8"全体攻击"}, std::ref(m_sdRuntimeConfig.attackMode), ATKMODE_BEGIN, [this](int)
          {
          }},

          {{u8"拼音输入法"}, std::ref(m_sdRuntimeConfig.ime), 0, [this](int state)
          {
              g_imeBoard->setActive(state);
          }},
      }

    , m_processRun([proc]()
      {
          fflassert(proc); return proc;
      }())
{
    // 1.0f -> SDL_MIX_MAXVOLUME
    // SDL_mixer initial sound/music volume is SDL_MIX_MAXVOLUME

    m_musicSlider.setValue(to_f(SDRuntimeConfig().bgmValue) / 100.0, false);
    m_soundEffectSlider.setValue(to_f(SDRuntimeConfig().soundEffValue) / 100.0, false);

    setShow(false);

    m_switchList.reserve(m_entryProtoList.size());
    for(auto &[titleList, valueRef, valueOffset, onSwitch]: m_entryProtoList){
        // can not skip invalid entry
        // m_entryProtoList and m_switch shares indices
        fflassert(!titleList.empty());
        const auto [infoX, infoY, buttonX, buttonY] = getEntryPLoc(m_switchList.size());

        if(titleList.size() == 1){
            m_switchList.push_back(new OnOffButton
            {
                DIR_UPLEFT,
                buttonX,
                buttonY,
                valueRef,
                valueOffset,
                2,
                onSwitch,
                this,
                true,
            });
        }
        else{
            m_switchList.push_back(new SwitchNextButton
            {
                DIR_UPLEFT,
                buttonX,
                buttonY,
                valueRef,
                valueOffset,
                to_d(titleList.size()),
                onSwitch,
                this,
                true,
            });
        }
    }
}

void RuntimeConfigExtBoard::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    m_frameBoard.drawEx(dstX, dstY, srcX, srcY, srcW, srcH);

    {
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

                    m_checkBox.dx(),
                    m_checkBox.dy(),
                    m_checkBox. w(),
                    m_checkBox. h())){
            m_checkBox.drawEx(drawDstX, drawDstY, drawSrcX, drawSrcY, drawSrcW, drawSrcH);
        }
    }


    drawEntryTitle(u8"【游戏设置】", 255, 35);

    drawEntryTitle(u8"背景音乐", 345,  97);
    drawEntryTitle(u8"音效",     345, 157);

    m_musicSwitch.draw();
    m_soundEffectSwitch.draw();

    if(m_musicSwitch.getValue()){
        m_musicSlider.draw();
    }

    if(m_soundEffectSwitch.getValue()){
        m_soundEffectSlider.draw();
    }

    for(size_t i = 0; i < m_switchList.size(); ++i){
        const auto buttonPtr = m_switchList.at(i);
        const auto &titleList = std::get<0>(m_entryProtoList.at(i));
        const auto &[infoX, infoY, buttonX, buttonY] = getEntryPLoc(i);

        if(titleList.size() == 1){
            drawEntryTitle(titleList.front().c_str(), infoX, infoY);
        }
        else{
            drawEntryTitle(titleList.at(buttonPtr->getValue() - buttonPtr->getValueOffset()).c_str(), infoX, infoY);
        }
        buttonPtr->draw();
    }
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
        static_cast<Widget *>(&m_checkBox),
        static_cast<Widget *>(&m_frameBoard),
        static_cast<Widget *>(&m_musicSwitch),
        static_cast<Widget *>(&m_soundEffectSwitch),
    }){
        if(widgetPtr->processEvent(event, valid)){
            return consumeFocus(true);
        }
    }

    if(m_musicSwitch.getValue() && m_musicSlider.processEvent(event, valid)){
        return consumeFocus(true);
    }

    if(m_soundEffectSwitch.getValue() && m_soundEffectSlider.processEvent(event, valid)){
        return consumeFocus(true);
    }

    for(const auto buttonPtr: m_switchList){
        if(buttonPtr->processEvent(event, valid)){
            return consumeFocus(true);
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

std::tuple<int, int, int, int> RuntimeConfigExtBoard::getEntryPLoc(size_t entry)
{
    if(entry < 12){
        return {100, 67 + entry * 30, 186, 55 + entry * 30};
    }
    else if(entry >= 12 && entry < 19){
        return {345, 67 + (entry - 7) * 30, 431, 55 + (entry - 7) * 30};
    }
    else{
        throw fflvalue(entry);
    }
}

void RuntimeConfigExtBoard::setConfig(SDRuntimeConfig config)
{
    m_sdRuntimeConfig = std::move(config);

    m_musicSlider.setValue(m_sdRuntimeConfig.bgmValue / 100.0, false);
    g_sdlDevice->setBGMVolume(getMusicVolume().value_or(0.0f));

    m_soundEffectSlider.setValue(m_sdRuntimeConfig.soundEffValue / 100.0, false);
    g_sdlDevice->setSoundEffectVolume(getSoundEffectVolume().value_or(0.0f));

    for(auto buttonPtr: m_switchList){
        buttonPtr->triggerCallback();
    }

    for(auto buttonPtr: {&m_musicSwitch, &m_soundEffectSwitch}){
        buttonPtr->triggerCallback();
    }
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
