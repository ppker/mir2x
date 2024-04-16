#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <cstring>
#include <cstdarg>
#include <cstdlib>
#include <cinttypes>
#include <FL/fl_ask.H>

#include "log.hpp"
#include "jobf.hpp"
#include "dbpod.hpp"
#include "totype.hpp"
#include "filesys.hpp"
#include "message.hpp"
#include "monster.hpp"
#include "mapbindb.hpp"
#include "fflerror.hpp"
#include "netdriver.hpp"
#include "serdesmsg.hpp"
#include "actorpool.hpp"
#include "syncdriver.hpp"
#include "mainwindow.hpp"
#include "monoserver.hpp"
#include "dispatcher.hpp"
#include "servicecore.hpp"
#include "commandwindow.hpp"

extern Log *g_log;
extern DBPod *g_dbPod;
extern MapBinDB *g_mapBinDB;
extern ActorPool *g_actorPool;
extern NetDriver *g_netDriver;
extern MonoServer *g_monoServer;
extern MainWindow *g_mainWindow;
extern ServerConfigureWindow *g_serverConfigureWindow;

void MonoServer::addLog(const Log::LogTypeLoc &typeLoc, const char *format, ...)
{
    std::string s;
    str_format(format, s);

    std::string logLine;
    std::stringstream errStream(s);
    std::vector<std::string> multiLine;

    while(std::getline(errStream, logLine, '\n')){
        multiLine.push_back(std::move(logLine));
    }

    if(const int logType = std::get<0>(typeLoc); logType != Log::LOGTYPEV_DEBUG){
        {
            const std::lock_guard<std::mutex> lockGuard(m_logLock);
            for(const auto &line: multiLine){
                m_logBuf.push_back((char)(logType));
                m_logBuf.insert(m_logBuf.end(), line.c_str(), line.c_str() + line.size() + 1); // add extra '\0'
            }
        }
        notifyGUI("FlushBrowser");
    }

    for(const auto &line: multiLine){
        g_log->addLog(typeLoc, "%s", line.c_str());
    }
}

void MonoServer::addCWLogString(uint32_t cwID, int logType, const char *prompt, const char *log)
{
    if(!str_haschar(prompt)){
        prompt = "";
    }

    if(cwID && (logType == 0 || logType == 1 || logType == 2)){
        // we should lock the internal buffer record
        // we won't assess any gui instance in this function
        {
            std::lock_guard<std::mutex> lockGuard(m_CWLogLock);
            m_CWLogBuf.insert(m_CWLogBuf.end(), (char *)(&cwID), (char *)(&cwID) + sizeof(cwID));
            m_CWLogBuf.push_back((char)(logType));

            const auto logString = to_cstr(log);
            m_CWLogBuf.insert(m_CWLogBuf.end(), prompt, prompt + std::strlen(prompt) + 1);
            m_CWLogBuf.insert(m_CWLogBuf.end(), logString, logString + std::strlen(logString) + 1);
        }
        notifyGUI("FlushCWBrowser");
    }
}

bool MonoServer::hasDatabase() const
{
    return g_dbPod->createQuery(u8R"###(select name from sqlite_master where type='table')###").executeStep();
}

bool MonoServer::hasCharacter(const char *charName) const
{
    if(!str_haschar(charName)){
        throw fflerror("invalid char name: %s", to_cstr(charName));
    }
    return g_dbPod->createQuery(u8R"###(select fld_dbid from tbl_char where fld_name = '%s')###", charName).executeStep();
}

int MonoServer::createAccount(const char *id, const char *password)
{
    if(!(str_haschar(id) && (str_haschar(password)))){
        throw fflerror("bad account: id = %s, password = %s", to_cstr(id), to_cstr(password));
    }

    if(!hasDatabase()){
        throw fflerror("no available database");
    }

    if(g_dbPod->createQuery(u8R"###(select fld_dbid from tbl_account where fld_account ='%s')###", id).executeStep()){
        return CRTACCERR_ACCOUNTEXIST;
    }

    g_dbPod->exec(u8R"###(insert into tbl_account(fld_account, fld_password) values ('%s', '%s'))###", id, password);
    return CRTACCERR_NONE;
}

