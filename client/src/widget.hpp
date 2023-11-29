// class Widget has no resize(), only setSize()
// widget has no box concept like gtk, it can't calculate size in parent

#pragma once
#include <list>
#include <tuple>
#include <array>
#include <cstdint>
#include <optional>
#include <SDL2/SDL.h>
#include "mathf.hpp"
#include "lalign.hpp"
#include "bevent.hpp"
#include "fflerror.hpp"
#include "protocoldef.hpp"

class Widget
{
    protected:
        Widget * const m_parent;

    protected:
        bool m_show   = true;
        bool m_focus  = false;
        bool m_active = true;

    private:
        dir8_t m_dir;

    private:
        int m_x;
        int m_y;

    protected:
        int m_w;
        int m_h;

    protected:
        // widget    : child widget pointer
        // autoDelete: automatically delete child widget
        // autoDraw  : automatically draw child widget in this->drawEx()
        //
        // when autoDraw is false
        // processEvent() and x()/y() still works in same way
        // but need to manually draw child widget in this->drawEx()
        //
        // alternative way is to define a new class for child class and override its drawEx()
        // but which requires a new class definition

        struct ChildWidgetElement
        {
            Widget *widget     = nullptr;
            bool    autoDelete = false;
            bool    autoDraw   = true;
        };

        std::list<ChildWidgetElement> m_childList;

    public:
        Widget(dir8_t argDir,

                int argX,
                int argY,
                int argW = 0,
                int argH = 0,

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false,
                bool    argAutoDraw   = true)

            : m_parent(argParent)
            , m_dir(argDir)
            , m_x(argX)
            , m_y(argY)
            , m_w(argW)
            , m_h(argH)
        {
            if(m_parent){
                m_parent->m_childList.emplace_back(this, argAutoDelete, argAutoDraw);
            }

            fflassert(m_w >= 0, m_w, m_h);
            fflassert(m_h >= 0, m_w, m_h);
        }

    public:
        virtual ~Widget()
        {
            for(auto &child: m_childList){
                if(child.autoDelete){
                    delete child.widget;
                }
            }
            m_childList.clear();
        }

    public:
        void draw() const
        {
            if(show()){
                drawEx(x(), y(), 0, 0, w(), h());
            }
        }

