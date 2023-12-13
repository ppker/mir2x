#include "sdldevice.hpp"
#include "shapeclipboard.hpp"

void ShapeClipBoard::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    if(!show()){
        return;
    }

    if(!m_drawFunc){
        return;
    }

    const SDLDeviceHelper::EnableRenderClipRectangle enableClip(dstX, dstY, srcW, srcH);
    m_drawFunc(this, dstX - srcX, dstY - srcY);
}