bool MonoServer::createAccountCharacter(const char *id, const char *charName, bool gender, int job)
{
    fflassert(str_haschar(id));
    fflassert(str_haschar(charName));
    fflassert(jobf::jobValid(job));
    fflassert(hasDatabase());

    if(hasCharacter(charName)){
        return false;
    }

    auto queryAccount = g_dbPod->createQuery(u8R"###(select fld_dbid from tbl_account where fld_account ='%s')###", id);
    if(!queryAccount.executeStep()){
        return false;
    }

    const auto dbid = check_cast<uint32_t, unsigned>(queryAccount.getColumn("fld_dbid"));
    const auto [mapID, mapx, mapy] = [job]() -> std::tuple<uint32_t, int, int>
    {
        if(job & (JOB_WIZARD | JOB_WARRIOR)){
            return {DBCOM_MAPID(u8"比奇县_0"), 441, 381};
        }

        if(job & JOB_TAOIST){
            return {DBCOM_MAPID(u8"道馆_1"), 405, 120};
        }

        throw fflerror("invalid job %d", job);
    }();

    g_dbPod->exec
    (
        u8R"###( insert into tbl_char(fld_dbid, fld_name, fld_gender, fld_job, fld_map, fld_mapx, fld_mapy) )###"
        u8R"###( values                                                                                     )###"
        u8R"###(     (%llu, '%s', %d, %d, %d, %d, %d);                                                      )###",

        to_llu(dbid),
        charName,
        to_d(gender),
        job,
        to_d(mapID),
        mapx,
        mapy
    );

    auto query = g_dbPod->createQuery(
        u8R"###( insert into tbl_chatmessage(fld_group, fld_from, fld_to, fld_timestamp, fld_message) )###"
        u8R"###( values                                                                               )###"
        u8R"###(     (0, %llu, %llu, %llu, ?);                                                        )###",

        to_llu(SYS_CHATDBID_SYSTEM),
        to_llu(dbid),
        to_llu(hres_tstamp::localtime()));

    query.bind(1, cerealf::serialize(std::string("<layout><par>欢迎来到mir2x传奇的世界！</par></layout>")));
    query.exec();

    uint32_t seqID = 1;
    const auto fnAddInitItem = [dbid, &seqID](const char8_t *itemName, size_t count = 1)
    {
        SDItem item
        {
            .itemID = DBCOM_ITEMID(itemName),
            .seqID  = seqID++,
            .count  = count,
        };

        if(DBCOM_ITEMRECORD(item.itemID).isWeapon()){
            item.extAttrList.insert(SDItem::build_EA_DC(100));
        }

        fflassert(item);
        const auto attrBuf = cerealf::serialize(item.extAttrList);
        auto query = g_dbPod->createQuery(
                u8R"###( replace into tbl_inventory(fld_dbid, fld_itemid, fld_seqid, fld_count, fld_duration, fld_maxduration, fld_extattrlist) )###"
                u8R"###( values                                                                                                                 )###"
                u8R"###(     (%llu, %llu, %llu, %llu, %llu, %llu, ?)                                                                            )###",

                to_llu(dbid),
                to_llu(item.itemID),
                to_llu(item.seqID),
                to_llu(item.count),
                to_llu(item.duration[0]),
                to_llu(item.duration[1]));

        query.bind(1, attrBuf.data(), attrBuf.length());
        query.exec();
    };

    fnAddInitItem(u8"火墙");
    fnAddInitItem(u8"地狱火");
    fnAddInitItem(u8"雷电术");
    fnAddInitItem(u8"召唤神兽");

    fnAddInitItem(u8"木剑");
    fnAddInitItem(u8"金创药（小）", 10);
    fnAddInitItem(u8"魔法药（小）", 10);

    if(gender){
        fnAddInitItem(u8"布衣（男）");
    }
    else{
        fnAddInitItem(u8"布衣（女）");
    }
    return true;
}

