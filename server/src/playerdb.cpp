#include <cstring>
#include "luaf.hpp"
#include "dbpod.hpp"
#include "player.hpp"
#include "monoserver.hpp"

extern DBPod *g_dbPod;
extern MonoServer *g_monoServer;

luaf::luaVar Player::dbGetVar(const std::string &var)
{
    fflassert(str_haschar(var));
    auto query = g_dbPod->createQuery("select fld_value from tbl_charvarlist where fld_dbid = %llu and fld_var = '%s' and fld_value is not null", to_llu(dbid()), var.c_str());

    if(query.executeStep()){
        return cerealf::deserialize<luaf::luaVar>(query.getColumn(0).getString());
    }
    else{
        return luaf::luaNil{}; // TODO throw or return nil ?
    }
}

void Player::dbSetVar(const std::string &var, luaf::luaVar value)
{
    fflassert(str_haschar(var));
    if(std::get_if<luaf::luaNil>(&value)){
        g_dbPod->exec("delete from tbl_charvarlist where fld_dbid = %llu and fld_var = '%s'", to_llu(dbid()), var.c_str());
    }
    else{
        auto query = g_dbPod->createQuery(
            u8R"###( insert into tbl_charvarlist(fld_dbid, fld_var, fld_value) )###"
            u8R"###( values                                                    )###"
            u8R"###(     (%llu, '%s', ?)                                       )###"
            u8R"###(                                                           )###"
            u8R"###( on conflict(fld_dbid, fld_var) do                         )###"
            u8R"###( update set                                                )###"
            u8R"###(                                                           )###"
            u8R"###(     fld_value=excluded.fld_value                          )###",

            to_llu(dbid()),
            var.c_str());

        query.bind(1, cerealf::serialize(value));
        query.exec();
    }
}

std::pair<bool, luaf::luaVar> Player::dbHasVar(const std::string &var)
{
    fflassert(str_haschar(var));
    auto query = g_dbPod->createQuery("select fld_value from tbl_charvarlist where fld_dbid = %llu and fld_var = '%s' and fld_value is not null", to_llu(dbid()), var.c_str());

    if(query.executeStep()){
        return std::make_pair(true, cerealf::deserialize<luaf::luaVar>(query.getColumn(0).getString()));
    }
    else{
        return std::make_pair(false, luaf::luaNil{});
    }
}

void Player::dbRemoveVar(const std::string &var)
{
    fflassert(str_haschar(var));
    g_dbPod->exec("delete from tbl_charvarlist where fld_dbid = %llu and fld_var = '%s'", to_llu(dbid()), var.c_str());
}

void Player::dbUpdateExp()
{
    g_dbPod->exec(u8R"###( update tbl_char set fld_exp = %llu where fld_dbid = %llu )###", to_llu(exp()), to_llu(dbid()));
}

void Player::dbUpdateMapGLoc()
{
    g_dbPod->exec(u8R"###( update tbl_char set fld_map = %d, fld_mapx = %d, fld_mapy = %d where fld_dbid = %llu )###", to_d(mapID()), X(), Y(), to_llu(dbid()));
}

void Player::dbUpdateHealth()
{
    g_dbPod->exec(u8R"###( update tbl_char set fld_hp = %d, fld_mp = %d where fld_dbid = %llu )###", m_sdHealth.hp, m_sdHealth.mp, to_llu(dbid()));
}

void Player::dbLoadInventory()
{
    // tbl_inventory:
    // +----------+------------+-----------+-----------+--------------+-----------------+
    // | fld_dbid | fld_itemid | fld_seqid | fld_count | fld_duration | fld_extattrlist |
    // +----------+------------+-----------+-----------+--------------+-----------------+
    // |<----primary key---->|

    m_sdItemStorage.inventory.clear();
    auto query = g_dbPod->createQuery("select * from tbl_inventory where fld_dbid = %llu", to_llu(dbid()));

    while(query.executeStep()){
        SDItem item
        {
            .itemID = check_cast<uint32_t, unsigned>(query.getColumn("fld_itemid")),
            .seqID  = check_cast<uint32_t, unsigned>(query.getColumn("fld_seqid" )),
            .count  = check_cast<  size_t, unsigned>(query.getColumn("fld_count" )),
            .duration
            {
                check_cast<size_t, unsigned>(query.getColumn("fld_duration")),
                check_cast<size_t, unsigned>(query.getColumn("fld_maxduration")),
            },
            .extAttrList = cerealf::deserialize<std::unordered_map<int, std::string>>(query.getColumn("fld_extattrlist")),
        };

        fflassert(item);
        m_sdItemStorage.inventory.add(std::move(item), true);
    }
}

