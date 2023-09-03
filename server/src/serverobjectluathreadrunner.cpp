#include "serdesmsg.hpp"
#include "serverobject.hpp"
#include "serverobjectluathreadrunner.hpp"

ServerObjectLuaThreadRunner::ServerObjectLuaThreadRunner(ServerObject *serverObject)
    : ServerLuaCoroutineRunner(serverObject->m_actorPod)
{
    bindFunctionCoop("_RSVD_NAME_queryQuestUID", [this](LuaCoopResumer onDone, std::string questName)
    {
        auto closed = std::make_shared<bool>(false);
        onDone.pushOnClose([closed]()
        {
            *closed = true;
        });

        m_actorPod->forward(uidf::getServiceCoreUID(), {AM_QUERYQUESTUID, cerealf::serialize(SDQueryQuestUID
        {
            .name = std::move(questName),
        })},

        [closed, onDone](const ActorMsgPack &rmpk)
        {
            if(*closed){
                return;
            }
            else{
                onDone.popOnClose();
            }

            switch(rmpk.type()){
                case AM_UID:
                    {
                        const auto amUID = rmpk.conv<AMUID>();
                        if(amUID.UID){
                            onDone(amUID.UID);
                        }
                        else{
                            onDone();
                        }
                        break;
                    }
                default:
                    {
                        onDone();
                        break;
                    }
            }
        });
    });

    bindFunctionCoop("_RSVD_NAME_queryQuestUIDList", [this](LuaCoopResumer onDone)
    {
        auto closed = std::make_shared<bool>(false);
        onDone.pushOnClose([closed]()
        {
            *closed = true;
        });

        m_actorPod->forward(uidf::getServiceCoreUID(), AM_QUERYQUESTUIDLIST, [closed, onDone](const ActorMsgPack &rmpk)
        {
            if(*closed){
                return;
            }
            else{
                onDone.popOnClose();
            }

            switch(rmpk.type()){
                case AM_UIDLIST:
                    {
                        const auto uidList = rmpk.deserialize<SDUIDList>();
                        onDone(sol::as_table(uidList));
                        break;
                    }
                default:
                    {
                        onDone();
                        break;
                    }
            }
        });
    });

    bindFunctionCoop("_RSVD_NAME_loadMap", [this](LuaCoopResumer onDone, std::string mapName)
    {
        fflassert(str_haschar(mapName));

        auto closed = std::make_shared<bool>(false);
        onDone.pushOnClose([closed, this]()
        {
            *closed = true;
        });

        AMLoadMap amLM;
        std::memset(&amLM, 0, sizeof(AMLoadMap));

        amLM.mapID = DBCOM_MAPID(to_u8cstr(mapName));
        amLM.activateMap = true;

        m_actorPod->forward(uidf::getServiceCoreUID(), {AM_LOADMAP, amLM}, [closed, mapID = amLM.mapID, onDone, this](const ActorMsgPack &mpk)
        {
            if(*closed){
                return;
            }
            else{
                onDone.popOnClose();
            }

            switch(mpk.type()){
                case AM_LOADMAPOK:
                    {
                        const auto amLMOK = mpk.conv<AMLoadMapOK>();
                        if(amLMOK.uid){
                            onDone(amLMOK.uid);
                        }
                        else{
                            onDone();
                        }
                        break;
                    }
                default:
                    {
                        onDone();
                        break;
                    }
            }
        });
    });

}