void MonoServer::createDefaultDatabase()
{
    const char8_t *defSQLCmdList[]
    {
        u8R"###( create table tbl_account(                                          )###"
        u8R"###(     fld_dbid           integer not null primary key autoincrement, )###"
        u8R"###(     fld_account        text    not null,                           )###"
        u8R"###(     fld_password       text    not null                            )###"
        u8R"###( ) strict;                                                          )###",

        u8R"###( insert into tbl_account(fld_dbid, fld_account, fld_password)       )###"
        u8R"###( values                                                             )###"
        u8R"###(     (1, 'admin', 'admin');                                         )###",

        u8R"###( create table tbl_char(                                             )###"
        u8R"###(     fld_dbid           integer not null primary key,               )###"
        u8R"###(     fld_name           text    not null,                           )###"
        u8R"###(     fld_namecolor      integer default 0,                          )###"
        u8R"###(     fld_gender         integer not null,                           )###"
        u8R"###(     fld_job            integer not null,                           )###"
        u8R"###(     fld_map            integer not null,                           )###"
        u8R"###(     fld_mapx           integer not null,                           )###"
        u8R"###(     fld_mapy           integer not null,                           )###"
        u8R"###(     fld_hp             integer default 10,                         )###"
        u8R"###(     fld_mp             integer default 10,                         )###"
        u8R"###(     fld_exp            integer default 0,                          )###"
        u8R"###(     fld_gold           integer default 10000,                      )###"
        u8R"###(     fld_hair           integer default 0,                          )###"
        u8R"###(     fld_haircolor      integer default 0,                          )###"
        u8R"###(                                                                    )###"
        u8R"###(     foreign key (fld_dbid) references tbl_account(fld_dbid)        )###"
        u8R"###( ) strict;                                                          )###",

        u8R"###( create table tbl_charvarlist(                                      )###"
        u8R"###(     fld_dbid           integer not null,                           )###"
        u8R"###(     fld_var            text    not null,                           )###"
        u8R"###(     fld_value          blob        null,                           )###"
        u8R"###(                                                                    )###"
        u8R"###(     foreign key (fld_dbid) references tbl_char(fld_dbid),          )###"
        u8R"###(     primary key (fld_dbid, fld_var)                                )###"
        u8R"###( ) strict;                                                          )###",

        u8R"###( create table tbl_belt(                                             )###"
        u8R"###(     fld_dbid           integer not null,                           )###"
        u8R"###(     fld_belt           integer not null,                           )###"
        u8R"###(                                                                    )###"
        u8R"###(     fld_itemid         integer not null,                           )###"
        u8R"###(     fld_count          integer not null,                           )###"
        u8R"###(                                                                    )###"
        u8R"###(     foreign key (fld_dbid) references tbl_char(fld_dbid),          )###"
        u8R"###(     primary key (fld_dbid, fld_belt)                               )###"
        u8R"###( ) strict;                                                          )###",

        u8R"###( create table tbl_wear(                                             )###"
        u8R"###(     fld_dbid           integer not null,                           )###"
        u8R"###(     fld_wear           integer not null,                           )###"
        u8R"###(                                                                    )###"
        u8R"###(     fld_itemid         integer not null,                           )###"
        u8R"###(     fld_count          integer not null,                           )###"
        u8R"###(     fld_duration       integer not null,                           )###"
        u8R"###(     fld_maxduration    integer not null,                           )###"
        u8R"###(     fld_extattrlist    blob    not null,                           )###"
        u8R"###(                                                                    )###"
        u8R"###(     foreign key (fld_dbid) references tbl_char(fld_dbid),          )###"
        u8R"###(     primary key (fld_dbid, fld_wear)                               )###"
        u8R"###( ) strict;                                                          )###",

        u8R"###( create table tbl_inventory(                                        )###"
        u8R"###(     fld_dbid           integer not null,                           )###"
        u8R"###(     fld_itemid         integer not null,                           )###"
        u8R"###(     fld_seqid          integer not null,                           )###"
        u8R"###(     fld_count          integer not null,                           )###"
        u8R"###(     fld_duration       integer not null,                           )###"
        u8R"###(     fld_maxduration    integer not null,                           )###"
        u8R"###(     fld_extattrlist    blob    not null,                           )###"
        u8R"###(                                                                    )###"
        u8R"###(     foreign key (fld_dbid) references tbl_char(fld_dbid),          )###"
        u8R"###(     primary key (fld_dbid, fld_itemid, fld_seqid)                  )###"
        u8R"###( ) strict;                                                          )###",

        u8R"###( create table tbl_secureditemlist(                                  )###"
        u8R"###(     fld_dbid           integer not null,                           )###"
        u8R"###(     fld_itemid         integer not null,                           )###"
        u8R"###(     fld_seqid          integer not null,                           )###"
        u8R"###(     fld_count          integer not null,                           )###"
        u8R"###(     fld_duration       integer not null,                           )###"
        u8R"###(     fld_maxduration    integer not null,                           )###"
        u8R"###(     fld_extattrlist    blob    not null,                           )###"
        u8R"###(                                                                    )###"
        u8R"###(     foreign key (fld_dbid) references tbl_char(fld_dbid),          )###"
        u8R"###(     primary key (fld_dbid, fld_itemid, fld_seqid)                  )###"
        u8R"###( ) strict;                                                          )###",

        u8R"###( create table tbl_learnedmagiclist(                                 )###"
        u8R"###(     fld_dbid           integer not null,                           )###"
        u8R"###(     fld_magicid        integer not null,                           )###"
        u8R"###(     fld_exp            integer default 0,                          )###"
        u8R"###(                                                                    )###"
        u8R"###(     foreign key (fld_dbid) references tbl_char(fld_dbid),          )###"
        u8R"###(     primary key (fld_dbid, fld_magicid)                            )###"
        u8R"###( ) strict;                                                          )###",

        u8R"###( create table tbl_playerconfig(                                     )###"
        u8R"###(     fld_dbid           integer not null,                           )###"
        u8R"###(     fld_magickeylist   blob        null default (x''),             )###"
        u8R"###(     fld_runtimeconfig  blob        null default (x''),             )###"
        u8R"###(                                                                    )###"
        u8R"###(     foreign key (fld_dbid) references tbl_char(fld_dbid),          )###"
        u8R"###(     primary key (fld_dbid)                                         )###"
        u8R"###( ) strict;                                                          )###",

        u8R"###( create table tbl_chatgroup(                                        )###"
        u8R"###(     fld_dbid           integer not null primary key autoincrement, )###"
        u8R"###(     fld_creator        integer not null,                           )###"
        u8R"###(     fld_createtime     integer not null,                           )###"
        u8R"###(     fld_name           blob        null default (x''),             )###"
        u8R"###(     fld_description    blob        null default (x''),             )###"
        u8R"###(     fld_announcement   blob        null default (x''),             )###"
        u8R"###(                                                                    )###"
        u8R"###(     foreign key (fld_creator) references tbl_char(fld_dbid)        )###"
        u8R"###( ) strict;                                                          )###",

        u8R"###( create table tbl_chatgroupmember(                                  )###"
        u8R"###(     fld_group          integer not null,                           )###"
        u8R"###(     fld_member         integer not null,                           )###"
        u8R"###(     fld_permission     integer not null,                           )###"
        u8R"###(     fld_jointime       integer not null,                           )###"
        u8R"###(                                                                    )###"
        u8R"###(     foreign key (fld_group ) references tbl_chatgroup(fld_dbid),   )###"
        u8R"###(     foreign key (fld_member) references tbl_char     (fld_dbid),   )###"
        u8R"###(     primary key (fld_group, fld_member)                            )###"
        u8R"###( ) strict;                                                          )###",

        u8R"###( create table tbl_friend(                                           )###"
        u8R"###(     fld_dbid           integer not null,                           )###"
        u8R"###(     fld_friend         integer not null,                           )###"
        u8R"###(                                                                    )###"
        u8R"###(     foreign key (fld_dbid  ) references tbl_char(fld_dbid),        )###"
        u8R"###(     foreign key (fld_friend) references tbl_char(fld_dbid),        )###"
        u8R"###(     primary key (fld_dbid, fld_friend)                             )###"
        u8R"###( ) strict;                                                          )###",

        u8R"###( create table tbl_chatmessage(                                      )###"
        u8R"###(     fld_id             integer not null primary key autoincrement, )###"
        u8R"###(     fld_group          integer not null,                           )###"
        u8R"###(     fld_from           integer not null,                           )###"
        u8R"###(     fld_to             integer not null,                           )###"
        u8R"###(     fld_timestamp      integer not null,                           )###"
        u8R"###(     fld_message        blob        null default (x''),             )###"
        u8R"###(                                                                    )###"
        u8R"###(     foreign key (fld_from) references tbl_char(fld_dbid),          )###"
        u8R"###(     foreign key (fld_to  ) references tbl_char(fld_dbid)           )###"
        u8R"###( ) strict;                                                          )###",
    };

    {
        auto dbTrans = g_dbPod->createTransaction();
        for(const auto sqlCmd: defSQLCmdList){
            g_dbPod->exec(sqlCmd);
        }
        dbTrans.commit();
    }

    createAccount("test", "123456");
    createAccount("good", "123456");
    createAccount("id_1", "123456");
    createAccount("id_2", "123456");

    createAccountCharacter("test", to_cstr(u8"亚当"),  true, JOB_TAOIST  | JOB_WIZARD);
    createAccountCharacter("good", to_cstr(u8"夏娃"), false, JOB_WARRIOR | JOB_WIZARD);
    createAccountCharacter("id_1", to_cstr(u8"逗逼"),  true, JOB_WIZARD);
    createAccountCharacter("id_2", to_cstr(u8"盖亚"),  true, JOB_WIZARD);

    addLog(LOGTYPE_INFO, "Create default sqlite3 database done");
}

