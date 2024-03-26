#include <utf8.h>
#include "colorf.hpp"
#include "fflerror.hpp"
#include "fontexdb.hpp"
#include "layoutboard.hpp"
#include "imeboard.hpp"
#include "log.hpp"
#include "mathf.hpp"
#include "strf.hpp"
#include "bevent.hpp"
#include "xmltypeset.hpp"

extern Log *g_log;
extern FontexDB *g_fontexDB;
extern IMEBoard *g_imeBoard;

void LayoutBoard::loadXML(const char *xmlString, size_t parLimit)
{
    if(!xmlString){
        throw fflerror("null xmlString");
    }

    m_parNodeList.clear();
    tinyxml2::XMLDocument xmlDoc;

    if(xmlDoc.Parse(xmlString) != tinyxml2::XML_SUCCESS){
        throw fflerror("parse xml failed: %s", xmlString ? xmlString : "(null)");
    }

    bool layoutXML = false;
    const auto rootElem = xmlDoc.RootElement();

    for(const char *cstr: {"layout", "Layout", "LAYOUT"}){
        if(std::string(rootElem->Name()) == cstr){
            layoutXML = true;
            break;
        }
    }

    if(!layoutXML){
        throw fflerror("string is not layout xml");
    }

    if(rootElem->FirstAttribute()){
        g_log->addLog(LOGTYPE_WARNING, "Layout XML doesn't accept attributes, ignored.");
    }

    size_t addedParCount = 0;
    for(auto p = rootElem->FirstChild(); p && (parLimit == 0 || addedParCount < parLimit); p = p->NextSibling()){
        if(!p->ToElement()){
            g_log->addLog(LOGTYPE_WARNING, "Not an element: %s", p->Value());
            continue;
        }

        bool parXML = false;
        for(const char *cstr: {"par", "Par", "PAR"}){
            if(std::string(p->Value()) == cstr){
                parXML = true;
                break;
            }
        }

        if(!parXML){
            g_log->addLog(LOGTYPE_WARNING, "Not a paragraph element: %s", p->Value());
            continue;
        }

        addPar(parCount(), m_parNodeConfig.margin, p, false);
        addedParCount++;
    }

    setupSize();
}

