#include "widget.hpp"

WidgetTreeNode::WidgetTreeNode(Widget *argParent, bool argAutoDelete)
    : m_parent(argParent)
{
    if(m_parent){
        m_parent->addChild(static_cast<Widget *>(this), argAutoDelete);
    }
}

WidgetTreeNode::~WidgetTreeNode()
{
}

Widget * WidgetTreeNode::parent(unsigned level)
{
    auto widptr = this;
    while(widptr && (level > 0)){
        widptr = widptr->m_parent;
        level--;
    }
    return static_cast<Widget *>(widptr);
}

const Widget * WidgetTreeNode::parent(unsigned level) const
{
    auto widptr = this;
    while(widptr && (level > 0)){
        widptr = widptr->m_parent;
        level--;
    }
    return static_cast<const Widget *>(widptr);
}

void WidgetTreeNode::moveFront(const Widget *widget)
{
    if(m_inLoop){
        throw fflerror("can not modify child list while in loop");
    }

    auto pivot = std::find_if(m_childList.begin(), m_childList.end(), [widget](const auto &x)
    {
        return x.widget == widget;
    });

    if(pivot == m_childList.end()){
        throw fflerror("can not find child widget");
    }

    std::rotate(m_childList.begin(), pivot, std::next(pivot));
}

void WidgetTreeNode::execDeath() noexcept
{
    for(auto &child: m_childList){
        if(child.widget){
            child.widget->execDeath();
        }
    }
    onDeath();
}

void WidgetTreeNode::addChild(Widget *widget, bool autoDelete)
{
    WidgetTreeNode *treeNode = widget;
    if(treeNode->m_parent){
        treeNode->m_parent->removeChild(widget, false);
    }

    treeNode->m_parent = static_cast<Widget *>(this);
    m_childList.emplace_front(widget, autoDelete);
}

void WidgetTreeNode::removeChild(Widget *widget, bool triggerDelete)
{
    for(auto p = m_childList.begin(); p != m_childList.end(); ++p){
        if(WidgetTreeNode * treeNode = p->widget; p->widget == widget){
            p->widget = nullptr;
            treeNode->m_parent = nullptr;

            if(triggerDelete && p->autoDelete){
                widget->execDeath();
                m_delayList.push_back(widget);
            }
            return;
        }
    }
}

void WidgetTreeNode::purge()
{
    if(m_inLoop){
        throw fflerror("can not modify child list while in loop");
    }

    foreachChild([](Widget *widget, bool)
    {
        widget->purge();
    });

    for(auto widget: m_delayList){
        delete widget;
    }

    m_delayList.clear();
    m_childList.remove_if([](const auto &x) -> bool
    {
        return x.widget == nullptr;
    });
}

void WidgetTreeNode::clearChild()
{
    for(auto &child: m_childList){
        if(child.widget){
            if(child.autoDelete){
                m_delayList.push_back(child.widget);
            }
            child.widget = nullptr;
        }
    }
}

bool WidgetTreeNode::hasChild() const
{
    return firstChild();
}

bool WidgetTreeNode::hasChild(const Widget *child) const
{
    for(auto p = m_childList.begin(); p != m_childList.end(); ++p){
        if(p->widget && p->widget == child){
            return true;
        }
    }
    return false;
}