void MonoServer::CreateDBConnection()
{
    const char *dbName = "mir2x.db3";
    g_dbPod->launch(dbName);

    if(!hasDatabase()){
        createDefaultDatabase();
    }
    addLog(LOGTYPE_INFO, "Connect to database %s successfully", dbName);
}

void MonoServer::LoadMapBinDB()
{
    std::string szMapPath = g_serverConfigureWindow->getConfig().mapPath;

    if(!g_mapBinDB->load(szMapPath.c_str())){
        throw fflerror("Failed to load mapbindb");
    }
}

void MonoServer::StartServiceCore()
{
    g_actorPool->launchPool();

    m_serviceCore = new ServiceCore();
    m_serviceCore->activate(-1.0);
}

void MonoServer::StartNetwork()
{
    g_netDriver->launch(g_serverConfigureWindow->getConfig().listenPort);
}

void MonoServer::Launch()
{
    CreateDBConnection();
    LoadMapBinDB();

    StartServiceCore();
    StartNetwork();
}

void MonoServer::propagateException() noexcept
{
    // TODO
    // add multi-thread protection
    try{
        if(!std::current_exception()){
            addLog(LOGTYPE_WARNING, "call MonoServer::propagateException() without exception captured");
            return;
        }

        // we do have an exception
        // but may not be std::exception, nest it...
        std::throw_with_nested(fflerror("rethrow in MonoServer::propagateException()"));
    }
    catch(...){
        // must have one exception...
        // now we are sure main thread will always capture an std::exception
        m_currException = std::current_exception();
        Fl::awake((void *)(uintptr_t)(2));
    }
}