void LayoutBoard::addPar(int loc, const std::array<int, 4> &parMargin, const tinyxml2::XMLNode *node, bool updateSize)
{
    if(loc < 0 || loc > parCount()){
        throw fflerror("invalid location: %d", loc);
    }

    if(!node){
        throw fflerror("null xml node");
    }

    const auto elemNode = node->ToElement();
    if(!elemNode){
        throw fflerror("can't cast to XMLElement");
    }

    bool parXML = false;
    for(const char *cstr: {"par", "Par", "PAR"}){
        if(std::string(elemNode->Name()) == cstr){
            parXML = true;
            break;
        }
    }

    if(!parXML){
        throw fflerror("not a paragraph node");
    }

    const int lineWidth = [elemNode, this]()
    {
        if(m_parNodeConfig.lineWidth <= 0){
            return -1;
        }

        // even we use default size
        // we won't let it create one-line mode by default
        // user should explicitly note that they want this special mode

        if(int lineWidth = m_parNodeConfig.lineWidth - m_parNodeConfig.margin[2] - m_parNodeConfig.margin[3]; lineWidth <= 0){
            throw fflerror("invalid default paragraph parameters");
        }
        else{
            elemNode->QueryIntAttribute("lineWidth", &lineWidth);
            if(lineWidth < -1){
                throw fflerror("invalid line width: %d", lineWidth);
            }
            return lineWidth;
        }
    }();

    const int lineAlign = [elemNode, this]()
    {
        if(const auto val = elemNode->Attribute("align")){
            if(std::string(val) == "left"       ) return LALIGN_LEFT;
            if(std::string(val) == "right"      ) return LALIGN_RIGHT;
            if(std::string(val) == "center"     ) return LALIGN_CENTER;
            if(std::string(val) == "justify"    ) return LALIGN_JUSTIFY;
            if(std::string(val) == "distributed") return LALIGN_DISTRIBUTED;
        }
        return m_parNodeConfig.align;
    }();

    const bool canThrough = [elemNode, this]()
    {
        bool canThrough = m_parNodeConfig.canThrough;
        elemNode->QueryBoolAttribute("canThrough", &canThrough);
        return canThrough;
    }();

    const uint8_t font = [elemNode, this]()
    {
        int font = m_parNodeConfig.font;
        if(const auto val = elemNode->Attribute("font")){
            try{
                font = std::stoi(val);
            }
            catch(...){
                font = g_fontexDB->findFontName(val);
            }
        }

        if(g_fontexDB->hasFont(font)){
            return font;
        }
        return 0;
    }();

    const uint8_t fontSize = [elemNode, this]()
    {
        int fontSize = m_parNodeConfig.fontSize;
        elemNode->QueryIntAttribute("size", &fontSize);
        return fontSize;
    }();

    const uint32_t fontColor = [elemNode, this]()
    {
        if(const char *val = elemNode->Attribute("color")){
            return colorf::string2RGBA(val);
        }
        return m_parNodeConfig.fontColor;
    }();

    const uint32_t fontBGColor = [elemNode, this]()
    {
        if(const char *val = elemNode->Attribute("bgcolor")){
            return colorf::string2RGBA(val);
        }
        return m_parNodeConfig.fontBGColor;
    }();

    const uint8_t fontStyle = m_parNodeConfig.fontStyle;

    const int lineSpace = [elemNode, this]()
    {
        int lineSpace = m_parNodeConfig.lineSpace;
        elemNode->QueryIntAttribute("lineSpace", &lineSpace);
        return lineSpace;
    }();

    const int wordSpace = [elemNode, this]()
    {
        int wordSpace = m_parNodeConfig.lineSpace;
        elemNode->QueryIntAttribute("wordSpace", &wordSpace);
        return wordSpace;
    }();

    auto parNodePtr = std::make_unique<XMLTypeset>
    (
        lineWidth,
        lineAlign,
        canThrough,
        font,
        fontSize,
        fontStyle,
        fontColor,
        fontBGColor,
        colorf::RGBA(0XFF, 0XFF, 0XFF, 0XFF),
        lineSpace,
        wordSpace
    );

    parNodePtr->loadXMLNode(node);
    auto currNode = m_parNodeList.insert(ithParIterator(loc), {-1, parMargin, std::move(parNodePtr)});

    if(currNode == m_parNodeList.begin()){
        currNode->startY = parMargin[1]; // TODO should be 0 here ?
    }
    else{
        const auto prevNode = std::prev(currNode);
        currNode->startY = prevNode->startY + prevNode->margin[0] + prevNode->margin[1] + prevNode->tpset->ph();
    }

    const int extraH = currNode->tpset->ph() + currNode->margin[0] + currNode->margin[1];
    for(auto p = std::next(currNode); p != m_parNodeList.end(); ++p){
        p->startY += extraH;
    }

    if(updateSize){
        setupSize();
    }
}

void LayoutBoard::setupStartY(int fromPar)
{
    fflassert(fromPar >= 0, fromPar);
    fflassert(fromPar < parCount(), fromPar, parCount());

    int startY = [this, fromPar]
    {
        if(fromPar == 0){
            return 0;
        }

        auto prev = std::prev(ithParIterator(fromPar));
        return prev->startY + prev->margin[0] + prev->margin[1] + prev->tpset->ph();
    }();

    for(auto par = fromPar; par < parCount(); ++par){
        auto parIter = ithParIterator(par);
        parIter->startY = startY;

        startY += parIter->margin[0];
        startY += parIter->margin[1];
        startY += parIter->tpset->ph();
    }
}

void LayoutBoard::addParXML(int loc, const std::array<int, 4> &parMargin, const char *xmlString)
{
    tinyxml2::XMLDocument xmlDoc;
    if(xmlDoc.Parse(xmlString) != tinyxml2::XML_SUCCESS){
        throw fflerror("parse xml failed: %s", xmlString ? xmlString : "(null)");
    }

    addPar(loc, parMargin, xmlDoc.RootElement(), true);
}