void Player::dbUpdateInventoryItem(const SDItem &item)
{
    fflassert(item);
    const auto attrBuf = cerealf::serialize(item.extAttrList);
    auto query = g_dbPod->createQuery(
            u8R"###( replace into tbl_inventory(fld_dbid, fld_itemid, fld_seqid, fld_count, fld_duration, fld_maxduration, fld_extattrlist) )###"
            u8R"###( values                                                                                                                 )###"
            u8R"###(     (%llu, %llu, %llu, %llu, %llu, %llu, ?)                                                                            )###",

            to_llu(dbid()),
            to_llu(item.itemID),
            to_llu(item.seqID),
            to_llu(item.count),
            to_llu(item.duration[0]),
            to_llu(item.duration[1]));

    query.bind(1, attrBuf.data(), attrBuf.length());
    query.exec();
}

void Player::dbRemoveInventoryItem(const SDItem &item)
{
    dbRemoveInventoryItem(item.itemID, item.seqID);
}

void Player::dbRemoveInventoryItem(uint32_t itemID, uint32_t seqID)
{
    // requies seqID to be non-zero
    // means won't support remove more than 1 item in database per call

    if(!(DBCOM_ITEMRECORD(itemID) && seqID > 0)){
        throw fflerror("invalid arguments: itemID = %llu, seqID = %llu", to_llu(itemID), to_llu(seqID));
    }
    g_dbPod->exec("delete from tbl_inventory where fld_dbid = %llu and fld_itemid = %llu and fld_seqid = %llu", to_llu(dbid()), to_llu(itemID), to_llu(seqID));
}

void Player::dbSecureItem(uint32_t itemID, uint32_t seqID)
{
    const auto &item = findInventoryItem(itemID, seqID);
    fflassert(item);

    const auto attrBuf = cerealf::serialize(item.extAttrList);
    auto query = g_dbPod->createQuery(
            u8R"###( replace into tbl_secureditemlist(fld_dbid, fld_itemid, fld_seqid, fld_count, fld_duration, fld_maxduration, fld_extattrlist) )###"
            u8R"###( values                                                                                                                       )###"
            u8R"###(     (%llu, %llu, %llu, %llu, %llu, %llu, ?)                                                                                  )###",

            to_llu(dbid()),
            to_llu(item.itemID),
            to_llu(item.seqID),
            to_llu(item.count),
            to_llu(item.duration[0]),
            to_llu(item.duration[1]));

    query.bind(1, attrBuf.data(), attrBuf.length());
    query.exec();
}

SDItem Player::dbRetrieveSecuredItem(uint32_t itemID, uint32_t seqID)
{
    fflassert(DBCOM_ITEMRECORD(itemID));
    fflassert(seqID > 0);

    auto query = g_dbPod->createQuery(
            u8R"###( delete from tbl_secureditemlist where fld_dbid = %llu and fld_itemid = %llu and fld_seqid = %llu returning * )###",

            to_llu(dbid()),
            to_llu(itemID),
            to_llu(seqID));

    while(query.executeStep()){
        SDItem item
        {
            .itemID = check_cast<uint32_t, unsigned>(query.getColumn("fld_itemid")),
            .seqID  = check_cast<uint32_t, unsigned>(query.getColumn("fld_seqid" )),
            .count  = check_cast<  size_t, unsigned>(query.getColumn("fld_count" )),
            .duration
            {
                check_cast<size_t, unsigned>(query.getColumn("fld_duration")),
                check_cast<size_t, unsigned>(query.getColumn("fld_maxduration")),
            },
            .extAttrList = cerealf::deserialize<std::unordered_map<int, std::string>>(query.getColumn("fld_extattrlist")),
        };

        fflassert(item);
        fflassert(!query.executeStep());
        return item;
    }
    throw fflerror("can't find item: itemID = %llu, seqID = %llu", to_llu(itemID), to_llu(seqID));
}