void MonoServer::checkException()
{
    if(m_currException){
        std::rethrow_exception(m_currException);
    }
}

void MonoServer::logException(const std::exception &except, std::string *strPtr) noexcept
{
    addLog(LOGTYPE_WARNING, "%s", except.what());
    try{
        if(strPtr){
            strPtr->assign(except.what());
        }
        std::rethrow_if_nested(except);
    }
    catch(const std::exception &nextedExcept){
        logException(nextedExcept, strPtr);
    }
    catch(...){
        addLog(LOGTYPE_WARNING, "Can't recognize exception, skipped...");
    }
}

void MonoServer::restart(const std::string &msg)
{
    // TODO: FLTK multi-threading support is weak, see:
    // http://www.fltk.org/doc-1.3/advanced.html#advanced_multithreading

    // Fl::awake() will send message to main loop
    // define the main loop to call exit(0) when pass 1 to main thread

    // this function itself can be called from
    //   1. main loop
    //   2. child thread

    if(msg.empty()){
        notifyGUI("Restart");
    }

    else if(msg.find('\n') != std::string::npos){
        notifyGUI("Restart\nfatal message contains \\n, ignored");
    }

    else{
        notifyGUI(std::string("Restart") + "\n" + msg);
    }
}

bool MonoServer::addMonster(uint32_t monsterID, uint32_t mapID, int x, int y, bool strictLoc)
{
    AMAddCharObject amACO;
    std::memset(&amACO, 0, sizeof(amACO));

    amACO.type = UID_MON;
    amACO.x = x;
    amACO.y = y;
    amACO.mapID = mapID;
    amACO.strictLoc = strictLoc;

    amACO.monster.monsterID = monsterID;
    amACO.monster.masterUID = 0;
    addLog(LOGTYPE_INFO, "Try to add monster, monsterID = %llu", to_llu(monsterID));

    switch(auto stRMPK = SyncDriver().forward(uidf::getServiceCoreUID(), {AM_ADDCO, amACO}, 0, 0); stRMPK.type()){
        case AM_UID:
            {
                if(const auto amUID = stRMPK.conv<AMUID>(); amUID.UID){
                    addLog(LOGTYPE_INFO, "Add monster succeeds");
                    return true;
                }
                break;
            }
        default:
            {
                break;
            }
    }

    addLog(LOGTYPE_WARNING, "Add monster failed");
    return false;
}

bool MonoServer::addNPChar(const char *npcName, uint16_t lookID, uint32_t mapID, int x, int y, int gfxDir, const char *fullScriptName)
{
    fflassert(mapID);
    fflassert(x >= 0);
    fflassert(y >= 0);

    fflassert(str_haschar(npcName));
    fflassert(str_haschar(fullScriptName));
    fflassert(filesys::hasFile(fullScriptName), fullScriptName);

    AMAddCharObject amACO;
    std::memset(&amACO, 0, sizeof(amACO));

    amACO.type  = UID_NPC;
    amACO.mapID = mapID;
    amACO.x = x;
    amACO.y = y;
    amACO.strictLoc = false;

    amACO.buf.assign(cerealf::serialize(SDInitNPChar
    {
        .lookID = lookID,
        .npcName = npcName,
        .fullScriptName = fullScriptName,
        .mapID = mapID,
        .x = x,
        .y = y,
        .gfxDir = gfxDir,
    }));

    addLog(LOGTYPE_INFO, "Try to add NPC, script: %s", to_cstr(fullScriptName));
    switch(auto rmpk = SyncDriver().forward(uidf::getServiceCoreUID(), {AM_ADDCO, amACO}, 0, 0); rmpk.type()){
        case AM_UID:
            {
                if(const auto amUID = rmpk.conv<AMUID>(); amUID.UID){
                    addLog(LOGTYPE_INFO, "Add NPC succeeds");
                    return true;
                }
                break;
            }
        default:
            {
                break;
            }
    }

    addLog(LOGTYPE_WARNING, "Add NPC failed");
    return false;
}

