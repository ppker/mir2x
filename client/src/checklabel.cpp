#include "widget.hpp"
#include "sdldevice.hpp"
#include "checklabel.hpp"

CheckLabel::CheckLabel(
        dir8_t argDir,
        int argX,
        int argY,

        int &argRef,
        const char8_t *argLabel,

        Widget *argParent,
        bool argAutoDelete)
    : Widget(argDir, argX, argY, 0, 0, argParent, argAutoDelete)
    , m_checkBox
      {
      }