std::vector<SDItem> Player::dbLoadSecuredItemList() const
{
    // tbl_secureditemlist:
    // +----------+------------+-----------+-----------+--------------+-----------------+
    // | fld_dbid | fld_itemid | fld_seqid | fld_count | fld_duration | fld_extattrlist |
    // +----------+------------+-----------+-----------+--------------+-----------------+
    // |<-----primary key----->|

    std::vector<SDItem> itemList;
    auto query = g_dbPod->createQuery("select * from tbl_secureditemlist where fld_dbid = %llu", to_llu(dbid()));

    while(query.executeStep()){
        SDItem item
        {
            .itemID = check_cast<uint32_t, unsigned>(query.getColumn("fld_itemid")),
            .seqID  = check_cast<uint32_t, unsigned>(query.getColumn("fld_seqid" )),
            .count  = check_cast<  size_t, unsigned>(query.getColumn("fld_count" )),
            .duration
            {
                check_cast<size_t, unsigned>(query.getColumn("fld_duration")),
                check_cast<size_t, unsigned>(query.getColumn("fld_maxduration")),
            },
            .extAttrList = cerealf::deserialize<std::unordered_map<int, std::string>>(query.getColumn("fld_extattrlist")),
        };

        fflassert(item);
        itemList.push_back(std::move(item));
    }
    return itemList;
}

void Player::dbLoadPlayerConfig()
{
    // tbl_playerconfig:
    // +----------+------------------+-------------------+
    // | fld_dbid | fld_magickeylist | fld_runtimeconfig |
    // +----------+------------------+-------------------+

    m_sdPlayerConfig.magicKeyList.clear();

    auto query = g_dbPod->createQuery("select * from tbl_playerconfig where fld_dbid = %llu", to_llu(dbid()));

    if(query.executeStep()){
        if(const std::string buf = query.getColumn("fld_magickeylist"); !buf.empty()){
            m_sdPlayerConfig.magicKeyList = cerealf::deserialize<SDMagicKeyList>(buf);
        }

        if(const std::string buf = query.getColumn("fld_runtimeconfig"); !buf.empty()){
            m_sdPlayerConfig.runtimeConfig = cerealf::deserialize<SDRuntimeConfig>(buf);
        }
    }
}

SDPlayerCandidates Player::dbQueryPlayerCandidates(const std::string &query)
{
    fflassert(str_haschar(query));
    auto queryCmd = [&query]
    {
        if(query.find_first_not_of("0123456789") == std::string_view::npos){
            return g_dbPod->createQuery("select fld_name from tbl_char where fld_dbid = %s or instr(fld_name, '%s') > 0", query.c_str(), query.c_str());
        }
        else{
            return g_dbPod->createQuery("select fld_name from tbl_char where instr(fld_name, '%s') > 0", query.c_str());
        }
    }();

    SDPlayerCandidates sdPC;
    while(queryCmd.executeStep()){
        sdPC.list.push_back(queryCmd.getColumn("fld_name").getString());
    }
    return sdPC;
}

void Player::dbUpdateMagicKey(uint32_t magicID, char key)
{
    fflassert(m_sdLearnedMagicList.has(magicID));
    fflassert((key >= 'a' && key <= 'z') || (key >= '0' && key <= '9'));

    if(!m_sdPlayerConfig.magicKeyList.setMagicKey(magicID, key)){
        return;
    }

    const auto keyBuf = cerealf::serialize(m_sdPlayerConfig.magicKeyList);
    auto query = g_dbPod->createQuery(
            u8R"###( insert into tbl_playerconfig(fld_dbid, fld_magickeylist) )###"
            u8R"###( values                                                   )###"
            u8R"###(     (%llu, ?)                                            )###"
            u8R"###(                                                          )###"
            u8R"###( on conflict(fld_dbid) do update set                      )###"
            u8R"###(     fld_magickeylist = excluded.fld_magickeylist         )###",

            to_llu(dbid()));

    query.bind(1, keyBuf.data(), keyBuf.length());
    query.exec();
}