bool MonoServer::loadMap(const std::string &mapName)
{
    AMLoadMap amLM;
    std::memset(&amLM, 0, sizeof(amLM));

    amLM.mapID = DBCOM_MAPID(to_u8cstr(mapName));
    if(amLM.mapID == 0){
        addLog(LOGTYPE_WARNING, "Invalid map name: %s", to_cstr(mapName));
        return false;
    }

    switch(const auto rmpk = SyncDriver().forward(uidf::getServiceCoreUID(), {AM_LOADMAP, amLM}); rmpk.type()){
        case AM_LOADMAPOK:
            {
                addLog(LOGTYPE_INFO, "Load map %s: uid = %s", to_cstr(mapName), uidf::getUIDString(uidf::getMapBaseUID(amLM.mapID)).c_str());
                return true;
            }
        default:
            {
                addLog(LOGTYPE_WARNING, "Load map %s failed", to_cstr(mapName));
                return false;
            }
    }
}

std::vector<int> MonoServer::getMapList()
{
    switch(auto stRMPK = SyncDriver().forward(uidf::getServiceCoreUID(), AM_QUERYMAPLIST); stRMPK.type()){
        case AM_MAPLIST:
            {
                AMMapList amML;
                std::memcpy(&amML, stRMPK.data(), sizeof(amML));

                std::vector<int> stMapList;
                for(size_t nIndex = 0; nIndex < std::extent<decltype(amML.MapList)>::value; ++nIndex){
                    if(amML.MapList[nIndex]){
                        stMapList.push_back(to_d(amML.MapList[nIndex]));
                    }
                    else{
                        break;
                    }
                }
                return stMapList;
            }
        default:
            {
                return {};
            }
    }
}

sol::optional<int> MonoServer::getMonsterCount(int nMonsterID, int nMapID)
{
    // I have two ways to implement this function
    //  1. getMapList() / GetMapUID() / GetMapAddress() / QueryMonsterCount()
    //  2. send QueryMonsterCount to ServiceCore, ServiceCore queries all maps and return the sum
    // currently using method-2

    if(true
            && nMapID     >= 0      // 0 means check all
            && nMonsterID >= 0){    // 0 means check all types

        // OK we send request to service core
        // and poll the result here till we get the sum

        AMQueryCOCount amQCOC;
        std::memset(&amQCOC, 0, sizeof(amQCOC));

        amQCOC.mapID                = to_u32(nMapID);
        amQCOC.Check.Monster        = true;
        amQCOC.CheckParam.MonsterID = to_u32(nMonsterID);

        switch(auto stRMPK = SyncDriver().forward(uidf::getServiceCoreUID(), {AM_QUERYCOCOUNT, amQCOC}); stRMPK.type()){
            case AM_COCOUNT:
                {
                    AMCOCount amCOC;
                    std::memcpy(&amCOC, stRMPK.data(), sizeof(amCOC));

                    // may overflow
                    // should put some check here
                    return sol::optional<int>(to_d(amCOC.Count));
                }
            case AM_ERROR:
            default:
                {
                    break;
                }
        }
    }
    return sol::optional<int>();
}

void MonoServer::notifyGUI(std::string notifStr)
{
    if(!notifStr.empty()){
        {
            std::lock_guard<std::mutex> lockGuard(m_notifyGUILock);
            m_notifyGUIQ.push(notifStr);
        }
        Fl::awake((void *)(uintptr_t)(1));
    }
}

