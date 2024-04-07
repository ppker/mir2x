// class Widget has no resize(), only setSize()
// widget has no box concept like gtk, it can't calculate size in parent

#pragma once
#include <any>
#include <list>
#include <utility>
#include <concepts>
#include <tuple>
#include <array>
#include <functional>
#include <variant>
#include <cstdint>
#include <optional>
#include <initializer_list>
#include <SDL2/SDL.h>
#include "mathf.hpp"
#include "lalign.hpp"
#include "bevent.hpp"
#include "fflerror.hpp"
#include "protocoldef.hpp"

class Widget
{
    public:
        using VarDir    = std::variant<             dir8_t, std::function<dir8_t(const Widget *)>>;
        using VarOffset = std::variant<                int, std::function<   int(const Widget *)>>;
        using VarSize   = std::variant<std::monostate, int, std::function<   int(const Widget *)>>;

    public:
        static bool hasIntDir(const Widget::VarDir &varDir)
        {
            return varDir.index() == 0;
        }

        static bool hasFuncDir(const Widget::VarDir &varDir)
        {
            return varDir.index() == 1;
        }

        static dir8_t  asIntDir(const Widget::VarDir &varDir) { return std::get<dir8_t>(varDir); }
        static dir8_t &asIntDir(      Widget::VarDir &varDir) { return std::get<dir8_t>(varDir); }

        static const std::function<dir8_t(const Widget *)> &asFuncDir(const Widget::VarDir &varDir) { return std::get<std::function<dir8_t(const Widget *)>>(varDir); }
        static       std::function<dir8_t(const Widget *)> &asFuncDir(      Widget::VarDir &varDir) { return std::get<std::function<dir8_t(const Widget *)>>(varDir); }

        static dir8_t evalDir(const Widget::VarDir &varDir, const Widget *widgetPtr)
        {
            return std::visit(varDispatcher
            {
                [](const dir8_t &arg)
                {
                    return arg;
                },

                [widgetPtr](const auto &arg)
                {
                    return arg ? arg(widgetPtr) : throw fflerror("invalid argument");
                },
            }, varDir);
        }

    public:
        static bool hasIntOffset(const Widget::VarOffset &varOff)
        {
            return varOff.index() == 0;
        }

        static bool hasFuncOffset(const Widget::VarOffset &varOff)
        {
            return varOff.index() == 1;
        }

        static int  asIntOffset(const Widget::VarOffset &varOff) { return std::get<int>(varOff); }
        static int &asIntOffset(      Widget::VarOffset &varOff) { return std::get<int>(varOff); }

        static const std::function<int(const Widget *)> &asFuncOffset(const Widget::VarOffset &varOff) { return std::get<std::function<int(const Widget *)>>(varOff); }
        static       std::function<int(const Widget *)> &asFuncOffset(      Widget::VarOffset &varOff) { return std::get<std::function<int(const Widget *)>>(varOff); }

        static int evalOffset(const Widget::VarOffset &varOffset, const Widget *widgetPtr)
        {
            return std::visit(varDispatcher
            {
                [](const int &arg)
                {
                    return arg;
                },

                [widgetPtr](const auto &arg)
                {
                    return arg ? arg(widgetPtr) : throw fflerror("invalid argument");
                },
            }, varOffset);
        }

    public:
        static bool hasSize(const Widget::VarSize &varSize)
        {
            return varSize.index() != 0;
        }

        static bool hasIntSize(const Widget::VarSize &varSize)
        {
            return varSize.index() == 1;
        }

        static bool hasFuncSize(const Widget::VarSize &varSize)
        {
            return varSize.index() == 2;
        }

        static int  asIntSize(const Widget::VarSize &varSize) { return std::get<int>(varSize); }
        static int &asIntSize(      Widget::VarSize &varSize) { return std::get<int>(varSize); }

        static const std::function<int(const Widget *)> &asFuncSize(const Widget::VarSize &varSize) { return std::get<std::function<int(const Widget *)>>(varSize); }
        static       std::function<int(const Widget *)> &asFuncSize(      Widget::VarSize &varSize) { return std::get<std::function<int(const Widget *)>>(varSize); }

    protected:
        Widget * m_parent;

    protected:
        bool m_show   = true;
        bool m_focus  = false;
        bool m_active = true;

    protected:
        std::any m_data;

    private:
        Widget::VarDir m_dir;

    private:
        Widget::VarOffset m_x;
        Widget::VarOffset m_y;

    protected:
        Widget::VarSize m_w;
        Widget::VarSize m_h;