void LayoutBoard::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    for(const auto &node: m_parNodeList){
        int srcXCrop = srcX;
        int srcYCrop = srcY;
        int dstXCrop = dstX;
        int dstYCrop = dstY;
        int srcWCrop = srcW;
        int srcHCrop = srcH;

        if(!mathf::cropROI(
                    &srcXCrop, &srcYCrop,
                    &srcWCrop, &srcHCrop,
                    &dstXCrop, &dstYCrop,

                    w(),
                    h(),

                    0, node.startY, node.tpset->pw(), node.tpset->ph(), 0, 0, -1, -1)){
            continue;
        }
        node.tpset->drawEx(dstXCrop, dstYCrop, srcXCrop - node.margin[2], srcYCrop - node.startY, srcWCrop, srcHCrop);
    }
}

void LayoutBoard::setupSize()
{
    m_w = [this]() -> int
    {
        int maxW = 0;
        for(const auto &node: m_parNodeList){
            maxW = std::max<int>(maxW, node.margin[2] + node.tpset->pw() + node.margin[3]);
        }
        return maxW;
    }();

    m_h = [this]() -> int
    {
        if(empty()){
            return 0;
        }

        const auto &backNode = m_parNodeList.back();
        return backNode.startY + backNode.margin[0] + backNode.margin[1] + backNode.tpset->ph();
    }();
}

void LayoutBoard::setLineWidth(int lineWidth)
{
    m_parNodeConfig.lineWidth = lineWidth;
    for(auto &node: m_parNodeList){
        node.tpset->setLineWidth(lineWidth);
    }

    for(auto p = m_parNodeList.begin(); p != m_parNodeList.end(); ++p){
        if(p == m_parNodeList.begin()){
            p->startY = p->margin[0];
        }
        else{
            const auto prevNode = std::prev(p);
            p->startY = prevNode->startY + prevNode->margin[0] + prevNode->margin[1] + prevNode->tpset->ph();
        }
    }

    setupSize();
}

