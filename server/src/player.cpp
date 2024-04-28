#include <cinttypes>
#include "dbpod.hpp"
#include "player.hpp"
#include "luaf.hpp"
#include "uidf.hpp"
#include "jobf.hpp"
#include "pathf.hpp"
#include "mathf.hpp"
#include "dbcomid.hpp"
#include "sysconst.hpp"
#include "netdriver.hpp"
#include "charobject.hpp"
#include "friendtype.hpp"
#include "protocoldef.hpp"
#include "buildconfig.hpp"
#include "serverargparser.hpp"

extern DBPod *g_dbPod;
extern NetDriver *g_netDriver;
extern MonoServer *g_monoServer;
extern ServerArgParser *g_serverArgParser;

Player::LuaThreadRunner::LuaThreadRunner(Player *playerPtr)
    : BattleObject::LuaThreadRunner(playerPtr)
{
    bindFunction("getLevel", [this]() -> uint64_t
    {
        return getPlayer()->level();
    });

    bindFunction("getGold", [this]() -> uint64_t
    {
        return getPlayer()->gold();
    });

    bindFunction("getGender", [this]() -> bool
    {
        return getPlayer()->gender();
    });

    bindFunction("getName", [this]() -> std::string
    {
        return getPlayer()->name();
    });

    bindFunction("getWLItem", [this](int wlType, sol::this_state s) -> sol::object
    {
        if(const auto &item = getPlayer()->m_sdItemStorage.wear.getWLItem(wlType)){
            return luaf::buildLuaObj(sol::state_view(s), item.asLuaVar());
        }
        else{
            return sol::make_object(sol::state_view(s), sol::nil);
        }
    });

    bindFunction("getTeamLeader", [this](sol::this_state s) -> sol::object
    {
        sol::state_view sv(s);
        if(getPlayer()->m_teamLeader){
            return sol::object(sv, sol::in_place_type<lua_Integer>, getPlayer()->m_teamLeader);
        }
        else{
            return sol::make_object(sv, sol::nil);
        }
    });

    bindFunctionCoop("_RSVD_NAME_getTeamMemberList", [this](LuaCoopResumer onDone)
    {
        auto closed = std::make_shared<bool>(false);
        onDone.pushOnClose([closed]()
        {
            *closed = true;
        });

        getPlayer()->pullTeamMemberList([closed, onDone](std::optional<SDTeamMemberList> sdTML)
        {
            if(*closed){
                return;
            }
            else{
                onDone.popOnClose();
            }

            if(sdTML.has_value()){
                onDone(sol::as_table(sdTML.value().getUIDList()));
            }
            else{
                onDone();
            }
        });
    });

    bindFunction("_RSVD_NAME_runQuestTrigger", [this](uint64_t questUID, int triggerType, sol::variadic_args args)
    {
        fflassert(uidf::isQuest(questUID), uidf::getUIDString(questUID));

        fflassert(triggerType >= SYS_ON_BEGIN, triggerType);
        fflassert(triggerType <  SYS_ON_END  , triggerType);

        switch(triggerType){
            case SYS_ON_LEVELUP:
                {
                    const auto [oldLevel, newLevel] = [&args, this]() -> std::tuple<int, int>
                    {
                        switch(args.size()){
                            case 1:
                                {
                                    fflassert(args[0].is<lua_Integer>());
                                    return {args[0].as<lua_Integer>(), getPlayer()->level()};
                                }
                            case 2:
                                {
                                    fflassert(args[0].is<lua_Integer>());
                                    fflassert(args[1].is<lua_Integer>());
                                    return {args[0].as<lua_Integer>(), args[1].as<lua_Integer>()};
                                }
                            default:
                                {
                                    throw fflvalue(args.size());
                                }
                        }
                    }();

                    getPlayer()->m_actorPod->forward(questUID, {AM_RUNQUESTTRIGGER, cerealf::serialize<SDQuestTriggerVar>(SDQuestTriggerLevelUp
                    {
                        .oldLevel = oldLevel,
                        .newLevel = newLevel,
                    })});
                    break;
                }
            case SYS_ON_KILL:
                {
                    fflassert(args.size() == 1, args.size());
                    fflassert(args[0].is<lua_Integer>());

                    const auto monsterID = to_u32(args[0].as<lua_Integer>());
                    getPlayer()->m_actorPod->forward(questUID, {AM_RUNQUESTTRIGGER, cerealf::serialize<SDQuestTriggerVar>(SDQuestTriggerKill
                    {
                        .monsterID = monsterID,
                    })});
                    break;
                }
            case SYS_ON_GAINEXP:
                {
                    fflassert(args.size() == 1, args.size());
                    fflassert(args[0].is<lua_Integer>());

                    const auto addedExp = to_d(args[0].as<lua_Integer>());
                    getPlayer()->m_actorPod->forward(questUID, {AM_RUNQUESTTRIGGER, cerealf::serialize<SDQuestTriggerVar>(SDQuestTriggerGainExp
                    {
                        .addedExp = addedExp,
                    })});
                    break;
                }
            case SYS_ON_GAINGOLD:
                {
                    fflassert(args.size() == 1, args.size());
                    fflassert(args[0].is<lua_Integer>());

                    const auto addedGold = to_d(args[0].as<lua_Integer>());
                    getPlayer()->m_actorPod->forward(questUID, {AM_RUNQUESTTRIGGER, cerealf::serialize<SDQuestTriggerVar>(SDQuestTriggerGainGold
                    {
                        .addedGold = addedGold,
                    })});
                    break;
                }
            case SYS_ON_GAINITEM:
                {
                    fflassert(args.size() == 1, args.size());
                    fflassert(args[0].is<lua_Integer>());

                    const auto itemID = to_u32(args[0].as<lua_Integer>());
                    getPlayer()->m_actorPod->forward(questUID, {AM_RUNQUESTTRIGGER, cerealf::serialize<SDQuestTriggerVar>(SDQuestTriggerGainItem
                    {
                        .itemID = itemID,
                    })});
                    break;
                }
            default:
                {
                    break;
                }
        }
    });

    bindFunction("postRawString", [this](std::string msg)
    {
        getPlayer()->postNetMessage(SM_TEXT, msg);
    });

    bindFunction("secureItem", [this](uint32_t itemID, uint32_t seqID)
    {
        getPlayer()->secureItem(itemID, seqID);
    });

    bindFunction("reportSecuredItemList", [this]()
    {
        getPlayer()->reportSecuredItemList();
    });

    bindFunction("addItem", [this](int itemID, int itemCount)
    {
        const auto &ir = DBCOM_ITEMRECORD(itemID);
        fflassert(ir);
        fflassert(itemCount > 0);

        if(ir.isGold()){
            getPlayer()->setGold(getPlayer()->getGold() + itemCount);
        }
        else{
            int added = 0;
            while(added < itemCount){
                const auto &addedItem = getPlayer()->addInventoryItem(SDItem
                {
                    .itemID = to_u32(itemID),
                    .seqID  = 1,
                    .count = std::min<size_t>(ir.packable() ? SYS_INVGRIDMAXHOLD : 1, itemCount - added),
                }, false);
                added += addedItem.count;
            }
        }
    });

    bindFunction("removeItem", [this](int itemID, int seqID, int count) -> bool
    {
        fflassert(itemID >  0, itemID);
        fflassert( seqID >= 0,  seqID);
        fflassert( count >  0,  count);

        const auto argItemID = to_u32(itemID);
        const auto argSeqID  = to_u32( seqID);
        const auto argCount  = to_uz ( count);

        const auto &ir = DBCOM_ITEMRECORD(argItemID);
        fflassert(ir);

        if(ir.isGold()){
            fflassert(argSeqID == 0, argSeqID);
            if(getPlayer()->m_sdItemStorage.gold >= argCount){
                getPlayer()->setGold(getPlayer()->m_sdItemStorage.gold - argCount);
                return true;
            }
            else{
                return false;
            }
        }
        else if(argSeqID > 0){
            fflassert(argCount == 1, argCount);
            return getPlayer()->removeInventoryItem(argItemID, argSeqID) > 0;
        }
        else{
            fflassert(argCount > 0);
            if(getPlayer()->hasInventoryItem(argItemID, argSeqID, argCount)){
                getPlayer()->removeInventoryItem(argItemID, 0, argCount);
                return true;
            }
            else{
                return false;
            }
        }
    });

    bindFunction("hasItem", [this](int itemID, int seqID, size_t count) -> bool
    {
        fflassert(itemID >  0, itemID);
        fflassert( seqID >= 0,  seqID);
        fflassert( count >  0,  count);

        return getPlayer()->hasInventoryItem(to_u32(itemID), to_u32(seqID), count);
    });

    bindFunction("dbGetVar", [this](std::string var, sol::this_state s)
    {
        return luaf::buildLuaObj(sol::state_view(s), getPlayer()->dbGetVar(var));
    });

    bindFunction("dbSetVar", [this](std::string var, sol::object value)
    {
        getPlayer()->dbSetVar(var, luaf::buildLuaVar(value));
    });

    bindFunction("dbHasVar", [this](std::string var, sol::this_state s)
    {
        auto &&[found, value] = getPlayer()->dbHasVar(var);

        sol::state_view sv(s);
        std::vector<sol::object> resList;

        if(found){
            resList.reserve(2);
            resList.push_back(sol::object(sv, sol::in_place_type<bool>, true));
            resList.push_back(luaf::buildLuaObj(sv, std::move(value)));
        }
        else{
            resList.reserve(1);
            resList.push_back(sol::object(sv, sol::in_place_type<bool>, false));
        }

        return sol::as_returns(resList);
    });

    bindFunction("dbRemoveVar", [this](std::string var)
    {
        getPlayer()->dbRemoveVar(std::move(var));
    });

    bindFunction("_RSVD_NAME_reportQuestDespList", [this](sol::object obj)
    {
        fflassert(obj.is<sol::table>(), luaf::luaObjTypeString(obj));
        SDQuestDespList sdQDL {};

        for(const auto &[quest, table]: obj.as<sol::table>()){
            fflassert(quest.is<std::string>(), luaf::luaObjTypeString(quest));
            fflassert(table.is<sol::table >(), luaf::luaObjTypeString(table));

            for(const auto &[fsm, desp]: table.as<sol::table>()){
                fflassert(fsm .is<std::string>(), luaf::luaObjTypeString(fsm ));
                fflassert(desp.is<std::string>(), luaf::luaObjTypeString(desp));
                sdQDL[quest.as<std::string>()][fsm.as<std::string>()] = desp.as<std::string>();
            }
        }

        getPlayer()->postNetMessage(SM_QUESTDESPLIST, cerealf::serialize(sdQDL));
    });

    bindFunctionCoop("_RSVD_NAME_spaceMove", [this](LuaCoopResumer onDone, uint32_t argMapID, int argX, int argY)
    {
        auto closed = std::make_shared<bool>(false);
        onDone.pushOnClose([closed]()
        {
            *closed = true;
        });

        const auto &mr = DBCOM_MAPRECORD(argMapID);
        fflassert(mr, argMapID);

        fflassert(argX >= 0, argX);
        fflassert(argY >= 0, argY);

        if(to_u32(argMapID) == getPlayer()->mapID()){
            getPlayer()->requestSpaceMove(argX, argY, false, [closed, onDone, this]()
            {
                if(*closed){
                    return;
                }
                else{
                    onDone.popOnClose();
                }
                onDone(getPlayer()->mapID(), getPlayer()->X(), getPlayer()->Y());
            },

            [closed, onDone]()
            {
                if(*closed){
                    return;
                }
                else{
                    onDone.popOnClose();
                }
                onDone();
            });
        }
        else{
            getPlayer()->requestMapSwitch(argMapID, argX, argY, false, [closed, onDone, this]()
            {
                if(*closed){
                    return;
                }
                else{
                    onDone.popOnClose();
                }
                onDone(getPlayer()->mapID(), getPlayer()->X(), getPlayer()->Y());
            },

            [closed, onDone]()
            {
                if(*closed){
                    return;
                }
                else{
                    onDone.popOnClose();
                }
                onDone();
            });
        }
    });

    bindFunctionCoop("_RSVD_NAME_randomMove", [this](LuaCoopResumer onDone)
    {
        const auto newGLoc = [this]() -> std::optional<std::array<int, 2>>
        {
            const int startDir = pathf::getRandDir();
            for(int i = 0; i < 8; ++i){
                if(const auto [newX, newY] = pathf::getFrontGLoc(getPlayer()->X(), getPlayer()->Y(), pathf::getNextDir(startDir, i)); getPlayer()->m_map->groundValid(newX, newY)){
                    return {{newX, newY}};
                }
            }
            return {};
        }();

        if(newGLoc.has_value()){
            auto closed = std::make_shared<bool>(false);
            onDone.pushOnClose([closed]()
            {
                *closed = true;
            });

            const auto [newX, newY] = newGLoc.value();
            getPlayer()->requestMove(newX, newY, SYS_DEFSPEED, false, false, [closed, oldX = getPlayer()->X(), oldY = getPlayer()->Y(), onDone, this]()
            {
                if(*closed){
                    return;
                }
                else{
                    onDone.popOnClose();
                }

                // player doesn't sendback its move to client in requestMove() because player's move usually driven by client
                // but here need to sendback the forced move since it's driven by server

                getPlayer()->reportAction(getPlayer()->UID(), getPlayer()->mapID(), ActionMove
                {
                    .speed = SYS_DEFSPEED,
                    .x = oldX,
                    .y = oldY,
                    .aimX = getPlayer()->X(),
                    .aimY = getPlayer()->Y(),
                });

                onDone(getPlayer()->mapID(), getPlayer()->X(), getPlayer()->Y());
            },

            [closed, onDone]()
            {
                if(*closed){
                    return;
                }
                else{
                    onDone.popOnClose();
                }
                onDone();
            });
        }
        else{
            onDone();
        }
    });

    bindFunctionCoop("_RSVD_NAME_queryQuestTriggerList", [this](LuaCoopResumer onDone, int triggerType)
    {
        fflassert(triggerType >= SYS_ON_BEGIN, triggerType);
        fflassert(triggerType <  SYS_ON_END  , triggerType);

        auto closed = std::make_shared<bool>(false);
        onDone.pushOnClose([closed]()
        {
            *closed = true;
        });

        AMQueryQuestTriggerList amQQTL;
        std::memset(&amQQTL, 0, sizeof(amQQTL));

        amQQTL.type = triggerType;

        getPlayer()->m_actorPod->forward(uidf::getServiceCoreUID(), {AM_QUERYQUESTTRIGGERLIST, amQQTL}, [closed, onDone, this](const ActorMsgPack &rmpk)
        {
            if(*closed){
                return;
            }
            else{
                onDone.popOnClose();
            }

            switch(rmpk.type()){
                case AM_OK:
                    {
                        onDone(rmpk.deserialize<std::vector<uint64_t>>());
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

    pfrCheck(execRawString(BEGIN_LUAINC(char)
#include "player.lua"
    END_LUAINC()));
}

Player::Player(const SDInitPlayer &initParam, const ServerMap *mapPtr)
    : BattleObject(mapPtr, uidf::getPlayerUID(initParam.dbid), initParam.x, initParam.y, DIR_DOWN)
    , m_exp(initParam.exp)
    , m_gender(initParam.gender)
    , m_job(initParam.job)
    , m_name(initParam.name)
    , m_nameColor(initParam.nameColor)
    , m_hair(initParam.hair)
    , m_hairColor(initParam.hairColor)
{
    m_sdHealth.uid = UID();
    m_sdHealth.hp = initParam.hp;
    m_sdHealth.mp = initParam.mp;
    m_sdHealth.maxHP = maxHP();
    m_sdHealth.maxMP = maxMP();
    m_sdHealth.hpRecover = 1;
    m_sdHealth.hpRecover = 1;

    m_sdItemStorage.gold = initParam.gold;

    dbLoadWear();
    dbLoadBelt();
    dbLoadInventory();
    dbLoadFriendList();
    dbLoadLearnedMagic();
    dbLoadPlayerConfig();
}

void Player::onActivate()
{
    BattleObject::onActivate();
    m_luaRunner = std::make_unique<Player::LuaThreadRunner>(this);
    m_luaRunner->spawn(m_threadKey++, "_RSVD_NAME_setupQuests()");
}

void Player::operateAM(const ActorMsgPack &rstMPK)
{
    switch(rstMPK.type()){
        case AM_METRONOME:
            {
                on_AM_METRONOME(rstMPK);
                break;
            }
        case AM_BADACTORPOD:
            {
                on_AM_BADACTORPOD(rstMPK);
                break;
            }
        case AM_NOTIFYNEWCO:
            {
                on_AM_NOTIFYNEWCO(rstMPK);
                break;
            }
        case AM_QUERYHEALTH:
            {
                on_AM_QUERYHEALTH(rstMPK);
                break;
            }
        case AM_CHECKMASTER:
            {
                on_AM_CHECKMASTER(rstMPK);
                break;
            }
        case AM_MAPSWITCHTRIGGER:
            {
                on_AM_MAPSWITCHTRIGGER(rstMPK);
                break;
            }
        case AM_QUERYLOCATION:
            {
                on_AM_QUERYLOCATION(rstMPK);
                break;
            }
        case AM_QUERYFRIENDTYPE:
            {
                on_AM_QUERYFRIENDTYPE(rstMPK);
                break;
            }
        case AM_EXP:
            {
                on_AM_EXP(rstMPK);
                break;
            }
        case AM_ADDBUFF:
            {
                on_AM_ADDBUFF(rstMPK);
                break;
            }
        case AM_REMOVEBUFF:
            {
                on_AM_REMOVEBUFF(rstMPK);
                break;
            }
        case AM_MISS:
            {
                on_AM_MISS(rstMPK);
                break;
            }
        case AM_HEAL:
            {
                on_AM_HEAL(rstMPK);
                break;
            }
        case AM_ACTION:
            {
                on_AM_ACTION(rstMPK);
                break;
            }
        case AM_ATTACK:
            {
                on_AM_ATTACK(rstMPK);
                break;
            }
        case AM_DEADFADEOUT:
            {
                on_AM_DEADFADEOUT(rstMPK);
                break;
            }
        case AM_BINDCHANNEL:
            {
                on_AM_BINDCHANNEL(rstMPK);
                break;
            }
        case AM_SENDPACKAGE:
            {
                on_AM_SENDPACKAGE(rstMPK);
                break;
            }
        case AM_RECVPACKAGE:
            {
                on_AM_RECVPACKAGE(rstMPK);
                break;
            }
        case AM_QUERYUIDBUFF:
            {
                on_AM_QUERYUIDBUFF(rstMPK);
                break;
            }
        case AM_QUERYCORECORD:
            {
                on_AM_QUERYCORECORD(rstMPK);
                break;
            }
        case AM_BADCHANNEL:
            {
                on_AM_BADCHANNEL(rstMPK);
                break;
            }
        case AM_OFFLINE:
            {
                on_AM_OFFLINE(rstMPK);
                break;
            }
        case AM_QUERYPLAYERNAME:
            {
                on_AM_QUERYPLAYERNAME(rstMPK);
                break;
            }
        case AM_QUERYPLAYERWLDESP:
            {
                on_AM_QUERYPLAYERWLDESP(rstMPK);
                break;
            }
        case AM_REMOVEGROUNDITEM:
            {
                on_AM_REMOVEGROUNDITEM(rstMPK);
                break;
            }
        case AM_CORECORD:
            {
                on_AM_CORECORD(rstMPK);
                break;
            }
        case AM_NOTIFYDEAD:
            {
                on_AM_NOTIFYDEAD(rstMPK);
                break;
            }
        case AM_REMOTECALL:
            {
                on_AM_REMOTECALL(rstMPK);
                break;
            }
        case AM_REQUESTJOINTEAM:
            {
                on_AM_REQUESTJOINTEAM(rstMPK);
                break;
            }
        case AM_REQUESTLEAVETEAM:
            {
                on_AM_REQUESTLEAVETEAM(rstMPK);
                break;
            }
        case AM_QUERYTEAMPLAYER:
            {
                on_AM_QUERYTEAMPLAYER(rstMPK);
                break;
            }
        case AM_QUERYTEAMMEMBERLIST:
            {
                on_AM_QUERYTEAMMEMBERLIST(rstMPK);
                break;
            }
        case AM_TEAMUPDATE:
            {
                on_AM_TEAMUPDATE(rstMPK);
                break;
            }
        default:
            {
                g_monoServer->addLog(LOGTYPE_WARNING, "Unsupported message: %s", mpkName(rstMPK.type()));
                break;
            }
    }
}

void Player::operateNet(uint8_t nType, const uint8_t *pData, size_t nDataLen, uint64_t respID)
{
    switch(nType){
#define _support_cm(cm) case cm: net_##cm(nType, pData, nDataLen, respID); break
        _support_cm(CM_ACTION                    );
        _support_cm(CM_BUY                       );
        _support_cm(CM_ADDFRIEND                 );
        _support_cm(CM_CHATMESSAGE               );
        _support_cm(CM_CONSUMEITEM               );
        _support_cm(CM_DROPITEM                  );
        _support_cm(CM_MAKEITEM                  );
        _support_cm(CM_NPCEVENT                  );
        _support_cm(CM_PICKUP                    );
        _support_cm(CM_PING                      );
        _support_cm(CM_QUERYCORECORD             );
        _support_cm(CM_QUERYGOLD                 );
        _support_cm(CM_QUERYPLAYERNAME           );
        _support_cm(CM_QUERYPLAYERWLDESP         );
        _support_cm(CM_QUERYCHATPEERLIST            );
        _support_cm(CM_QUERYSELLITEMLIST         );
        _support_cm(CM_QUERYUIDBUFF              );
        _support_cm(CM_REQUESTADDEXP             );
        _support_cm(CM_REQUESTEQUIPBELT          );
        _support_cm(CM_REQUESTEQUIPWEAR          );
        _support_cm(CM_REQUESTGRABBELT           );
        _support_cm(CM_REQUESTGRABWEAR           );
        _support_cm(CM_REQUESTJOINTEAM           );
        _support_cm(CM_REQUESTKILLPETS           );
        _support_cm(CM_REQUESTLEAVETEAM          );
        _support_cm(CM_REQUESTRETRIEVESECUREDITEM);
        _support_cm(CM_REQUESTLATESTCHATMESSAGE  );
        _support_cm(CM_REQUESTSPACEMOVE          );
        _support_cm(CM_SETMAGICKEY               );
        _support_cm(CM_SETRUNTIMECONFIG          );
        _support_cm(CM_CREATECHATGROUP           );
        default:
            {
                throw fflerror("unsupported client message: %s", ClientMsg(nType).name().c_str());
            }
#undef _support_cm
    }
}

bool Player::update()
{
    if(m_buffList.update()){
        dispatchBuffIDList();
    }
    return true;
}

void Player::reportCO(uint64_t toUID)
{
    if(!toUID){
        return;
    }

    AMCORecord amCOR;
    std::memset(&amCOR, 0, sizeof(amCOR));

    amCOR.UID = UID();
    amCOR.mapID = mapID();
    amCOR.action = makeActionStand();
    amCOR.Player.gender = gender();
    amCOR.Player.job = job();
    amCOR.Player.Level = level();
    m_actorPod->forward(toUID, {AM_CORECORD, amCOR});
}

void Player::reportStand()
{
    reportAction(UID(), mapID(), makeActionStand());
}

void Player::reportAction(uint64_t uid, uint32_t actionMapID, const ActionNode &action)
{
    fflassert(uid);

    SMAction smA;
    std::memset(&smA, 0, sizeof(smA));

    // player can forward CO's action that not on same map
    // this is used for CO map switch, client use it to remove left neighbors

    smA.UID = uid;
    smA.mapID = actionMapID;
    smA.action = action;

    postNetMessage(SM_ACTION, smA);
}

void Player::reportDeadUID(uint64_t nDeadUID)
{
    SMNotifyDead smND;
    std::memset(&smND, 0, sizeof(smND));

    smND.UID = nDeadUID;
    postNetMessage(SM_NOTIFYDEAD, smND);
}

void Player::reportHealth()
{
    dispatchNetPackage(true, SM_HEALTH, cerealf::serialize(m_sdHealth));
}

void Player::reportNextStrike()
{
    postNetMessage(SM_NEXTSTRIKE);
}

bool Player::goDie()
{
    if(m_dead.get()){
        return true;
    }
    m_dead.set(true);

    addDelay(2 * 1000, [this](){ goGhost(); });
    return true;
}

bool Player::goGhost()
{
    if(!m_dead.get()){
        return false;
    }

    AMDeadFadeOut amDFO;
    std::memset(&amDFO, 0, sizeof(amDFO));

    amDFO.UID   = UID();
    amDFO.mapID = mapID();
    amDFO.X     = X();
    amDFO.Y     = Y();

    if(true
            && hasActorPod()
            && m_map
            && m_map->hasActorPod()){
        m_actorPod->forward(m_map->UID(), {AM_DEADFADEOUT, amDFO});
    }

    deactivate();
    return true;
}

bool Player::dcValid(int, bool)
{
    return true;
}

DamageNode Player::getAttackDamage(int nDC, int) const
{
    const auto node = getCombatNode(m_sdItemStorage.wear, {}, UID(), level());
    const double elemRatio = 1.0 + 0.1 * [nDC, &node]() -> int
    {
        const auto &mr = DBCOM_MAGICRECORD(nDC);
        fflassert(mr);

        switch(magicElemID(mr.elem)){
            case MET_FIRE   : return node.dcElem.fire;
            case MET_ICE    : return node.dcElem.ice;
            case MET_LIGHT  : return node.dcElem.light;
            case MET_WIND   : return node.dcElem.wind;
            case MET_HOLY   : return node.dcElem.holy;
            case MET_DARK   : return node.dcElem.dark;
            case MET_PHANTOM: return node.dcElem.phantom;
            default         : return 0;
        }
    }();

    switch(nDC){
            case DBCOM_MAGICID(u8"烈火剑法"):
            case DBCOM_MAGICID(u8"翔空剑法"):
            case DBCOM_MAGICID(u8"莲月剑法"):
            case DBCOM_MAGICID(u8"半月弯刀"):
            case DBCOM_MAGICID(u8"十方斩"  ):
            case DBCOM_MAGICID(u8"攻杀剑术"):
            case DBCOM_MAGICID(u8"刺杀剑术"):
            case DBCOM_MAGICID(u8"物理攻击"):
            {
                return PlainPhyDamage
                {
                    .damage = node.randPickDC(),
                    .dcHit = node.dcHit,
                };
            }
        case DBCOM_MAGICID(u8"灵魂火符"):
        case DBCOM_MAGICID(u8"冰月神掌"):
        case DBCOM_MAGICID(u8"冰月震天"):
            {
                return MagicDamage
                {
                    .magicID = nDC,
                    .damage = to_d(std::lround(node.randPickSC() * elemRatio)),
                    .mcHit = node.mcHit,
                };
            }
        case DBCOM_MAGICID(u8"雷电术"):
        case DBCOM_MAGICID(u8"火球术"):
        case DBCOM_MAGICID(u8"大火球"):
        case DBCOM_MAGICID(u8"疾光电影"):
        case DBCOM_MAGICID(u8"地狱火"):
        case DBCOM_MAGICID(u8"冰沙掌"):
            {
                return MagicDamage
                {
                    .magicID = nDC,
                    .damage = to_d(std::lround(node.randPickMC() * elemRatio)),
                    .mcHit = node.mcHit,
                };
            }
        default:
            {
                return {};
            }
    }
}

bool Player::struckDamage(uint64_t, const DamageNode &node)
{
    // hack for debug
    // make the player never die
    return true;

    if(node){
        const auto damage = [&node, this]() -> int
        {
            const auto combatNode = getCombatNode(m_sdItemStorage.wear, {}, UID(), level());
            if(DBCOM_MAGICID(u8"物理攻击") == to_u32(node.magicID)){
                return std::max<int>(0, node.damage - mathf::rand<int>(combatNode.ac[0], combatNode.ac[1]));
            }

            const double elemRatio = std::max<double>(0.0, 1.0 + 0.1 * [&node, &combatNode, this]() -> int
            {
                const auto &mr = DBCOM_MAGICRECORD(node.magicID);
                fflassert(mr);

                switch(magicElemID(mr.elem)){
                    case MET_FIRE   : return combatNode.acElem.fire;
                    case MET_ICE    : return combatNode.acElem.ice;
                    case MET_LIGHT  : return combatNode.acElem.light;
                    case MET_WIND   : return combatNode.acElem.wind;
                    case MET_HOLY   : return combatNode.acElem.holy;
                    case MET_DARK   : return combatNode.acElem.dark;
                    case MET_PHANTOM: return combatNode.acElem.phantom;
                    default         : return 0;
                }
            }());
            return std::max<int>(0, node.damage - std::lround(mathf::rand<int>(combatNode.mac[0], combatNode.mac[1]) * elemRatio));
        }();

        if(damage > 0){
            updateHealth(-damage);
            if(m_sdHealth.hp <= 0){
                goDie();
            }
        }
        return true;
    }
    return false;
}

bool Player::ActionValid(const ActionNode &)
{
    return true;
}

void Player::dispatchOffline()
{
    if(true
            && hasActorPod()
            && m_map
            && m_map->hasActorPod()){

        AMOffline amO;
        std::memset(&amO, 0, sizeof(amO));

        amO.UID   = UID();
        amO.mapID = mapID();
        amO.X     = X();
        amO.Y     = Y();

        m_actorPod->forward(m_map->UID(), {AM_OFFLINE, amO});
        return;
    }

    g_monoServer->addLog(LOGTYPE_WARNING, "Can't dispatch offline event");
}

void Player::reportOffline(uint64_t nUID, uint32_t nMapID)
{
    fflassert(nUID);
    fflassert(nMapID);

    // player can initiatively start the offline procedure
    // in this case the m_channID still contains a good channel id, we need to call close

    if(m_channID.has_value() && m_channID.value()){
        SMOffline smO;
        std::memset(&smO, 0, sizeof(smO));

        smO.UID = nUID;
        smO.mapID = nMapID;
        postNetMessage(SM_OFFLINE, smO);

        // player initiatively close the channel
        // the NetDriver::close() only *request* the channel to be closed, it schedule an event
        // after this line the channel slot may still be non-empty, but we shall not post any network message
        // so use m_channID = 0 as a flag, please check comments for Player::on_AM_BADCHANNEL()

        g_netDriver->close(m_channID.value());
        m_channID = 0;
    }
}

bool Player::goOffline()
{
    dispatchOffline();
    reportOffline(UID(), mapID());
    dbUpdateMapGLoc();

    deactivate();
    return true;
}

void Player::postNetMessage(uint8_t headCode, const void *buf, size_t bufLen, uint64_t respID)
{
    if(m_channID.has_value() && m_channID.value()){
        g_netDriver->post(m_channID.value(), headCode, (const uint8_t *)(buf), bufLen, respID);
    }
    else{
        goOffline();
    }
}

void Player::onCMActionStand(CMAction stCMA)
{
    int nX = stCMA.action.x;
    int nY = stCMA.action.y;
    int nDirection = stCMA.action.direction;

    if(true
            && m_map
            && m_map->validC(nX, nY)){

        // server get report stand
        // means client is trying to re-sync
        // try client's current location and always response

        switch(estimateHop(nX, nY)){
            case 1:
                {
                    requestMove(nX, nY, SYS_MAXSPEED, false, false,
                    [this, stCMA]()
                    {
                        onCMActionStand(stCMA);
                    },
                    [this]()
                    {
                        reportStand();
                    });
                    return;
                }
            case 0:
            default:
                {
                    if(pathf::dirValid(nDirection)){
                        m_direction = nDirection;
                    }

                    reportStand();
                    return;
                }
        }
    }
}

void Player::onCMActionMove(CMAction stCMA)
{
    // server won't do any path finding
    // client should sent action with only one-hop movement

    int nX0 = stCMA.action.x;
    int nY0 = stCMA.action.y;
    int nX1 = stCMA.action.aimX;
    int nY1 = stCMA.action.aimY;

    switch(estimateHop(nX0, nY0)){
        case 0:
            {
                requestMove(nX1, nY1, moveSpeed(), false, false, [this]()
                {
                    dbUpdateMapGLoc();
                },

                [this]()
                {
                    reportStand();
                });
                return;
            }
        case 1:
            {
                requestMove(nX0, nY0, SYS_MAXSPEED, false, false, [this, stCMA]()
                {
                    onCMActionMove(stCMA);
                },
                [this]()
                {
                    reportStand();
                });
                return;
            }
        default:
            {
                reportStand();
                return;
            }
    }
}

void Player::onCMActionMine(CMAction stCMA)
{
    // server won't do any path finding
    // client should sent action with only one-hop movement

    const ActionMine mine = stCMA.action;
    switch(mathf::LDistance2(mine.x, mine.y, X(), Y())){
        case 0:
            {
                return;
            }
        case 1:
        case 2:
            {
                if(DBCOM_ITEMRECORD(m_sdItemStorage.wear.getWLItem(WLG_WEAPON)).equip.weapon.mine){
                    dispatchAction(mine);
                    addInventoryItem(SDItem
                    {
                        .itemID = DBCOM_ITEMID(u8"黑铁"),
                        .seqID  = 1,
                        .count  = 1,
                    }, false);
                }
                return;
            }
        default:
            {
                return;
            }
    }
}

void Player::onCMActionAttack(CMAction stCMA)
{
    getCOLocation(stCMA.action.aimUID, [this, stCMA](const COLocation &rstLocation)
    {
        int nX0 = stCMA.action.x;
        int nY0 = stCMA.action.y;

        int nDCType = stCMA.action.extParam.attack.magicID;
        uint64_t nAimUID = stCMA.action.aimUID;

        if(rstLocation.mapID != mapID()){
            return;
        }

        switch(nDCType){
            case DBCOM_MAGICID(u8"烈火剑法"):
            case DBCOM_MAGICID(u8"翔空剑法"):
            case DBCOM_MAGICID(u8"莲月剑法"):
            case DBCOM_MAGICID(u8"半月弯刀"):
            case DBCOM_MAGICID(u8"十方斩"  ):
            case DBCOM_MAGICID(u8"攻杀剑术"):
            case DBCOM_MAGICID(u8"刺杀剑术"):
            case DBCOM_MAGICID(u8"物理攻击"):
                {
                    switch(estimateHop(nX0, nY0)){
                        case 0:
                            {
                                if(const auto aimDir = pathf::getOffDir(X(), Y(), rstLocation.x, rstLocation.y); pathf::dirValid(aimDir)){
                                    m_direction = aimDir;
                                    dispatchAction(makeActionStand());

                                    // don't need to send direction change back to client
                                    // it has already turned
                                }

                                switch(mathf::LDistance2(nX0, nY0, rstLocation.x, rstLocation.y)){
                                    case 1:
                                    case 2:
                                        {
                                            const auto [buffID, modifierID] = m_buffList.rollAttackModifier();

                                            // client reports 攻杀技术 but server need to validate if it's scheduled
                                            // if not scheduled then dispatch 物理攻击 instead, this is for client anti-cheat
                                            dispatchAction(ActionAttack
                                            {
                                                .speed = stCMA.action.speed,
                                                .x = stCMA.action.x,
                                                .y = stCMA.action.y,
                                                .aimUID = stCMA.action.aimUID,
                                                .extParam
                                                {
                                                    .magicID = [nDCType, this]() -> uint32_t
                                                    {
                                                        if(to_u32(nDCType) == DBCOM_MAGICID(u8"攻杀剑术") && !m_nextStrike){
                                                            return DBCOM_MAGICID(u8"物理攻击");
                                                        }
                                                        else{
                                                            return nDCType;
                                                        }
                                                    }(),
                                                    .modifierID = to_u32(modifierID),
                                                },
                                            });

                                            std::vector<uint64_t> aimUIDList;
                                            switch(nDCType){
                                                case DBCOM_MAGICID(u8"莲月剑法"):
                                                    {
                                                        aimUIDList.push_back(nAimUID);
                                                        aimUIDList.push_back(nAimUID); // attack twice
                                                        break;
                                                    }
                                                case DBCOM_MAGICID(u8"半月弯刀"):
                                                    {
                                                        scoped_alloc::svobuf_wrapper<std::tuple<int, int>, 3> aimGridList;
                                                        for(int d: {-1, 0, 1}){
                                                            aimGridList.c.push_back(pathf::getFrontGLoc(X(), Y(), pathf::getNextDir(Direction(), d)));
                                                        }

                                                        for(const auto &[uid, coLoc]: m_inViewCOList){
                                                            if(std::find(aimGridList.c.begin(), aimGridList.c.end(), std::make_tuple(coLoc.x, coLoc.y)) != aimGridList.c.end()){
                                                                aimUIDList.push_back(uid);
                                                            }
                                                        }
                                                        break;
                                                    }
                                                case DBCOM_MAGICID(u8"十方斩"):
                                                    {
                                                        for(const auto &[uid, coLoc]: m_inViewCOList){
                                                            if(mathf::CDistance<int>(X(), Y(), coLoc.x, coLoc.y) <= 1){
                                                                aimUIDList.push_back(uid);
                                                            }
                                                        }
                                                        break;
                                                    }
                                                case DBCOM_MAGICID(u8"刺杀剑术"):
                                                    {
                                                        std::array<std::tuple<int, int>, 2> aimGridList
                                                        {
                                                            pathf::getFrontGLoc(X(), Y(), Direction(), 1),
                                                            pathf::getFrontGLoc(X(), Y(), Direction(), 2),
                                                        };

                                                        for(const auto &[uid, coLoc]: m_inViewCOList){
                                                            if(std::find(aimGridList.begin(), aimGridList.end(), std::make_tuple(coLoc.x, coLoc.y)) != aimGridList.end()){
                                                                aimUIDList.push_back(uid);
                                                            }
                                                        }
                                                        break;
                                                    }
                                                case DBCOM_MAGICID(u8"翔空剑法"):
                                                case DBCOM_MAGICID(u8"攻杀剑术"):
                                                case DBCOM_MAGICID(u8"烈火剑法"):
                                                case DBCOM_MAGICID(u8"物理攻击"):
                                                default:
                                                    {
                                                        aimUIDList.push_back(nAimUID);
                                                        break;
                                                    }
                                            }

                                            for(const auto uid: aimUIDList){
                                                if(buffID){
                                                    sendBuff(uid, 0, buffID);
                                                }
                                                dispatchAttackDamage(uid, nDCType, 0);
                                            }

                                            if(m_nextStrike){
                                                m_nextStrike = false;
                                            }
                                            else{
                                                m_nextStrike = (mathf::rand<int>(0, 2) == 0);
                                            }

                                            if(m_nextStrike){
                                                reportNextStrike();
                                            }
                                            return;
                                        }
                                    default:
                                        {
                                            return;
                                        }
                                }
                                return;
                            }
                        case 1:
                            {
                                requestMove(nX0, nY0, SYS_MAXSPEED, false, false,
                                [this, stCMA]()
                                {
                                    onCMActionAttack(stCMA);
                                },
                                [this]()
                                {
                                    reportStand();
                                });
                                return;
                            }
                        default:
                            {
                                return;
                            }
                    }
                    return;
                }
            default:
                {
                    return;
                }
        }
    });
}

void Player::onCMActionSpinKick(CMAction cmA)
{
    fflassert(cmA.action.type == ACTION_SPINKICK);
    dispatchAction(cmA.action);
}

void Player::onCMActionPickUp(CMAction cmA)
{
    fflassert(cmA.action.type == ACTION_PICKUP);
    dispatchAction(cmA.action);
}

void Player::onCMActionSpell(CMAction cmA)
{
    fflassert(cmA.action.type == ACTION_SPELL);
    const auto magicID = cmA.action.extParam.spell.magicID;

    dispatchAction(cmA.action);
    const auto node = getCombatNode(m_sdItemStorage.wear, m_sdLearnedMagicList, UID(), level());

    switch(magicID){
        case DBCOM_MAGICID(u8"治愈术"):
        case DBCOM_MAGICID(u8"施毒术"):
        case DBCOM_MAGICID(u8"幽灵盾"):
        case DBCOM_MAGICID(u8"神圣战甲术"):
            {
                const auto buffID = DBCOM_BUFFID(DBCOM_MAGICRECORD(magicID).name);
                const auto &br = DBCOM_BUFFRECORD(buffID);

                fflassert(buffID);
                fflassert(br);

                if(cmA.action.aimUID){
                    switch(uidf::getUIDType(cmA.action.aimUID)){
                        case UID_MON:
                        case UID_PLY:
                            {
                                if(br.favor == 0){
                                    sendBuff(cmA.action.aimUID, 0, buffID);
                                }
                                else{
                                    checkFriend(cmA.action.aimUID, [magicID, cmA, buffID, this](int friendType)
                                    {
                                        const auto &br = DBCOM_BUFFRECORD(buffID);
                                        fflassert(br);

                                        switch(friendType){
                                            case FT_FRIEND:
                                                {
                                                    if(br.favor >= 0){
                                                        sendBuff(cmA.action.aimUID, 0, buffID);
                                                    }
                                                    return;
                                                }
                                            case FT_ENEMY:
                                                {
                                                    if(br.favor <= 0){
                                                        sendBuff(cmA.action.aimUID, 0, buffID);
                                                    }
                                                    return;
                                                }
                                            case FT_NEUTRAL:
                                                {
                                                    sendBuff(cmA.action.aimUID, 0, buffID);
                                                    return;
                                                }
                                            default:
                                                {
                                                    return;
                                                }
                                        }
                                    });
                                }
                                break;
                            }
                        default:
                            {
                                break;
                            }
                    }
                }
                else if(br.favor >= 0){
                    addBuff(UID(), 0, buffID);
                }
                break;
            }
        case DBCOM_MAGICID(u8"火球术"):
        case DBCOM_MAGICID(u8"大火球"):
        case DBCOM_MAGICID(u8"灵魂火符"):
        case DBCOM_MAGICID(u8"冰月神掌"):
        case DBCOM_MAGICID(u8"冰月震天"):
            {
                // 灵魂火符 doesn't need to send back the CASTMAGIC message
                // the ACTION_SPELL creates the magic

                if(cmA.action.aimUID){
                    getCOLocation(cmA.action.aimUID, [this, cmA](const COLocation &coLoc)
                    {
                        const auto ld = mathf::LDistance<float>(coLoc.x, coLoc.y, cmA.action.x, cmA.action.y);
                        const auto delay = ld * 100;

                        addDelay(delay, [cmA, this]()
                        {
                            dispatchAttackDamage(cmA.action.aimUID, cmA.action.extParam.spell.magicID, 0);
                        });
                    });
                }
                break;
            }
        case DBCOM_MAGICID(u8"雷电术"):
            {
                SMCastMagic smFM;
                std::memset(&smFM, 0, sizeof(smFM));

                smFM.UID    = UID();
                smFM.mapID  = mapID();
                smFM.Magic  = magicID;
                smFM.Speed  = MagicSpeed();
                smFM.X      = cmA.action.x;
                smFM.Y      = cmA.action.y;
                smFM.AimUID = cmA.action.aimUID;

                addDelay(1400, [this, smFM]()
                {
                    dispatchNetPackage(true, SM_CASTMAGIC, smFM);
                    addDelay(300, [smFM, this]()
                    {
                        dispatchAttackDamage(smFM.AimUID, DBCOM_MAGICID(u8"雷电术"), 0);
                    });
                });
                break;
            }
        case DBCOM_MAGICID(u8"魔法盾"):
        case DBCOM_MAGICID(u8"阴阳法环"):
            {
                SMCastMagic smFM;
                std::memset(&smFM, 0, sizeof(smFM));

                smFM.UID   = UID();
                smFM.Magic = magicID;
                smFM.Speed = MagicSpeed();

                addDelay(800, [this, smFM]()
                {
                    dispatchNetPackage(true, SM_CASTMAGIC, smFM);
                    addDelay(10000, [this]()
                    {
                        SMBuff smB;
                        std::memset(&smB, 0, sizeof(smB));

                        smB.uid   = UID();
                        smB.type  = BFT_SHIELD;
                        smB.state = BFS_OFF;
                        dispatchNetPackage(true, SM_BUFF, smB);
                    });
                });
                break;
            }
        case DBCOM_MAGICID(u8"召唤骷髅"):
        case DBCOM_MAGICID(u8"超强召唤骷髅"):
            {
                const auto [nFrontX, nFrontY] = pathf::getFrontGLoc(X(), Y(), Direction(), 2);

                SMCastMagic smFM;
                std::memset(&smFM, 0, sizeof(smFM));

                smFM.UID   = UID();
                smFM.mapID = mapID();
                smFM.Magic = magicID;
                smFM.Speed = MagicSpeed();
                smFM.AimX  = nFrontX;
                smFM.AimY  = nFrontY;

                addDelay(600, [this, magicID, smFM]()
                {
                    for(int i = 0; i < g_serverArgParser->summonCount; ++i){
                        if(to_u32(magicID) == DBCOM_MAGICID(u8"召唤骷髅")){
                            addMonster(DBCOM_MONSTERID(u8"变异骷髅"), smFM.AimX, smFM.AimY, false);
                        }
                        else{
                            addMonster(DBCOM_MONSTERID(u8"超强骷髅"), smFM.AimX, smFM.AimY, false);
                        }
                    }

                    // addMonster will send ACTION_SPAWN to client
                    // client then use it to play the magic for 召唤骷髅, we don't send magic message here
                });
                break;
            }
        case DBCOM_MAGICID(u8"召唤神兽"):
            {
                const auto [nFrontX, nFrontY] = pathf::getFrontGLoc(X(), Y(), Direction(), 2);

                SMCastMagic smFM;
                std::memset(&smFM, 0, sizeof(smFM));

                smFM.UID   = UID();
                smFM.mapID = mapID();
                smFM.Magic = magicID;
                smFM.Speed = MagicSpeed();
                smFM.AimX  = nFrontX;
                smFM.AimY  = nFrontY;

                addDelay(1000, [this, smFM]()
                {
                    for(int i = 0; i < g_serverArgParser->summonCount; ++i){
                        addMonster(DBCOM_MONSTERID(u8"神兽"), smFM.AimX, smFM.AimY, false);
                    }
                });
                break;
            }
        case DBCOM_MAGICID(u8"火墙"):
            {
                addDelay(550, [this, cmA, node]()
                {
                    AMCastFireWall amCFW;
                    std::memset(&amCFW, 0, sizeof(amCFW));

                    amCFW.minDC = node.mc[0];
                    amCFW.maxDC = node.mc[1];
                    amCFW.mcHit = node.mcHit;

                    amCFW.duration = 20 * 1000;
                    amCFW.dps      = 2;

                    // not 3x3
                    // fire wall takes grids as a cross
                    //
                    // +---+---+---+
                    // |   | v |   |
                    // +---+---+---+
                    // | v | v | v |
                    // +---+---+---+
                    // |   | v |   |
                    // +---+---+---+

                    for(const int dir: {DIR_NONE, DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT}){
                        if(dir == DIR_NONE){
                            amCFW.x = cmA.action.aimX;
                            amCFW.y = cmA.action.aimY;
                        }
                        else{
                            std::tie(amCFW.x, amCFW.y) = pathf::getFrontGLoc(cmA.action.aimX, cmA.action.aimY, dir, 1);
                        }

                        if(m_map->groundValid(amCFW.x, amCFW.y)){
                            m_actorPod->forward(m_map->UID(), {AM_CASTFIREWALL, amCFW});
                        }
                    }
                });
                break;
            }
        case DBCOM_MAGICID(u8"地狱火"):
        case DBCOM_MAGICID(u8"冰沙掌"):
        case DBCOM_MAGICID(u8"疾光电影"):
            {
                if(const auto dirIndex = pathf::getDir8(cmA.action.aimX - cmA.action.x, cmA.action.aimY - cmA.action.y); (dirIndex >= 0) && pathf::dirValid(dirIndex + DIR_BEGIN)){
                    m_direction = dirIndex + DIR_BEGIN;
                }

                std::set<std::tuple<int, int>> pathGridList;
                switch(Direction()){
                    case DIR_UP:
                    case DIR_DOWN:
                    case DIR_LEFT:
                    case DIR_RIGHT:
                        {
                            for(const auto distance: {1, 2, 3, 4, 5, 6, 7, 8}){
                                const auto [pathGX, pathGY] = pathf::getFrontGLoc(X(), Y(), Direction(), distance);
                                pathGridList.insert({pathGX, pathGY});

                                if(distance > 3){
                                    const auto [sgnDX, sgnDY] = pathf::getFrontGLoc(0, 0, Direction(), 1);
                                    pathGridList.insert({pathGX + sgnDY, pathGY + sgnDX}); // switch sgnDX and sgnDY and plus/minus
                                    pathGridList.insert({pathGX - sgnDY, pathGY - sgnDX});
                                }
                            }
                            break;
                        }
                    case DIR_UPLEFT:
                    case DIR_UPRIGHT:
                    case DIR_DOWNLEFT:
                    case DIR_DOWNRIGHT:
                        {
                            for(const auto distance: {1, 2, 3, 4, 5, 6, 7, 8}){
                                const auto [pathGX, pathGY] = pathf::getFrontGLoc(X(), Y(), Direction(), distance);
                                pathGridList.insert({pathGX, pathGY});

                                const auto [sgnDX, sgnDY] = pathf::getFrontGLoc(0, 0, Direction(), 1);
                                pathGridList.insert({pathGX + sgnDX, pathGY        });
                                pathGridList.insert({pathGX        , pathGY + sgnDY});
                            }
                            break;
                        }
                    default:
                        {
                            throw fflreach();
                        }
                }

                AMStrikeFixedLocDamage amSFLD;
                std::memset(&amSFLD, 0, sizeof(amSFLD));

                for(const auto &[pathGX, pathGY]: pathGridList){
                    if(m_map->groundValid(pathGX, pathGY)){
                        amSFLD.x = pathGX;
                        amSFLD.y = pathGY;
                        amSFLD.damage = getAttackDamage(magicID, 0);
                        addDelay(550 + mathf::CDistance(X(), Y(), amSFLD.x, amSFLD.y) * 100, [amSFLD, castMapID = mapID(), this]()
                        {
                            if(castMapID == mapID()){
                                m_actorPod->forward(m_map->UID(), {AM_STRIKEFIXEDLOCDAMAGE, amSFLD});
                                if(g_serverArgParser->showStrikeGrid){
                                    SMStrikeGrid smSG;
                                    std::memset(&smSG, 0, sizeof(smSG));

                                    smSG.x = amSFLD.x;
                                    smSG.y = amSFLD.y;
                                    dispatchNetPackage(true, SM_STRIKEGRID, smSG);
                                }
                            }
                        });
                    }
                }
                break;
            }
        default:
            {
                break;
            }
    }
}

void Player::gainExp(int addedExp)
{
    if(addedExp <= 0){
        return;
    }

    const auto oldLevel = level();
    const auto oldMaxHP = maxHP();
    const auto oldMaxMP = maxMP();

    m_exp += addedExp;
    m_luaRunner->spawn(m_threadKey++, str_printf("_RSVD_NAME_trigger(SYS_ON_GAINEXP, %d)", addedExp));

    const auto addedMaxHP = std::max<int>(maxHP() - oldMaxHP, 0);
    const auto addedMaxMP = std::max<int>(maxMP() - oldMaxMP, 0);

    dbUpdateExp();
    postExp();

    if(level() > oldLevel){
        m_luaRunner->spawn(m_threadKey++, str_printf("_RSVD_NAME_trigger(SYS_ON_LEVELUP, %d, %d)", to_d(oldLevel), to_d(level())));
    }

    if(addedMaxHP > 0 || addedMaxMP > 0){
        updateHealth(0, 0, addedMaxHP, addedMaxMP);
    }
}

bool Player::CanPickUp(uint32_t, uint32_t)
{
    return true;
}

void Player::reportGold()
{
    SMGold smG;
    std::memset(&smG, 0, sizeof(smG));
    smG.gold = gold();
    postNetMessage(SM_GOLD, smG);
}

void Player::reportRemoveItem(uint32_t itemID, uint32_t seqID, size_t count)
{
    SMRemoveItem smRI;
    std::memset(&smRI, 0, sizeof(smRI));

    smRI.itemID = itemID;
    smRI. seqID =  seqID;
    smRI. count =  count;
    postNetMessage(SM_REMOVEITEM, smRI);
}

void Player::reportSecuredItemList()
{
    postNetMessage(SM_SHOWSECUREDITEMLIST, cerealf::serialize(SDShowSecuredItemList
    {
        .itemList = dbLoadSecuredItemList(),
    }));
}

void Player::reportTeamMemberList()
{
    pullTeamMemberList([this](std::optional<SDTeamMemberList> sdTML)
    {
        if(sdTML.has_value()){
            postNetMessage(SM_TEAMMEMBERLIST, cerealf::serialize(sdTML.value()));
        }
    });
}

void Player::checkFriend(uint64_t targetUID, std::function<void(int)> fnOp)
{
    // this function means:
    // this player says: how I fell about targetUID

    fflassert(targetUID);
    fflassert(targetUID != UID());

    switch(uidf::getUIDType(targetUID)){
        case UID_NPC:
            {
                if(fnOp){
                    fnOp(FT_NEUTRAL);
                }
                return;
            }
        case UID_PLY:
            {
                if(fnOp){
                    fnOp(isOffender(targetUID) ? FT_ENEMY : FT_NEUTRAL);
                }
                return;
            }
        case UID_MON:
            {
                queryFinalMaster(targetUID, [this, targetUID, fnOp](uint64_t finalMasterUID)
                {
                    if(!finalMasterUID){
                        if(fnOp){
                            fnOp(FT_ERROR);
                        }
                        return;
                    }

                    switch(uidf::getUIDType(finalMasterUID)){
                        case UID_PLY:
                            {
                                if(fnOp){
                                    fnOp(isOffender(targetUID) ? FT_ENEMY : FT_NEUTRAL);
                                }
                                return;
                            }
                        case UID_MON:
                            {
                                if(fnOp){
                                    fnOp(FT_ENEMY);
                                }
                                return;
                            }
                        default:
                            {
                                throw fflvalue(uidf::getUIDString(finalMasterUID));
                            }
                    }
                });
                return;
            }
        default:
            {
                throw fflvalue(uidf::getUIDString(targetUID));
            }
    }
}

void Player::RequestKillPets()
{
    for(auto uid: m_slaveList){
        m_actorPod->forward(uid, {AM_MASTERKILL});
    }
    m_slaveList.clear();
}

void Player::postOnlineOK()
{
    SMOnlineOK smOOK;
    std::memset(&smOOK, 0, sizeof(smOOK));

    smOOK.uid = UID();
    smOOK.name.assign(m_name);
    smOOK.gender = gender();
    smOOK.job = job();
    smOOK.mapID = mapID();
    smOOK.action = makeActionStand();

    postNetMessage(SM_ONLINEOK, smOOK);
    postNetMessage(SM_STARTGAMESCENE, cerealf::serialize(SDStartGameScene
    {
        .uid = UID(),
        .mapID = mapID(),

        .x = X(),
        .y = Y(),
        .direction = Direction(),

        .desp
        {
            .wear = m_sdItemStorage.wear,
            .hair = m_hair,
            .hairColor = m_hairColor,
        },

        .name = m_name,
        .nameColor = m_nameColor,
    }));

    postExp();
    postNetMessage(SM_HEALTH,           cerealf::serialize(m_sdHealth));
    postNetMessage(SM_INVENTORY,        cerealf::serialize(m_sdItemStorage.inventory));
    postNetMessage(SM_BELT,             cerealf::serialize(m_sdItemStorage.belt));
    postNetMessage(SM_LEARNEDMAGICLIST, cerealf::serialize(m_sdLearnedMagicList));
    postNetMessage(SM_PLAYERCONFIG,     cerealf::serialize(m_sdPlayerConfig));
    postNetMessage(SM_FRIENDLIST,       cerealf::serialize(m_sdFriendList));

    std::vector<std::pair<bool, uint32_t>> friendIDList
    {
        {false, SYS_CHATDBID_SYSTEM},
        {false, dbid()             },
    };

    std::for_each(m_sdFriendList.begin(), m_sdFriendList.end(), [&friendIDList](const auto &peer)
    {
        friendIDList.push_back({peer.group(), peer.id});
    });

    if(!friendIDList.empty()){
        postNetMessage(SM_CHATMESSAGELIST, cerealf::serialize(dbRetrieveLatestChatMessage(friendIDList, 1, true, true)));
    }

    for(int wltype = WLG_BEGIN; wltype < WLG_END; ++wltype){
        if(const auto &item = m_sdItemStorage.wear.getWLItem(wltype)){
            if(const auto buffIDOpt = item.getExtAttr<SDItem::EA_BUFFID_t>(); buffIDOpt.has_value() && buffIDOpt.value()){
                if(const auto pbuff = addBuff(UID(), 0, buffIDOpt.value())){
                    addWLOffTrigger(wltype, [buffSeq = pbuff->buffSeq(), this]()
                    {
                        removeBuff(buffSeq, true);
                    });
                }
            }
        }
    }
}

bool Player::hasInventoryItem(uint32_t itemID, uint32_t seqID, size_t count) const
{
    return m_sdItemStorage.inventory.has(itemID, seqID) >= count;
}

const SDItem &Player::addInventoryItem(SDItem item, bool keepSeqID)
{
    const auto &addedItem = m_sdItemStorage.inventory.add(std::move(item), keepSeqID);
    dbUpdateInventoryItem(addedItem);

    m_luaRunner->spawn(m_threadKey++, str_printf("_RSVD_NAME_trigger(SYS_ON_GAINITEM, %llu)", to_llu(item.itemID)));

    postNetMessage(SM_UPDATEITEM, cerealf::serialize(SDUpdateItem
    {
        .item = addedItem,
    }));
    return addedItem;
}

size_t Player::removeInventoryItem(const SDItem &item)
{
    return removeInventoryItem(item.itemID, item.seqID);
}

size_t Player::removeInventoryItem(uint32_t itemID, uint32_t seqID)
{
    fflassert(DBCOM_ITEMRECORD(itemID));
    fflassert(seqID > 0);
    return removeInventoryItem(itemID, seqID, SIZE_MAX);
}

size_t Player::removeInventoryItem(uint32_t itemID, uint32_t seqID, size_t count)
{
    const auto &ir = DBCOM_ITEMRECORD(itemID);

    fflassert(ir);
    fflassert(count > 0);
    fflassert(!ir.isGold());

    size_t doneCount = 0;
    while(doneCount < count){
        const auto [removedCount, removedSeqID, itemPtr] = m_sdItemStorage.inventory.remove(itemID, seqID, count - doneCount);
        if(!removedCount){
            break;
        }

        if(itemPtr){
            dbUpdateInventoryItem(*itemPtr);
        }
        else{
            dbRemoveInventoryItem(itemID, removedSeqID);
        }

        doneCount += removedCount;
        reportRemoveItem(itemID, removedSeqID, removedCount);
    }
    return doneCount;
}

const SDItem &Player::findInventoryItem(uint32_t itemID, uint32_t seqID) const
{
    fflassert(DBCOM_ITEMRECORD(itemID));
    return m_sdItemStorage.inventory.find(itemID, seqID);
}

void Player::secureItem(uint32_t itemID, uint32_t seqID)
{
    fflassert(findInventoryItem(itemID, seqID));
    dbSecureItem(itemID, seqID);
    removeInventoryItem(itemID, seqID);
}

void Player::removeSecuredItem(uint32_t itemID, uint32_t seqID)
{
    addInventoryItem(dbRetrieveSecuredItem(itemID, seqID), false);

    SMRemoveSecuredItem smRSI;
    std::memset(&smRSI, 0, sizeof(smRSI));

    smRSI.itemID = itemID;
    smRSI. seqID =  seqID;
    postNetMessage(SM_REMOVESECUREDITEM, smRSI);
}

void Player::setGold(size_t gold)
{
    m_sdItemStorage.gold = gold;
    g_dbPod->exec("update tbl_char set fld_gold = %llu where fld_dbid = %llu", to_llu(m_sdItemStorage.gold), to_llu(dbid()));
    reportGold();
}

bool Player::updateHealth(int addHP, int addMP, int addMaxHP, int addMaxMP)
{
    if(BattleObject::updateHealth(addHP, addMP, addMaxHP, addMaxMP)){
        dbUpdateHealth();
        postNetMessage(SM_HEALTH, cerealf::serialize(m_sdHealth));
        return true;
    }
    return false;
}

void Player::setWLItem(int wltype, SDItem item)
{
    if(!(wltype >= WLG_BEGIN && wltype < WLG_END)){
        throw fflerror("bad wltype: %d", wltype);
    }

    m_sdItemStorage.wear.setWLItem(wltype, item);
    const auto sdEquipWearBuf = cerealf::serialize(SDEquipWear
    {
        .uid = UID(),
        .wltype = wltype,
        .item = item,
    });

    foreachInViewCO([sdEquipWearBuf, this](const COLocation &coLoc)
    {
        if(uidf::getUIDType(coLoc.uid) == UID_PLY){
            forwardNetPackage(coLoc.uid, SM_EQUIPWEAR, sdEquipWearBuf);
        }
    });
}

void Player::postExp()
{
    SMExp smE;
    std::memset(&smE, 0, sizeof(smE));
    smE.exp = exp();
    postNetMessage(SM_EXP, smE);
}

bool Player::canWear(uint32_t itemID, int wltype) const
{
    fflassert(itemID, itemID);
    fflassert(wltype >= WLG_BEGIN, wltype);
    fflassert(wltype <  WLG_END  , wltype);

    const auto &ir = DBCOM_ITEMRECORD(itemID);
    if(!ir){
        return false;
    }

    if(!ir.wearable(wltype)){
        return false;
    }

    if(wltype == WLG_DRESS && (!ir.clothGender().has_value() || ir.clothGender().value() != gender())){
        return false;
    }

    // TODO
    // check item requirement

    return true;
}

std::vector<std::string> Player::parseRemoteCall(const char *query)
{
    fflassert(str_haschar(query));

    const char *beginPtr = query;
    const char *endPtr   = query + std::strlen(query);

    std::vector<std::string> result;
    while(true){
        beginPtr = std::find_if_not(beginPtr, endPtr, [](char chByte)
        {
            return chByte == ' ';
        });

        if(beginPtr == endPtr){
            break;
        }

        const char *donePtr = std::find(beginPtr, endPtr, ' ');
        result.emplace_back(beginPtr, donePtr);
        beginPtr = donePtr;
    }
    return result;
}

void Player::notifySlaveGLoc()
{
    for(const auto uid: m_slaveList){
        dispatchAction(uid, makeActionStand());
    }
}

int Player::maxHP() const
{
    const int maxHPTaoist  = 100 + level() *  50;
    const int maxHPWarrior = 300 + level() * 100;
    const int maxHPWizard  =  50 + level() *  20;

    int result = 0;
    if(job() & JOB_WARRIOR) result = std::max<int>(result, maxHPWarrior);
    if(job() & JOB_TAOIST ) result = std::max<int>(result, maxHPTaoist );
    if(job() & JOB_WIZARD ) result = std::max<int>(result, maxHPWizard );
    return result;
}

int Player::maxMP() const
{
    const int maxMPTaoist  = 200 + level() *  50;
    const int maxMPWarrior = 100 + level() *  10;
    const int maxMPWizard  = 500 + level() * 200;

    int result = 0;
    if(job() & JOB_WARRIOR) result = std::max<int>(result, maxMPWarrior);
    if(job() & JOB_TAOIST ) result = std::max<int>(result, maxMPTaoist );
    if(job() & JOB_WIZARD ) result = std::max<int>(result, maxMPWizard );
    return result;
}

bool Player::consumeBook(uint32_t itemID)
{
    const auto &ir = DBCOM_ITEMRECORD(itemID);
    fflassert(ir);
    fflassert(ir.isBook());

    const auto magicID = DBCOM_MAGICID(ir.name);
    const auto &mr = DBCOM_MAGICRECORD(magicID);

    fflassert(magicID);
    fflassert(mr);

    if(m_sdLearnedMagicList.has(magicID)){
        postNetMessage(SM_TEXT, str_printf(u8"无法学习%s，因为你已掌握此技能", to_cstr(mr.name)));
        return false;
    }

    if(!g_serverArgParser->disableLearnMagicCheckJob){
        bool hasJob = false;
        for(const auto jobstr: jobf::jobName(job())){
            if(jobstr){
                if(to_u8sv(jobstr) == mr.req.job){
                    hasJob = true;
                }
            }
            else{
                break;
            }
        }

        if(!hasJob){
            postNetMessage(SM_TEXT, str_printf(u8"无法学习%s，因为此项技能需要职业为%s", to_cstr(mr.name), to_cstr(mr.req.job)));
            return false;
        }
    }

    if(to_d(level()) < mr.req.level[0] && !g_serverArgParser->disableLearnMagicCheckLevel){
        postNetMessage(SM_TEXT, str_printf(u8"无法学习%s，因为你尚未到达%d级", to_cstr(mr.name), mr.req.level[0]));
        return false;
    }

    if(str_haschar(mr.req.prior) && !g_serverArgParser->disableLearnMagicCheckPrior){
        const auto priorMagicID = DBCOM_MAGICID(mr.req.prior);
        const auto &priorMR = DBCOM_MAGICRECORD(priorMagicID);

        fflassert(priorMagicID);
        fflassert(priorMR);

        if(!m_sdLearnedMagicList.has(priorMagicID)){
            postNetMessage(SM_TEXT, str_printf(u8"无法学习%s，因为你尚未学习前置魔法%s", to_cstr(mr.name), to_cstr(priorMR.name)));
            return false;
        }
    }

    m_sdLearnedMagicList.magicList.push_back(SDLearnedMagic
    {
        .magicID = magicID,
    });

    dbLearnMagic(magicID);
    postNetMessage(SM_TEXT, str_printf(u8"恭喜掌握%s", to_cstr(mr.name)));
    postNetMessage(SM_LEARNEDMAGICLIST, cerealf::serialize(m_sdLearnedMagicList));
    return true;
}

bool Player::consumePotion(uint32_t itemID)
{
    const auto &ir = DBCOM_ITEMRECORD(itemID);
    fflassert(ir);
    fflassert(ir.isPotion());

    if(addBuff(UID(), 0, DBCOM_BUFFID(ir.name))){
        return true;
    }
    return false;
}

void Player::pullTeamMemberList(std::function<void(std::optional<SDTeamMemberList>)> fnHandle)
{
    if(!fnHandle){
        return;
    }

    if(!m_teamLeader){
        fnHandle(SDTeamMemberList{});
        return;
    }

    if(m_teamLeader != UID()){
        m_actorPod->forward(m_teamLeader, AM_QUERYTEAMMEMBERLIST, [fnHandle, this](const ActorMsgPack &mpk)
        {
            switch(mpk.type()){
                case AM_TEAMMEMBERLIST:
                    {
                        const auto sdTML = mpk.deserialize<SDTeamMemberList>();
                        fflassert(sdTML.hasMember(UID())); // keep this function read only
                        fnHandle(sdTML);
                        break;
                    }
                default:
                    {
                        fnHandle({});
                        break;
                    }
            }
        });
        return;
    }

    auto cnter = std::make_shared<size_t>(0);
    auto sdTML = std::make_shared<SDTeamMemberList>();

    sdTML->teamLeader = m_teamLeader;
    sdTML->memberList.resize(m_teamMemberList.size());

    for(size_t i = 0; i < m_teamMemberList.size(); ++i){
        if(m_teamMemberList.at(i) == UID()){
            sdTML->memberList[i] = SDTeamPlayer
            {
                .uid = UID(),
                .level = level(),
                .name = name(),
            };

            if(++(*cnter) == m_teamMemberList.size()){
                fnHandle(*sdTML);
            }
        }
        else{
            m_actorPod->forward(m_teamMemberList.at(i), AM_QUERYTEAMPLAYER, [i, cnter, sdTML, memberCount = m_teamMemberList.size(), fnHandle, this](const ActorMsgPack &mpk)
            {
                switch(mpk.type()){
                    case AM_TEAMPLAYER:
                        {
                            sdTML->memberList.at(i) = mpk.deserialize<SDTeamPlayer>();
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }

                if(++(*cnter) == memberCount){
                    fnHandle(*sdTML);
                }
            });
        }
    }
}