    public:
        virtual void drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
        {
            if(!show()){
                return;
            }

            for(auto p = m_childList.rbegin(); p != m_childList.rend(); ++p){
                if(!(p->autoDraw && p->widget->show())){
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

                            p->widget->dx(),
                            p->widget->dy(),
                            p->widget-> w(),
                            p->widget-> h())){
                    continue;
                }
                p->widget->drawEx(dstXCrop, dstYCrop, srcXCrop, srcYCrop, srcWCrop, srcHCrop);
            }
        }

    public:
        void drawAt(
                int dir,
                int dstX,
                int dstY,

                int dstRgnX = 0,
                int dstRgnY = 0,

                int dstRgnW = -1,
                int dstRgnH = -1) const
        {
            const auto [dstUpLeftX, dstUpLeftY] = [dir, dstX, dstY, this]() -> std::array<int, 2>
            {
                switch(dir){
                    case DIR_UPLEFT   : return {dstX                , dstY                };
                    case DIR_UP       : return {dstX - (w() - 1) / 2, dstY                };
                    case DIR_UPRIGHT  : return {dstX - (w() - 1)    , dstY                };
                    case DIR_RIGHT    : return {dstX - (w() - 1)    , dstY - (h() - 1) / 2};
                    case DIR_DOWNRIGHT: return {dstX - (w() - 1)    , dstY - (h() - 1)    };
                    case DIR_DOWN     : return {dstX - (w() - 1) / 2, dstY - (h() - 1)    };
                    case DIR_DOWNLEFT : return {dstX                , dstY - (h() - 1)    };
                    case DIR_LEFT     : return {dstX                , dstY - (h() - 1) / 2};
                    default           : return {dstX - (w() - 1) / 2, dstY - (h() - 1) / 2};
                }
            }();

            int srcXCrop = 0;
            int srcYCrop = 0;
            int dstXCrop = dstUpLeftX;
            int dstYCrop = dstUpLeftY;
            int srcWCrop = w();
            int srcHCrop = h();

            if(mathf::cropROI(
                        &srcXCrop, &srcYCrop,
                        &srcWCrop, &srcHCrop,
                        &dstXCrop, &dstYCrop,

                        w(),
                        h(),

                        0, 0, -1, -1,
                        dstRgnX, dstRgnY, dstRgnW, dstRgnH)){
                drawEx(dstXCrop, dstYCrop, srcXCrop, srcYCrop, srcWCrop, srcHCrop);
            }
        }

    public:
        virtual void update(double fUpdateTime)
        {
            for(auto &child: m_childList){
                child.widget->update(fUpdateTime);
            }
        }

    public:
        //  valid: this event has been consumed by other widget
        // return: does current widget take this event?
        //         always return false if given event has been take by previous widget
        virtual bool processEvent(const SDL_Event &event, bool valid)
        {
            // this function alters the draw order
            // if a widget has children having overlapping then be careful

            if(!show()){
                return false;
            }

            bool took = false;
            auto focusedNode = m_childList.end();

            for(auto p = m_childList.begin(); p != m_childList.end(); ++p){
                if(!p->widget->show()){
                    continue;
                }

                took |= p->widget->processEvent(event, valid && !took);
                if(focusedNode == m_childList.end() && p->widget->focus()){
                    focusedNode = p;
                }
            }

            if(focusedNode != m_childList.end()){
                m_childList.push_front(*focusedNode);
                m_childList.erase(focusedNode);
            }
            return took;
        }

    public:
        int x() const
        {
            const auto anchorX = [this]() -> int
            {
                if(m_parent){
                    return m_parent->x() + m_x;
                }
                else{
                    return m_x;
                }
            }();

            switch(m_dir){
                case DIR_UPLEFT   : return anchorX;
                case DIR_UP       : return anchorX - (w() - 1) / 2;
                case DIR_UPRIGHT  : return anchorX - (w() - 1);
                case DIR_RIGHT    : return anchorX - (w() - 1);
                case DIR_DOWNRIGHT: return anchorX - (w() - 1);
                case DIR_DOWN     : return anchorX - (w() - 1) / 2;
                case DIR_DOWNLEFT : return anchorX;
                case DIR_LEFT     : return anchorX;
                default           : return anchorX - (w() - 1) / 2;
            }
        }

        int y() const
        {
            const auto anchorY = [this]() -> int
            {
                if(m_parent){
                    return m_parent->y() + m_y;
                }
                else{
                    return m_y;
                }
            }();

            switch(m_dir){
                case DIR_UPLEFT   : return anchorY;
                case DIR_UP       : return anchorY;
                case DIR_UPRIGHT  : return anchorY;
                case DIR_RIGHT    : return anchorY - (h() - 1) / 2;
                case DIR_DOWNRIGHT: return anchorY - (h() - 1);
                case DIR_DOWN     : return anchorY - (h() - 1);
                case DIR_DOWNLEFT : return anchorY - (h() - 1);
                case DIR_LEFT     : return anchorY - (h() - 1) / 2;
                default           : return anchorY - (h() - 1) / 2;
            }
        }

        int w() const
        {
            return m_w;
        }

        int h() const
        {
            return m_h;
        }

        Widget * parent()
        {
            return m_parent;
        }

        const Widget * parent() const
        {
            return m_parent;
        }

    public:
        int dx() const
        {
            return x() - (m_parent ? m_parent->x() : 0);
        }

        int dy() const
        {
            return y() - (m_parent ? m_parent->y() : 0);
        }

    public:
        bool in(int pixelX, int pixelY) const
        {
            return (pixelX >= x() && pixelX < x() + w()) && (pixelY >= y() && pixelY < y() + h());
        }

    public:
        void setFocus(bool argFocus)
        {
            m_focus = argFocus;
        }

        bool focus() const
        {
            return m_focus;
        }

        // focus helper
        // we have tons of code like:
        //
        //     if(...){
        //         p->focus(true);  // get focus
        //         return true;     // since the event changes focus, then event is consumed
        //     }
        //     else{
        //         p->focus(false); // event doesn't help to move focus to the widget
        //         return false;    // not consumed, try next widget
        //     }
        //
        // this function helps to simplify the code to:
        //
        //     return p->consumeFocus(...)

        bool consumeFocus(bool argFocus)
        {
            setFocus(argFocus);
            return argFocus;
        }

    public:
        void setShow(bool argShow)
        {
            m_show = argShow;
        }

        bool show() const
        {
            return m_show;
        }

        void flipShow()
        {
            setShow(!m_show);
        }

    public:
        void setActive(bool argActive)
        {
            m_active = argActive;
        }

        bool active() const
        {
            return m_active;
        }

        void flipActive()
        {
            setActive(!m_active);
        }

    public:
        void moveBy(std::optional<int> dx, std::optional<int> dy)
        {
            m_x += dx.value_or(0);
            m_y += dy.value_or(0);
        }

        void moveTo(std::optional<int> x, std::optional<int> y)
        {
            m_x = x.value_or(m_x);
            m_y = y.value_or(m_y);
        }

        void moveAt(std::optional<dir8_t> dir, std::optional<int> x, std::optional<int> y)
        {
            m_dir = dir.value_or(m_dir);
            m_x = x.value_or(m_x);
            m_y = y.value_or(m_y);
        }

    public:
        void setSize(std::optional<int> argW, std::optional<int> argH)
        {
            const int newW = argW.value_or(m_w);
            const int newH = argH.value_or(m_h);

            fflassert(newW >= 0, newW, newH);
            fflassert(newH >= 0, newW, newH);

            m_w = newW;
            m_h = newH;
        }

        template<typename T> void setSize(const T &t)
        {
            const auto [argW, argH] = t;
            setSize(argW, argH);
        }
};