void MonoServer::parseNotifyGUIQ()
{
    const auto fnGetTokenList = [](const std::string &cmdStr) -> std::deque<std::string>
    {
        size_t currLoc = 0;
        std::deque<std::string> tokenList;

        while(currLoc <= cmdStr.size()){
            const auto loc0 = cmdStr.find_first_not_of("\n", currLoc);
            const auto loc1 = cmdStr.find_first_of    ("\n", currLoc);

            if(loc0 == std::string::npos){
                // done parsing
                // there are no more tokens in the string
                break;
            }

            else if(loc1 == std::string::npos){
                // last one
                // loc0 is OK
                // loc1 reaches the end
                // this is the last token in the list
                tokenList.push_back(cmdStr.substr(loc0));
                break;
            }

            else if(loc0 < loc1){
                // match cases
                // make sure loc0 < loc1 to avoid cases like: "    OK 1"
                tokenList.push_back(cmdStr.substr(loc0, loc1 - loc0));
                currLoc = loc1;
                continue;
            }

            else{
                // case for loc0 > loc1, no equal here
                // cases like "   OK 1", take no token and move forward
                currLoc = loc0;
                continue;
            }
        }
        return tokenList;
    };

    while(true){
        std::string currNotify;
        {
            std::lock_guard<std::mutex> lockGuard(m_notifyGUILock);
            if(m_notifyGUIQ.empty()){
                break;
            }

            currNotify = std::move(m_notifyGUIQ.front());
            m_notifyGUIQ.pop();
        }

        const auto tokenList = fnGetTokenList(currNotify);
        if(tokenList.empty()){
            continue;
        }

        const auto fnCheckFront = [&tokenList](const std::vector<std::string> &keyList) -> bool
        {
            for(const auto &key: keyList){
                if(tokenList.front() == key){
                    return true;
                }
            }
            return false;
        };

        if(fnCheckFront({"exit", "Exit", "EXIT"})){
            std::exit(0);
            return;
        }

        if(fnCheckFront({"restart", "Restart", "RESTART"})){
            if(tokenList.size() == 1){
                fl_alert("Fatal error");
            }
            else{
                fl_alert("Fatal error: %s", tokenList.at(1).c_str());
            }
            std::exit(0);
            return;
        }

        if(fnCheckFront({"flushbrowser", "FlushBrowser", "FLUSHBROWSER"})){
            g_monoServer->FlushBrowser();
            continue;
        }

        if(fnCheckFront({"flushcwbrowser", "FlushCWBrowser", "FLUSHCWBROWSER"})){
            g_monoServer->FlushCWBrowser();
            continue;
        }

        if(fnCheckFront({"exitcw", "ExitCW", "EXITCW"})){
            const int cwid = [&tokenList]() -> int
            {
                try{
                    return std::stoi(tokenList.at(1));
                }catch(...){
                    //
                }
                return 0;
            }();

            if(cwid > 0){
                g_mainWindow->DeleteCommandWindow(cwid);
            }
            continue;
        }

        g_monoServer->addLog(LOGTYPE_WARNING, "Unsupported notification: %s", tokenList.front().c_str());
    }
}

void MonoServer::FlushBrowser()
{
    std::lock_guard<std::mutex> stLockGuard(m_logLock);
    {
        auto nCurrLoc = (size_t)(0);
        while(nCurrLoc < m_logBuf.size()){
            g_mainWindow->addLog(to_d(m_logBuf[nCurrLoc]), &(m_logBuf[nCurrLoc + 1]));
            nCurrLoc += (1 + 1 + std::strlen(&(m_logBuf[nCurrLoc + 1])));
        }
        m_logBuf.clear();
    }
}

void MonoServer::FlushCWBrowser()
{
    std::lock_guard<std::mutex> stLockGuard(m_CWLogLock);
    {
        // internal structure of CWLogBuf:
        // [xxxx][x][..0][....0][xxxx][x][..0][....0] ....
        //
        // [xxxx]  : CWID
        // [x]     : LogType: 0 ~ 2
        // [..0]   : szPrompt
        // [....0] : szLogInfo

        size_t nCurrLoc = 0;
        while(nCurrLoc < m_CWLogBuf.size()){
            uint32_t nCWID = 0;
            std::memcpy(&nCWID, &(m_CWLogBuf[nCurrLoc]), sizeof(nCWID));

            auto nInfoLen0 = std::strlen(&(m_CWLogBuf[nCurrLoc + sizeof(nCWID) + 1]));
            auto nInfoLen1 = std::strlen(&(m_CWLogBuf[nCurrLoc + sizeof(nCWID) + 1 + nInfoLen0 + 1]));

            auto pInfo0 = &(m_CWLogBuf[nCurrLoc + sizeof(nCWID) + 1]);
            auto pInfo1 = &(m_CWLogBuf[nCurrLoc + sizeof(nCWID) + 1 + nInfoLen0 + 1]);

            g_mainWindow->addCWLogString(nCWID, to_d(m_CWLogBuf[nCurrLoc + sizeof(nCWID)]), pInfo0, pInfo1);
            nCurrLoc += (sizeof(nCWID) + 1 + nInfoLen0 + 1 + nInfoLen1 + 1);
        }
        m_CWLogBuf.clear();
    }
}

uint64_t MonoServer::sleepExt(uint64_t tickCount)
{
    const auto enterTime = std::chrono::steady_clock::now();
    const auto excptTime = enterTime + std::chrono::milliseconds(tickCount);

    if(tickCount > 20){
        std::this_thread::sleep_for(std::chrono::milliseconds(tickCount - 10));
    }

    while(true){
        if(const auto currTime = std::chrono::steady_clock::now(); currTime >= excptTime){
            return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(currTime - enterTime).count());
        }
        else{
            std::this_thread::sleep_for((excptTime - currTime) / 2);
        }
    }
    return 0;
}