bool LayoutBoard::processEvent(const SDL_Event &event, bool valid)
{
    if(m_parNodeList.empty()){
        return false;
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                if(!focus()){
                    return false;
                }

                switch(event.key.keysym.sym){
                    case SDLK_TAB:
                        {
                            if(m_onTab){
                                m_onTab();
                            }
                            return true;
                        }
                    case SDLK_RETURN:
                        {
                            if(m_onCR){
                                m_onCR();
                            }
                            return true;
                        }
                    case SDLK_LEFT:
                        {
                            if(m_cursorLoc.x == 0 && m_cursorLoc.y == 0){
                                if(m_cursorLoc.par == 0){
                                }
                                else{
                                    m_cursorLoc.par--;
                                    std::tie(m_cursorLoc.x, m_cursorLoc.y) = ithParIterator(m_cursorLoc.par)->tpset->lastTokenLoc();
                                }
                            }
                            else{
                                std::tie(m_cursorLoc.x, m_cursorLoc.y) = ithParIterator(m_cursorLoc.par)->tpset->prevTokenLoc(m_cursorLoc.x, m_cursorLoc.y);
                            }

                            m_cursorBlink = 0.0;
                            return true;
                        }
                    case SDLK_RIGHT:
                        {
                            const auto [currLastX, currLastY] = ithParIterator(m_cursorLoc.par)->tpset->lastTokenLoc();
                            if(currLastX + 1 == m_cursorLoc.x && currLastY == m_cursorLoc.y){
                                if(m_cursorLoc.par + 1 < parCount()){
                                    m_cursorLoc.par++;
                                    m_cursorLoc.x = 0;
                                    m_cursorLoc.y = 0;
                                }
                                else{
                                }
                            }
                            else{
                                std::tie(m_cursorLoc.x, m_cursorLoc.y) = ithParIterator(m_cursorLoc.par)->tpset->nextTokenLoc(m_cursorLoc.x, m_cursorLoc.y);
                            }

                            m_cursorBlink = 0.0;
                            return true;
                        }
                    case SDLK_BACKSPACE:
                        {
                            m_cursorBlink = 0.0;
                            return true;
                        }
                    case SDLK_ESCAPE:
                        {
                            setFocus(false);
                            return true;
                        }
                    default:
                        {
                            const char keyChar = SDLDeviceHelper::getKeyChar(event, true);
                            if(m_imeEnabled && g_imeBoard->active() && (keyChar >= 'a' && keyChar <= 'z')){
                                g_imeBoard->gainFocus("", str_printf("%c", keyChar), this, [this](std::string s)
                                {
                                    ithParIterator(m_cursorLoc.par)->tpset->insertUTF8String(m_cursorLoc.x, m_cursorLoc.y, s.c_str());
                                    for(size_t i = 0, count = utf8::distance(s.begin(), s.end()); i < count; ++i){
                                        std::tie(m_cursorLoc.x, m_cursorLoc.y) = ithParIterator(m_cursorLoc.par)->tpset->nextTokenLoc(m_cursorLoc.x, m_cursorLoc.y);
                                    }

                                    setupStartY(m_cursorLoc.par);
                                    setupSize();
                                });
                            }
                            else if(keyChar != '\0'){
                                ithParIterator(m_cursorLoc.par)->tpset->insertUTF8String(m_cursorLoc.x, m_cursorLoc.y, str_printf("%c", keyChar).c_str());
                            }

                            m_cursorBlink = 0.0;
                            return true;
                        }
                }
            }
        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            {
                int locX = -1;
                int locY = -1;

                const auto fnHandleEvent = [&event, &locX, &locY, this](ParNode *node, bool currValid) -> bool
                {
                    if(!currValid){
                        node->tpset->clearEvent(-1);
                        return false;
                    }

                    switch(event.type){
                        case SDL_MOUSEMOTION:
                            {
                                locX = event.motion.x;
                                locY = event.motion.y;

                                const int dx = event.motion.x - (x()                + node->margin[2]);
                                const int dy = event.motion.y - (y() + node->startY + node->margin[0]);

                                const auto [tokenX, tokenY] = node->tpset->locToken(dx, dy, true);
                                if(!node->tpset->tokenLocValid(tokenX, tokenY)){
                                    node->tpset->clearEvent(-1);
                                    return false;
                                }

                                const auto newEvent = (event.motion.state & SDL_BUTTON_LMASK) ? BEVENT_DOWN : BEVENT_ON;
                                const auto leafID = node->tpset->getToken(tokenX, tokenY)->leaf;

                                const auto oldEvent = node->tpset->markLeafEvent(leafID, newEvent);
                                if(const auto attrListPtr = node->tpset->leafEvent(leafID); attrListPtr && m_eventCB){
                                    m_eventCB(*attrListPtr, oldEvent, newEvent);
                                }
                                node->tpset->clearEvent(leafID);
                                return true;
                            }
                        case SDL_MOUSEBUTTONUP:
                        case SDL_MOUSEBUTTONDOWN:
                            {
                                locX = event.button.x;
                                locY = event.button.y;

                                const int dx = event.button.x - (x()                + node->margin[2]);
                                const int dy = event.button.y - (y() + node->startY + node->margin[0]);

                                const auto [tokenX, tokenY] = node->tpset->locToken(dx, dy, true);
                                if(!node->tpset->tokenLocValid(tokenX, tokenY)){
                                    node->tpset->clearEvent(-1);
                                    return false;
                                }

                                const auto newEvent = (event.type == SDL_MOUSEBUTTONUP) ? BEVENT_ON : BEVENT_DOWN;
                                const auto leafID = node->tpset->getToken(tokenX, tokenY)->leaf;

                                const auto oldEvent = node->tpset->markLeafEvent(leafID, newEvent);
                                if(const auto attrListPtr = node->tpset->leafEvent(leafID); attrListPtr && m_eventCB){
                                    m_eventCB(*attrListPtr, oldEvent, newEvent);
                                }
                                node->tpset->clearEvent(leafID);
                                return true;
                            }
                        default:
                            {
                                // layout board only handle mouse motion/click events
                                // ignore any other unexcepted events

                                // for GNOME3 I found sometimes here comes SDL_KEYMAPCHANGED after SDL_MOUSEBUTTONDOWN
                                // need to ignore this event, don't call clearEvent()
                                return false;
                            }
                    }
                };

                bool takeEvent = false;
                for(auto &node: m_parNodeList){
                    takeEvent |= fnHandleEvent(&node, valid && !takeEvent);
                }

                if(!takeEvent){
                    takeEvent = in(locX, locY);
                }

                return takeEvent;
            }

        default:
            {
                return false;
            }
    }
}
