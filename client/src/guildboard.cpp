#include "sdldevice.hpp"
#include "pngtexdb.hpp"
#include "guildboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

GuildBoard::GuildBoard(int argX, int argY, ProcessRun *runPtr, Widget *widgetPtr, bool autoDelete)
    : Widget
      {
          DIR_UPLEFT,
          argX,
          argY,
          594,
          444,
          {},

          widgetPtr,
          autoDelete,
      }

    , m_processRun(runPtr)
{}


void GuildBoard::drawEx(int dstX, int dstY, int, int, int, int) const
{
    if(auto texPtr = g_progUseDB->retrieve(0X00000500)){
        g_sdlDevice->drawTexture(texPtr, dstX, dstY);
    }
}
