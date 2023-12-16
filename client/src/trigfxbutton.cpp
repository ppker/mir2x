#include "colorf.hpp"
#include "trigfxbutton.hpp"

TrigfxButton::TrigfxButton(
        dir8_t argDir,
        int argX,
        int argY,

        std::array<const Widget *, 3> gfxList,
        std::array<std::optional<uint32_t>, 3> seffIDList,

        std::function<void()> fnOnOverIn,
        std::function<void()> fnOnOverOut,
        std::function<void()> fnOnClick,

        int offXOnOver,
        int offYOnOver,
        int offXOnClick,
        int offYOnClick,

        bool onClickDone,

        Widget *widgetPtr,
        bool    autoDelete)

    : ButtonBase
      {
          argDir,
          argX,
          argY,
          0,
          0,

          std::move(fnOnOverIn),
          std::move(fnOnOverOut),
          std::move(fnOnClick),

          seffIDList[0],
          seffIDList[1],
          seffIDList[2],

          offXOnOver,
          offYOnOver,
          offXOnClick,
          offYOnClick,

          onClickDone,
          widgetPtr,
          autoDelete,
      }

    , m_gfxList
      {
          gfxList[0],
          gfxList[1],
          gfxList[2],
      }
{
    initButtonSize();
}

void TrigfxButton::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    if(auto gfxPtr = m_gfxList[getState()]){
        const int offX = m_offset[getState()][0];
        const int offY = m_offset[getState()][1];
        gfxPtr->drawEx(dstX + offX, dstY + offY, srcX, srcY, srcW, srcH);
    }
}

void TrigfxButton::initButtonSize()
{
    int maxW = 0;
    int maxH = 0;

    for(const auto &p: m_gfxList){
        maxW = std::max<int>(p->w(), maxW);
        maxH = std::max<int>(p->h(), maxH);
    }

    setSize(maxW, maxH);
}