void Player::dbUpdateRuntimeConfig()
{
    const auto configBuf = cerealf::serialize(m_sdPlayerConfig.runtimeConfig);
    auto query = g_dbPod->createQuery(
            u8R"###( insert into tbl_playerconfig(fld_dbid, fld_runtimeconfig) )###"
            u8R"###( values                                                    )###"
            u8R"###(     (%llu, ?)                                             )###"
            u8R"###(                                                           )###"
            u8R"###( on conflict(fld_dbid) do update set                       )###"
            u8R"###(     fld_runtimeconfig = excluded.fld_runtimeconfig        )###",

            to_llu(dbid()));

    query.bind(1, configBuf.data(), configBuf.length());
    query.exec();
}

void Player::dbLoadBelt()
{
    // tbl_belt:
    // +----------+----------+------------+-----------+
    // | fld_dbid | fld_belt | fld_itemid | fld_count |
    // +----------+----------+------------+-----------+
    // |<----primary key---->|

    m_sdItemStorage.belt.clear();
    auto query = g_dbPod->createQuery("select * from tbl_belt where fld_dbid = %llu", to_llu(dbid()));

    while(query.executeStep()){
        const auto index = check_cast<size_t, unsigned>(query.getColumn("fld_belt"));
        if(index >= 6){
            throw fflerror("invalid belt slot: %zu", index);
        }

        const auto itemID = check_cast<uint32_t, unsigned>(query.getColumn("fld_itemid"));
        const auto typeStr = to_u8sv(DBCOM_ITEMRECORD(itemID).type);
        if(true
                && typeStr != u8"恢复药水"
                && typeStr != u8"传送卷轴"){
            throw fflerror("invalid item type to belt slot");
        }

        SDItem item
        {
            .itemID = itemID,
            .count  = check_cast<size_t, unsigned>(query.getColumn("fld_count")),
        };

        fflassert(item);
        m_sdItemStorage.belt.list.at(index) = std::move(item);
    }
}

void Player::dbUpdateBeltItem(size_t slot, const SDItem &item)
{
    if(slot >= 6){
        throw fflerror("invalid belt slot: %zu", slot);
    }

    fflassert(item);
    const auto typeStr = to_u8sv(DBCOM_ITEMRECORD(item.itemID).type);
    if(true
            && typeStr != u8"恢复药水"
            && typeStr != u8"传送卷轴"){
        throw fflerror("invalid item type to belt slot");
    }

    g_dbPod->exec(
            u8R"###( replace into tbl_belt(fld_dbid, fld_belt, fld_itemid, fld_count) )###"
            u8R"###( values                                                           )###"
            u8R"###(     (%llu, %llu, %llu, %llu)                                     )###",

            to_llu(dbid()),
            to_llu(slot),
            to_llu(item.itemID),
            to_llu(item.count));
}

void Player::dbRemoveBeltItem(size_t slot)
{
    if(slot >= 6){
        throw fflerror("invalid belt slot: %zu", slot);
    }
    g_dbPod->exec("delete from tbl_belt where fld_dbid = %llu and fld_belt = %zu", to_llu(dbid()), slot);
}

void Player::dbLoadWear()
{
    // tbl_wear:
    // +----------+----------+------------+-----------+--------------+-----------------+
    // | fld_dbid | fld_wear | fld_itemid | fld_count | fld_duration | fld_extattrlist |
    // +----------+----------+------------+-----------+--------------+-----------------+
    // |<----primary key---->|

    m_sdItemStorage.wear.clear();
    auto query = g_dbPod->createQuery("select * from tbl_wear where fld_dbid = %llu", to_llu(dbid()));

    while(query.executeStep()){
        const auto wltype = check_cast<int, unsigned>(query.getColumn("fld_wear"));
        SDItem item
        {
            .itemID = check_cast<uint32_t, unsigned>(query.getColumn("fld_itemid")),
            .seqID  = 0,
            .count  = check_cast<size_t, unsigned>(query.getColumn("fld_count")),
            .duration
            {
                check_cast<size_t, unsigned>(query.getColumn("fld_duration")),
                check_cast<size_t, unsigned>(query.getColumn("fld_maxduration")),
            },
            .extAttrList = cerealf::deserialize<std::unordered_map<int, std::string>>(query.getColumn("fld_extattrlist")),
        };

        if(!DBCOM_ITEMRECORD(item.itemID).wearable(wltype)){
            throw fflerror("invalid item type to wear grid");
        }

        fflassert(item);
        m_sdItemStorage.wear.setWLItem(wltype, std::move(item));
    }
}