    protected:
        struct ChildWidgetElement
        {
            Widget *widget     = nullptr;
            bool    autoDelete = false;
        };

        std::list<ChildWidgetElement> m_childList;

    public:
        Widget(Widget::VarDir argDir,

                Widget::VarOffset argX,
                Widget::VarOffset argY,

                Widget::VarSize argW = {},
                Widget::VarSize argH = {},

                std::initializer_list<std::tuple<Widget *, Widget::VarDir, Widget::VarOffset, Widget::VarOffset, bool>> argChildList = {},

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

            : m_parent(argParent)
            , m_dir(std::move(argDir))
            , m_x  (std::move(argX))
            , m_y  (std::move(argY))
            , m_w  (std::move(argW))
            , m_h  (std::move(argH))
        {
            if(m_parent){
                m_parent->addChild(this, argAutoDelete);
            }

            // don't check if w/h is a function
            // because it may refers to sub-widget which has not be initialized yet

            if(Widget::hasFuncDir   (m_dir)){ fflassert(Widget::asFuncDir   (m_dir), m_dir); }
            if(Widget::hasFuncOffset(m_x  )){ fflassert(Widget::asFuncOffset(m_x  ), m_x  ); }
            if(Widget::hasFuncOffset(m_y  )){ fflassert(Widget::asFuncOffset(m_y  ), m_y  ); }
            if(Widget::hasFuncSize  (m_w  )){ fflassert(Widget::asFuncSize  (m_w  ), m_w  ); }
            if(Widget::hasFuncSize  (m_h  )){ fflassert(Widget::asFuncSize  (m_h  ), m_h  ); }

            if(Widget::hasIntSize(m_w)){ fflassert(Widget::asIntSize(m_w) >= 0, m_w); }
            if(Widget::hasIntSize(m_h)){ fflassert(Widget::asIntSize(m_h) >= 0, m_h); }

            for(const auto &[childPtr, offDir, offX, offY, autoDelete]: argChildList){
                addChild(childPtr, autoDelete);
                childPtr->moveAt(offDir, offX, offY);
            }
        }

    public:
        virtual ~Widget()
        {
            clearChild();
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
                if(!p->widget->show()){
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

    private:
        static int sizeOff(int size, int index)
        {
            /**/ if(index <  0) return        0;
            else if(index == 0) return size / 2;
            else                return size - 1;
        }

        static int xSizeOff(dir8_t argDir, int argW)
        {
            switch(argDir){
                case DIR_UPLEFT   : return sizeOff(argW, -1);
                case DIR_UP       : return sizeOff(argW,  0);
                case DIR_UPRIGHT  : return sizeOff(argW,  1);
                case DIR_RIGHT    : return sizeOff(argW,  1);
                case DIR_DOWNRIGHT: return sizeOff(argW,  1);
                case DIR_DOWN     : return sizeOff(argW,  0);
                case DIR_DOWNLEFT : return sizeOff(argW, -1);
                case DIR_LEFT     : return sizeOff(argW, -1);
                default           : return sizeOff(argW,  0);
            }
        }

        static int ySizeOff(dir8_t argDir, int argH)
        {
            switch(argDir){
                case DIR_UPLEFT   : return sizeOff(argH, -1);
                case DIR_UP       : return sizeOff(argH, -1);
                case DIR_UPRIGHT  : return sizeOff(argH, -1);
                case DIR_RIGHT    : return sizeOff(argH,  0);
                case DIR_DOWNRIGHT: return sizeOff(argH,  1);
                case DIR_DOWN     : return sizeOff(argH,  1);
                case DIR_DOWNLEFT : return sizeOff(argH,  1);
                case DIR_LEFT     : return sizeOff(argH,  0);
                default           : return sizeOff(argH,  0);
            }
        }

    public:
        void drawAt(
                dir8_t dstDir,
                int    dstX,
                int    dstY,

                int dstRgnX = 0,
                int dstRgnY = 0,

                int dstRgnW = -1,
                int dstRgnH = -1) const
        {
            int srcXCrop = 0;
            int srcYCrop = 0;
            int dstXCrop = dstX - xSizeOff(dstDir, w());
            int dstYCrop = dstY - ySizeOff(dstDir, h());
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
                if(p->widget->focus()){
                    if(focusedNode == m_childList.end()){
                        focusedNode = p;
                    }
                    else{
                        throw fflerror("multiple widget focused by one event");
                    }
                }
            }

            if(focusedNode != m_childList.begin() && focusedNode != m_childList.end()){
                m_childList.push_front(*focusedNode);
                m_childList.erase(focusedNode);
            }
            return took;
        }

    public:
        static void handleEvent(const SDL_Event &event, bool &valid, std::initializer_list<Widget *> widgetList)
        {
            bool tookEvent = false;
            for(auto &w: widgetList){
                tookEvent |= w->processEvent(event, valid && !tookEvent);
            }
            valid = valid && !tookEvent;
        }

    public:
        virtual dir8_t dir() const
        {
            return Widget::evalDir(m_dir, this);
        }

        virtual int x() const
        {
            return Widget::evalOffset(m_x, this) + (m_parent ? m_parent->x() : 0) - xSizeOff(dir(), w());
        }

        virtual int y() const
        {
            return Widget::evalOffset(m_y, this) + (m_parent ? m_parent->y() : 0) - ySizeOff(dir(), h());
        }

        virtual int w() const
        {
            const auto width = std::visit(varDispatcher
            {
                [](const int &arg)
                {
                    return arg;
                },

                [this](const std::function<int(const Widget *)> &arg)
                {
                    return arg ? arg(this) : throw fflerror("invalid argument");
                },

                [this](const auto &)
                {
                    int maxW = 0;
                    for(const auto &child: m_childList){
                        if(child.widget->show()){
                            maxW = std::max<int>(maxW, child.widget->dx() + child.widget->w());
                        }
                    }
                    return maxW;
                }
            }, m_w);

            fflassert(width >= 0, width, m_w);
            return width;
        }

        virtual int h() const
        {
            const auto height = std::visit(varDispatcher
            {
                [](const int &arg)
                {
                    return arg;
                },

                [this](const std::function<int(const Widget *)> &arg)
                {
                    return arg ? arg(this) : throw fflerror("invalid argument");
                },

                [this](const auto &)
                {
                    int maxH = 0;
                    for(const auto &child: m_childList){
                        if(child.widget->show()){
                            maxH = std::max<int>(maxH, child.widget->dy() + child.widget->h());
                        }
                    }
                    return maxH;
                }
            }, m_h);

            fflassert(height >= 0, m_h);
            return height;
        }

    public:
        Widget * parent(unsigned level = 1)
        {
            auto widptr = this;
            while(widptr && (level > 0)){
                widptr = widptr->m_parent;
                level--;
            }
            return widptr;
        }

        const Widget * parent(unsigned level = 1) const
        {
            auto widptr = this;
            while(widptr && (level > 0)){
                widptr = widptr->m_parent;
                level--;
            }
            return widptr;
        }

    public:
        virtual int dx() const { return Widget::evalOffset(m_x, this) - xSizeOff(dir(), w()); }
        virtual int dy() const { return Widget::evalOffset(m_y, this) - ySizeOff(dir(), h()); }

    public:
        std::any &data()
        {
            return m_data;
        }

        Widget *setData(std::any argData)
        {
            m_data = std::move(argData);
            return this;
        }

    public:
        virtual bool in(int pixelX, int pixelY) const
        {
            return mathf::pointInRectangle<int>(pixelX, pixelY, x(), y(), w(), h());
        }

    public:
        virtual void setFocus(bool argFocus)
        {
            for(auto p = m_childList.begin(); p != m_childList.end(); ++p){
                p->widget->setFocus(false);
            }
            m_focus = argFocus;
        }

        virtual bool focus() const
        {
            for(auto p = m_childList.begin(); p != m_childList.end(); ++p){
                if(p->widget->focus()){
                    return true;
                }
            }
            return m_focus;
        }

    public:
        void clearChild()
        {
            for(auto &child: m_childList){
                if(child.autoDelete){
                    delete child.widget;
                }
            }
            m_childList.clear();
        }

    public:
        bool hasChild() const
        {
            return !m_childList.empty();
        }

        bool hasChild(const Widget *child) const
        {
            for(auto p = m_childList.begin(); p != m_childList.end(); ++p){
                if(p->widget == child){
                    return true;
                }
            }
            return false;
        }

        const Widget *hasChild(std::function<bool(const Widget *)> fnOp) const
        {
            for(auto p = m_childList.begin(); p != m_childList.end(); ++p){
                if(fnOp && fnOp(p->widget)){
                    return p->widget;
                }
            }
            return nullptr;
        }

        Widget *hasChild(std::function<bool(const Widget *)> fnOp)
        {
            for(auto p = m_childList.begin(); p != m_childList.end(); ++p){
                if(fnOp && fnOp(p->widget)){
                    return p->widget;
                }
            }
            return nullptr;
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

        bool consumeFocus(bool argFocus, Widget *child = nullptr)
        {
            if(argFocus){
                if(child){
                    if(hasChild(child)){
                        if(child->focus()){
                            // don't setup here
                            // when we setup focus in a deep call, this preserve focus of deep sub-widget
                        }
                        else{
                            setFocus(false);
                            child->setFocus(true);
                        }
                    }
                    else{
                        throw fflerror("widget has no child: %p", to_cvptr(child));
                    }
                }
                else{
                    setFocus(true);
                }
            }
            else{
                if(child){
                    throw fflerror("unexpected child: %p", to_cvptr(child));
                }
                else{
                    setFocus(false);
                }
            }
            return argFocus;
        }

    public:
        Widget *focusedChild() const
        {
            if(!m_childList.empty() && m_childList.front().widget->focus()){
                return m_childList.front().widget;
            }
            return nullptr;
        }

    public:
        virtual void setShow(bool argShow)
        {
            m_show = argShow;
        }

        virtual bool show() const
        {
            return m_show;
        }

        void flipShow()
        {
            setShow(!show());
        }

    public:
        virtual void setActive(bool argActive)
        {
            m_active = argActive;
        }

        virtual bool active() const
        {
            if(m_parent && !m_parent->active()){
                return false;
            }
            return m_active;
        }

        void flipActive()
        {
            setActive(!active());
        }

    public:
        void moveBy(Widget::VarOffset dx, Widget::VarOffset dy)
        {
            const auto fnOp = [](Widget::VarOffset u, Widget::VarOffset v) -> Widget::VarOffset
            {
                if(Widget::hasIntOffset(u) && Widget::hasIntOffset(v)){
                    return Widget::asIntOffset(u) + Widget::asIntOffset(v);
                }

                return [u = std::move(u), v = std::move(v)](const Widget *widgetPtr)
                {
                    return (Widget::hasFuncOffset(u) ? Widget::asFuncOffset(u)(widgetPtr) : Widget::asIntOffset(u))
                        +  (Widget::hasFuncOffset(v) ? Widget::asFuncOffset(v)(widgetPtr) : Widget::asIntOffset(v));
                };
            };

            m_x = fnOp(std::move(m_x), std::move(dx));
            m_y = fnOp(std::move(m_y), std::move(dy));
        }

        void moveAt(Widget::VarDir argDir, Widget::VarOffset argX, Widget::VarOffset argY)
        {
            m_dir = std::move(argDir);
            m_x   = std::move(argX  );
            m_y   = std::move(argY  );
        }

    public:
        void moveXTo(Widget::VarOffset arg) { m_x = std::move(arg); }
        void moveYTo(Widget::VarOffset arg) { m_y = std::move(arg); }

        void moveTo(Widget::VarOffset argX, Widget::VarOffset argY)
        {
            m_x = std::move(argX);
            m_y = std::move(argY);
        }

    public:
        void setW(Widget::VarSize argSize)
        {
            m_w = std::move(argSize);
        }

        void setH(Widget::VarSize argSize)
        {
            m_h = std::move(argSize);
        }

        void setSize(Widget::VarSize argW, Widget::VarSize argH)
        {
            m_w = std::move(argW);
            m_h = std::move(argH);
        }

    public:
        virtual void addChild(Widget *widget, bool autoDelete)
        {
            if(widget->m_parent){
                widget->m_parent->removeChild(widget, false);
            }

            widget->m_parent = this;
            m_childList.emplace_front(widget, autoDelete);
        }

        virtual void removeChild(Widget *widget, bool triggerDelete)
        {
            for(auto p = m_childList.begin(); p != m_childList.end(); ++p){
                if(p->widget == widget){
                    p->widget->m_parent = nullptr;
                    if(triggerDelete && p->autoDelete){
                        delete p->widget;
                    }

                    m_childList.erase(p);
                    return;
                }
            }
        }

    public:
        void moveFront(const Widget *child)
        {
            for(auto p = std::next(m_childList.begin()); p != m_childList.end(); ++p){
                if(p->widget == child){
                    m_childList.splice(m_childList.begin(), m_childList, p, std::next(p));
                    return;
                }
            }
        }

    public:
        const char *name() const
        {
            return typeid(*this).name();
        }

    public:
        template<std::invocable<const Widget *, bool> F> void foreachChild(F f) const
        {
            for(const auto &child: m_childList){
                f(child.widget, child.autoDelete);
            }
        }
};