void MonoServer::regLuaExport(CommandLuaModule *modulePtr, uint32_t nCWID)
{
    if(!(modulePtr && nCWID)){
        throw fflerror("invalid argument: module = %p, window ID = %llu", to_cvptr(modulePtr), to_llu(nCWID));
    }

    // register command quit
    // exit current command window and free all related resource
    modulePtr->bindFunction("quit", [this, nCWID]()
    {
        // 1. show exiting messages
        addCWLogString(nCWID, 0, "> ", "Command window is requested to exit now...");

        // 2. flush command windows
        //    we need to flush message before exit the command window
        //    otherwise next created command window may get them if it uses the same CWID
        notifyGUI("FlushCWBrowser");
        notifyGUI(std::string("ExitCW\n") + std::to_string(nCWID));
    });

    // register command addCWLogString
    // print one line in command window, won't add message to log system
    modulePtr->bindFunction("addCWLogString", [this, nCWID](sol::object logType, sol::object prompt, sol::object logInfo)
    {
        if(true
                && logType.is<int>()
                && prompt.is<std::string>()
                && logInfo.is<std::string>()){
            addCWLogString(nCWID, logType.as<int>(), prompt.as<std::string>().c_str(), logInfo.as<std::string>().c_str());
            return;
        }

        // invalid argument provided
        addCWLogString(nCWID, 2, ">>> ", "addCWLogString(logType: int, prompt: string, logInfo: string)");
    });

    // register command countMonster(monsterID, mapID)
    modulePtr->bindFunction("countMonster", [this, nCWID](int nMonsterID, int nMapID) -> int
    {
        auto nRet = getMonsterCount(nMonsterID, nMapID).value_or(-1);
        if(nRet < 0){
            addCWLogString(nCWID, 2, ">>> ", "countMonster(MonsterID: int, mapID: int) failed");
        }
        return nRet;
    });

    // register command addMonster
    // will support add monster by monster name and map name
    // here we need to register a function to do the monster creation
    //      1. fnAddMonster(sol::variadic_args stVariadicArgs)
    //      2. fnAddMonster(int nMonsterID, int nMapID, sol::variadic_args stVariadicArgs)
    // I want to use method-1 in future, method-2 is more readable but can't handle the parameter issue:
    //      for i in mapList()
    //      do
    //          addMonster(i)
    //      end
    // here we get an exception from lua caught by sol2: ``std::bad_alloc"
    // but we want more detailed information like print the function usage out
    modulePtr->bindFunction("addMonster", [this, nCWID](int nMonsterID, int nMapID, sol::variadic_args stVariadicArgs) -> bool
    {
        auto fnPrintUsage = [this, nCWID]()
        {
            addCWLogString(nCWID, 2, ">>> ", "addMonster(MonsterID: int, mapID: int)");
            addCWLogString(nCWID, 2, ">>> ", "addMonster(MonsterID: int, mapID: int, X: int, Y: int)");
            addCWLogString(nCWID, 2, ">>> ", "addMonster(MonsterID: int, mapID: int, X: int, Y: int, Random: bool)");
        };

        std::vector<sol::object> stArgList(stVariadicArgs.begin(), stVariadicArgs.end());
        switch(stArgList.size()){
            case 0:
                {
                    return addMonster(to_u32(nMonsterID), to_u32(nMapID), -1, -1, false);
                }
            case 1:
                {
                    fnPrintUsage();
                    return false;
                }
            case 2:
                {
                    if(true
                            && stArgList[0].is<int>()
                            && stArgList[1].is<int>()){
                        return addMonster(to_u32(nMonsterID), to_u32(nMapID), stArgList[0].as<int>(), stArgList[1].as<int>(), false);
                    }
                    else{
                        fnPrintUsage();
                        return false;
                    }
                }
            case 3:
                {
                    if(true
                            && stArgList[0].is<int >()
                            && stArgList[1].is<int >()
                            && stArgList[2].is<bool>()){
                        return addMonster(to_u32(nMonsterID), to_u32(nMapID), stArgList[0].as<int>(), stArgList[1].as<int>(), stArgList[2].as<bool>());
                    }
                    else{
                        fnPrintUsage();
                        return false;
                    }
                }
            default:
                {
                    fnPrintUsage();
                    return false;
                }
        }
    });

    // register command mapList
    // return a table (userData) to lua for ipairs() check
    modulePtr->bindFunction("getMapIDList", [this](sol::this_state stThisLua)
    {
        return sol::make_object(sol::state_view(stThisLua), getMapList());
    });

    modulePtr->bindFunction("loadMap", [this](std::string mapName)
    {
        return loadMap(mapName);
    });

    modulePtr->bindFunction("getCWID", [nCWID]() -> int
    {
        return nCWID;
    });

    modulePtr->bindFunction("history", [nCWID, this]()
    {
        for(const auto &s: g_mainWindow->getCWHistory(nCWID)){
            if(!s.empty()){
                addCWLogString(nCWID, 0, "> ", s.c_str());
            }
        }
    });

    modulePtr->pfrCheck(modulePtr->execRawString(BEGIN_LUAINC(char)
#include "monoserver.lua"
    END_LUAINC()));
}