void Player::dbUpdateWearItem(int wltype, const SDItem &item)
{
    if(!(wltype >= WLG_BEGIN && wltype < WLG_END)){
        throw fflerror("bad wltype: %d", wltype);
    }

    fflassert(item);
    if(!DBCOM_ITEMRECORD(item.itemID).wearable(wltype)){
        throw fflerror("invalid item type to wear grid");
    }

    // only save itemID and wltype
    // drop the seqID when saving to database

    const auto attrBuf = cerealf::serialize(item.extAttrList);
    auto query = g_dbPod->createQuery(
            u8R"###( replace into tbl_wear(fld_dbid, fld_wear, fld_itemid, fld_count, fld_duration, fld_maxduration, fld_extattrlist) )###"
            u8R"###( values                                                                                                           )###"
            u8R"###(     (%llu, %llu, %llu, %llu, %llu, %llu, ?)                                                                      )###",

            to_llu(dbid()),
            to_llu(wltype),
            to_llu(item.itemID),
            to_llu(item.count),
            to_llu(item.duration[0]),
            to_llu(item.duration[1]));

    query.bind(1, attrBuf.data(), attrBuf.length());
    query.exec();
}

void Player::dbRemoveWearItem(int wltype)
{
    if(!(wltype >= WLG_BEGIN && wltype < WLG_END)){
        throw fflerror("bad wltype: %d", wltype);
    }
    g_dbPod->exec("delete from tbl_wear where fld_dbid = %llu and fld_wear = %llu", to_llu(dbid()), to_llu(wltype));
}

void Player::dbLearnMagic(uint32_t magicID)
{
    // tbl_learnedmagiclist:
    // +----------+-------------+---------+
    // | fld_dbid | fld_magicid | fld_exp |
    // +----------+-------------+---------+
    // |<-----primary key------>|

    fflassert(DBCOM_MAGICRECORD(magicID));
    g_dbPod->exec(
            u8R"###( insert into tbl_learnedmagiclist(fld_dbid, fld_magicid) )###"
            u8R"###( values                                                  )###"
            u8R"###(     (%llu, %llu)                                        )###",

            to_llu(dbid()),
            to_llu(magicID));
}

void Player::dbAddMagicExp(uint32_t magicID, size_t exp)
{
    fflassert(DBCOM_MAGICRECORD(magicID));
    if(exp > 0){
        g_dbPod->exec("update tbl_learnedmagiclist set fld_exp = fld_exp + %llu where fld_dbid = %llu and fld_magic = %zu", to_llu(dbid()), to_llu(magicID), exp);
    }
}

void Player::dbLoadLearnedMagic()
{
    // tbl_learnedmagiclist:
    // +----------+-----------+---------+
    // | fld_dbid | fld_magic | fld_exp |
    // +----------+-----------+---------+
    // |<----primary key----->|

    m_sdLearnedMagicList.clear();
    auto query = g_dbPod->createQuery("select * from tbl_learnedmagiclist where fld_dbid = %llu", to_llu(dbid()));

    while(query.executeStep()){
        m_sdLearnedMagicList.magicList.push_back(SDLearnedMagic
        {
            .magicID = check_cast<uint32_t, unsigned>(query.getColumn("fld_magicid")),
            .exp = query.getColumn("fld_exp"),
        });
    }
}

void Player::dbLoadFriendList()
{
    // tbl_learnedmagiclist:
    // +----------+------------+
    // | fld_dbid | fld_friend |
    // +----------+------------+
    // |          +------+
    // |                 |
    // |<--primary key-->|

    m_sdFriendList.clear();
    auto query = g_dbPod->createQuery("select * from tbl_friend where fld_dbid = %llu", to_llu(dbid()));

    while(query.executeStep()){
        m_sdFriendList.push_back(check_cast<uint32_t, unsigned>(query.getColumn("fld_friend")));
    }
}

std::tuple<uint64_t, uint64_t> Player::dbSaveChatMessage(uint32_t toDBID, const std::string_view &sv)
{
    auto tstamp= hres_tstamp::localtime();
    auto query = g_dbPod->createQuery(
        u8R"###( insert into tbl_chatmessage(fld_from, fld_to, fld_timestamp, fld_message) )###"
        u8R"###( values                                                                    )###"
        u8R"###(     (%llu, %llu, %llu, ?)                                                 )###"
        u8R"###( returning                                                                 )###"
        u8R"###(     fld_id;                                                               )###",

        to_llu(dbid()),
        to_llu(toDBID),
        to_llu(tstamp));

    query.bind(1, sv.data(), sv.size());
    if(query.executeStep()){
        return {query.getColumn("fld_id").getInt64(), tstamp};
    }
    else{
        throw fflerror("failed to insert chat message to database");
    }
}

