#pragma once
#include <list>
#include <array>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include "widget.hpp"
#include "xmltypeset.hpp"

class LayoutBoard: public Widget
{
    private:
        struct parNode
        {
            // margin[0]  up
            // margin[1]  down
            // margin[2]  left
            // margin[3]  right

            int startY = -1;
            std::array<int, 4> margin {0, 0, 0, 0};
            std::unique_ptr<XMLTypeset> tpset; // no copy support for XMLTypeset
        };

    private:
        std::list<parNode> m_parNodeList;

    private:
        struct parNodeConfig
        {
            // default setup of paragraph in LayoutBoard
            // may needed this to support theme

            int lineWidth;
            std::array<int, 4> margin;

            bool canThrough;

            uint8_t  font;
            uint8_t  fontSize;
            uint8_t  fontStyle;
            uint32_t fontColor;
            uint32_t fontBGColor;

            int align;
            int lineSpace;
            int wordSpace;

        } m_parNodeConfig;

    private:
        bool m_canSelect;

    private:
        const std::function<void(const std::unordered_map<std::string, std::string> &, int, int)> m_eventCB;

    public:
        LayoutBoard(
                Widget::VarDir    argDir,
                Widget::VarOffset argX,
                Widget::VarOffset argY,

                int argLineWidth,

                const char *argInitXML = nullptr,
                size_t argParLimit = 0,

                std::array<int, 4> argMargin = {0, 0, 0, 0},

                bool argCanSelect  = false,
                bool argCanThrough = false,

                uint8_t  argFont        =  0,
                uint8_t  argFontSize    = 10,
                uint8_t  argFontStyle   =  0,
                uint32_t argFontColor   =  colorf::WHITE + colorf::A_SHF(255),
                uint32_t argFontBGColor =  0,

                int argLineAlign = LALIGN_LEFT,
                int argLineSpace = 0,
                int argWordSpace = 0,

                const std::function<void(const std::unordered_map<std::string, std::string> &, int, int)> &argEventCB = nullptr,

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

            : Widget
              {
                  std::move(argDir),
                  std::move(argX),
                  std::move(argY),

                  0,
                  0,
                  {},

                  argParent,
                  argAutoDelete,
              }

            , m_parNodeConfig
              {
                  argLineWidth,
                  argMargin,
                  argCanThrough,
                  argFont,
                  argFontSize,
                  argFontStyle,
                  argFontColor,
                  argFontBGColor,
                  argLineAlign,
                  argLineSpace,
                  argWordSpace,
              }

            , m_canSelect(argCanSelect)
            , m_eventCB(argEventCB)
        {
            for(size_t i = 0; i < m_parNodeConfig.margin.size(); ++i){
                if(m_parNodeConfig.margin[i] < 0){
                    throw fflerror("invalid parNodeConfig::margin[%zu]: %d", i, m_parNodeConfig.margin[i]);
                }
            }

            if((m_parNodeConfig.lineWidth > 0) && (m_parNodeConfig.lineWidth <= m_parNodeConfig.margin[2] + m_parNodeConfig.margin[3])){
                throw fflerror("invalid default paragraph parameters");
            }

            if(argInitXML){
                loadXML(argInitXML, argParLimit);
            }
        }

    public:
        void loadXML(const char *, size_t = 0);

    public:
        void addParXML(int, const std::array<int, 4> &, const char *);

    public:
        void update(double ms) override
        {
            for(const auto &node: m_parNodeList){
                node.tpset->update(ms);
            }
        }

    public:
        int parCount() const
        {
            return to_d(m_parNodeList.size());
        }

    private:
        auto ithParIterator(int i)
        {
            auto p = m_parNodeList.begin();
            std::advance(p, i);
            return p;
        }

    public:
        bool empty() const
        {
            return m_parNodeList.empty();
        }

    public:
        void clear()
        {
            m_w = 0;
            m_h = 0;
            m_parNodeList.clear();
        }

    public:
        void setFont(uint8_t font)
        {
            m_parNodeConfig.font = font;
        }

        void setFontSize(uint8_t fontSize)
        {
            m_parNodeConfig.fontSize = fontSize;
        }

        void setFontStyle(uint8_t fontStyle)
        {
            m_parNodeConfig.fontStyle = fontStyle;
        }

        void setFontColor(uint32_t fontColor)
        {
            m_parNodeConfig.fontColor = fontColor;
        }

        void setFontBGColor(uint32_t fontBGColor)
        {
            m_parNodeConfig.fontBGColor = fontBGColor;
        }

        void setLineWidth(int);

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;

    private:
        void setupSize();
        void addPar(int, const std::array<int, 4> &, const tinyxml2::XMLNode *, bool);

    public:
        int getLineWidth() const
        {
            return m_parNodeConfig.lineWidth;
        }
};