SDChatMessageList Player::dbRetrieveLatestChatMessage(const uint32_t *dbidList, size_t dbidCount, size_t limitPerDBID, bool includeSend, bool includeRecv)
{
    fflassert(dbidList);
    fflassert(dbidCount);
    fflassert(includeSend || includeRecv);

    std::vector<std::string> queries;
    for(const auto other: std::span(dbidList, dbidCount)){
        queries.push_back("select * from ( select * from tbl_chatmessage where ");
        if(includeSend){
            queries.back().append(str_printf("(fld_from = %llu and fld_to = %llu) ", to_llu(dbid()), to_llu(other)));
        }

        if(includeRecv){
            if(includeSend){
                queries.back().append("or ");
            }
            queries.back().append(str_printf("(fld_from = %llu and fld_to = %llu) ", to_llu(other), to_llu(dbid())));
        }

        queries.back().append("order by fld_timestamp desc ");

        if(limitPerDBID > 0){
            queries.back().append(str_printf("limit %zu ", limitPerDBID));
        }

        queries.back().append(" )");
    }

    SDChatMessageList result;
    auto query = g_dbPod->createQuery(str_join(queries, " union ").c_str());

    while(query.executeStep()){
        result.push_back(SDChatMessage
        {
            .id        = check_cast<uint64_t, unsigned>(query.getColumn("fld_id")),
            .from      = check_cast<uint32_t, unsigned>(query.getColumn("fld_from")),
            .to        = check_cast<uint32_t, unsigned>(query.getColumn("fld_to")),
            .timestamp = check_cast<uint64_t, unsigned>(query.getColumn("fld_timestamp")),
            .message   = query.getColumn("fld_message").getString(),
        });
    }
    return result;
}

// std::vector<SDChatMessage> Player::dbRetrieveChatMessageList(const DBRetrieveChatMessageListParams &params)
// {
//     std::vector<std::string> conds;
//
//     if(params.id > 0){
//         conds.push_back(str_printf("(fld_id = %llu)", to_llu(params.id)));
//     }
//
//     if(params.from > 0){
//         conds.push_back(str_printf("(fld_from = %llu)", to_llu(params.from)));
//     }
//
//     if(params.to > 0){
//         conds.push_back(str_printf("(fld_to = %llu)", to_llu(params.to)));
//     }
//
//     if(params.dateFrom > 0){
//         conds.push_back(str_printf("(fld_timestamp >= %llu)", to_llu(params.dateFrom)));
//     }
//
//     if(params.dateTo > 0){
//         if(params.dateFrom > params.dateTo){
//             throw fflerror("invalid date range, from %llu, to %llu", to_llu(params.dateFrom), to_llu(params.dateTo));
//         }
//         conds.push_back(str_printf("(fld_timestamp <= %llu)", to_llu(params.dateTo)));
//     }
//
//     std::string query = "select * from tbl_chatmessage";
//     if(!conds.empty()){
//         query.append(" where ");
//         query.append(str_join(conds, " and "));
//     }
//
//     if(params.limitCount > 0){
//         query.append(str_printf(" limit %llu"), to_llu(params.limitCount));
//     }
//
//     query.append("order by fld_timestamp");
//
//     std::vector<SDChatMessage> result;
//     auto query = g_dbPod->createQuery(query.c_str());
//
//     while(query.executeStep()){
//         m_sdFriendList.push_back(SDChatMessage
//         {
//             .id        = check_cast<uint64_t, unsigned long>(query.getColumn("fld_id")),
//             .from      = check_cast<uint32_t, unsigned long>(query.getColumn("fld_from")),
//             .to        = check_cast<uint32_t, unsigned long>(query.getColumn("fld_to")),
//             .timestamp = check_cast<uint64_t, unsigned long>(query.getColumn("fld_timestamp")),
//             .message   = cerealf::deserialize(query.getColumn("fld_message")),
//         });
//     }
//     return result;
// }
