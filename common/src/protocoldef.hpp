#pragma once
#include <cstdint>
#include "motion.hpp"

// define of directioin
//
//               0
//            7     1
//          6    +--> 2
//            5  |  3
//               V
//               4
//
enum dir8_t: int
{
    DIR_NONE  = 0,
    DIR_BEGIN = 1,
    DIR_UP    = 1,
    DIR_UPRIGHT,
    DIR_RIGHT,
    DIR_DOWNRIGHT,
    DIR_DOWN,
    DIR_DOWNLEFT,
    DIR_LEFT,
    DIR_UPLEFT,
    DIR_END,
};

enum PathFindType: int
{
    PF_NONE  = 0, // grid outside of map
    PF_BEGIN = 1,

    PF_OBSTACLE = 1,
    PF_OCCUPIED,
    PF_LOCKED,
    PF_FREE,
    PF_END,
};

enum ActExtType: int
{
    ACTEXT_NONE = 0,

    ACTEXT_FLYIN,
    ACTEXT_FLYOUT,

    ACTEXT_FADEIN,
    ACTEXT_FADEOUT,
};

enum LookIDType: int
{
    LID_BEGIN = 0,
    LID_END   = LID_BEGIN + 2048,
};

enum EffectType: int
{
    EFF_NONE = 0,

    EFF_HPSTEAL = (1 << 8),
    EFF_MPSTEAL = (2 << 8),
    EFF_PUNCH   = (3 << 8),
    EFF_STONE   = (4 << 8),
};

enum GenType: int
{
    GT_NONE = 0,
    GT_MALE,
    GT_FEMALE,
};

enum MonsterAttackType: int
{
    MA_NONE = 0,
    MA_NORMAL,
    MA_DOGZ,
    MA_ATTACKALL,
};

enum WeaponIDType: int
{
    WEAPON_NONE  = 0,
    WEAPON_BEGIN = 1,
    WEAPON_END   = 256,
};

enum HelmetIDType: int
{
    HELMET_NONE  = 0,
    HELMET_BEGIN = 1,
    HELMET_END   = 256,
};

enum DressIDType : int
{
    DRESS_NONE  = -1,
    DRESS_BEGIN =  0,   // zero uses naked gfx
    DRESS_END   =  256,
};

enum HairIDType: int
{
    HAIR_NONE  = 0,
    HAIR_BEGIN = 1,
    HAIR_END   = 256,
};

enum NPCErrorType: int
{
    NPCE_NONE = 0,
    NPCE_TOOFAR,
    NPCE_BADEVENTID,
};

enum JobType: int
{
    JOB_NONE    = 0,
    JOB_BEGIN   = 1,
    JOB_WARRIOR = 1 << 0,
    JOB_TAOIST  = 1 << 1,
    JOB_WIZARD  = 1 << 2,
    JOB_END     = 1 << 3,
};

// keep it POD
// used in actor/server/client message
// this only includes player look to draw, no necklace, rings etc
struct PlayerLook
{
    uint32_t hair;
    uint32_t hairColor;

    uint32_t helmet;
    uint32_t helmetColor;

    uint32_t dress;
    uint32_t dressColor;

    uint32_t weapon;
    uint32_t weaponColor;
};

struct PlayerWear
{
    uint32_t necklace;
    uint32_t armring[2];
    uint32_t ring[2];

    uint32_t shoes;
    uint32_t torch;
    uint32_t charm;
};

enum WearLookGridType: int
{
    WLG_NONE  = 0,
    WLG_BEGIN = 1,

    WLG_L_BEGIN = WLG_BEGIN,
    WLG_DRESS   = WLG_L_BEGIN,
    WLG_HELMET,
    WLG_WEAPON,
    WLG_L_END,

    WLG_W_BEGIN = WLG_L_END,
    WLG_SHOES   = WLG_W_BEGIN,
    WLG_NECKLACE,
    WLG_ARMRING0,
    WLG_ARMRING1,
    WLG_RING0,
    WLG_RING1,
    WLG_TORCH,
    WLG_CHARM,
    WLG_W_END,

    WLG_END = WLG_W_END,
};

enum AttributeType: int
{
    ATR_NONE  = 0,
    ATR_BEGIN = 1,
    ATR_DC    = 1,
    ATR_MC,
    ATR_SC,
    ATR_AC,
    ATR_MAC,
    ATR_HP,
    ATR_MP,
    ATR_HPR,
    ATR_MPR,
    ATR_HIT,
    ATR_SPD,
    ATR_END,
};

enum BuyErrorType: int
{
    BUYERR_NONE    = 0,
    BUYERR_BEGIN   = 1,
    BUYERR_BADITEM = 1,
    BUYERR_LOCKED,
    BUYERR_SOLDOUT,
    BUYERR_INSUFFCIENT,
    BUYERR_END,
};

enum EquipWearErrorType: int
{
    EQWERR_NONE = 0,
    EQWERR_BEGIN = 1,
    EQWERR_NOITEM = 1,
    EQWERR_BADITEM,
    EQWERR_BADWLTYPE,
    EQWERR_INSUFF,
    EQWERR_END,
};

enum GrabWearErrorType: int
{
    GWERR_NONE = 0,
    GWERR_BEGIN = 1,
    GWERR_NOITEM = 1,
    GWERR_BIND, // some item you can't manually remove it after wear
    GWERR_END,
};

enum EquipBeltErrorType: int
{
    EQBERR_NONE = 0,
    EQBERR_BEGIN = 1,
    EQBERR_NOITEM = 1,
    EQBERR_BADITEM,
    EQBERR_BADITEMTYPE,
    EQBERR_BADSLOT,
    EQBERR_END,
};

enum GrabBeltErrorType: int
{
    GBERR_NONE = 0,
    GBERR_BEGIN = 1,
    GBERR_NOITEM = 1,
    GBERR_END,
};

enum CreateAccountErrorType: int
{
    CRTACCERR_NONE         = 0,
    CRTACCERR_BEGIN        = 1,
    CRTACCERR_ACCOUNTEXIST = 1,
    CRTACCERR_BADACCOUNT,
    CRTACCERR_BADPASSWORD,
    CRTACCERR_END,
};

enum ChangePasswordErrorType: int
{
    CHGPWDERR_NONE       = 0,
    CHGPWDERR_BEGIN      = 1,
    CHGPWDERR_BADACCOUNT = 1,       // bad      account for general check
    CHGPWDERR_BADPASSWORD,          // bad     password for general check
    CHGPWDERR_BADNEWPASSWORD,       // bad new password for general check
    CHGPWDERR_BADACCOUNTPASSWORD,   // can not find (account, password) in database
    CHGPWDERR_END,
};

enum LoginErrorType: int
{
    LOGINERR_NONE      = 0,
    LOGINERR_BEGIN     = 1,
    LOGINERR_NOACCOUNT = 1,
    LOGINERR_MULTILOGIN,
    LOGINERR_END,
};

enum CreateCharErrorType: int
{
    CRTCHARERR_NONE    = 0,
    CRTCHARERR_BEGIN   = 1,
    CRTCHARERR_NOLOGIN = 1,     //
    CRTCHARERR_CHAREXIST,       // current account has char created
    CRTCHARERR_NAMEEXIST,       // name has been used by another existing char
    CRTCHARERR_BADNAME,         // name not allowed
    CRTCHARERR_DBERROR,         // sqlite3 error when creating player
    CRTCHARERR_END,
};

enum QueryCharErrorType: int
{
    QUERYCHARERR_NONE    = 0,
    QUERYCHARERR_BEGIN   = 1,
    QUERYCHARERR_NOLOGIN = 1,
    QUERYCHARERR_NOCHAR,
    QUERYCHARERR_END,
};

enum DeleteCharErrorType: int
{
    DELCHARERR_NONE    = 0,
    DELCHARERR_BEGIN   = 1,
    DELCHARERR_NOLOGIN = 1,
    DELCHARERR_NOCHAR,
    DELCHARERR_BADPASSWORD,
    DELCHARERR_END,
};

enum OnlineErrorTpe: int
{
    ONLINEERR_NONE  = 0,
    ONLINEERR_BEGIN = 1,
    ONLINEERR_NOLOGIN,
    ONLINEERR_MULTIONLINE,
    ONLINEERR_NOCHAR,
    ONLINEERR_AMERROR,
    ONLINEERR_END,
};

enum BuffType: int
{
    BFT_NONE   = 0,
    BFT_BEGIN  = 1,
    BFT_SHIELD = BFT_BEGIN,
    BFT_END,
};

enum BuffStateType: int
{
    BFS_NONE  = 0,
    BFS_BEGIN = 1,
    BFS_ON    = BFS_BEGIN,
    BFS_OFF,
    BFS_END,
};

enum InvOpType: int
{
    INVOP_NONE  = 0,
    INVOP_BEGIN = 1,
    INVOP_TRADE = INVOP_BEGIN,
    INVOP_SECURE,
    INVOP_REPAIR,
    INVOP_END,
};

enum AttackModeType: int
{
    ATKMODE_NONE  = 0,
    ATKMODE_BEGIN = 1,

    ATKMODE_PEACE = ATKMODE_BEGIN,
    ATKMODE_GROUP,
    ATKMODE_GUILD,
    ATKMODE_ALL,
    ATKMODE_END,
};

enum TeamErrorType: int
{
    TEAMERR_NONE  = 0,
    TEAMERR_BEGIN = 1,
    TEAMERR_INTEAM,
    TEAMERR_END,
};

enum AddFriendResultType: int
{
    AF_NONE     = 0,
    AF_BEGIN    = 1,
    AF_ACCEPTED = 1,    // manually/automatically accepted
    AF_REJECTED,        // manually/automatically rejected
    AF_PENDING,         // waiting response
    AF_EXIST,           // has already been your friend
    AF_END,
};

enum ChatPeerType: int
{
    CP_NONE    = 0,
    CP_BEGIN   = 1,
    CP_SPECIAL = 1,
    CP_PLAYER,
    CP_GROUP,
    CP_END,
};

template<typename... Ts> struct VarDispatcher: Ts...
{
    using Ts::operator()...;
};

template<typename T> class ValueKeeper final
{
    private:
        T &m_ref;
        T  m_oldValue;

    public:
        template<typename K> ValueKeeper(T &keep, K &&k)
            : m_ref(keep)
            , m_oldValue(keep)
        {
            m_ref = k;
        }

        ~ValueKeeper()
        {
            m_ref = m_oldValue;
        }
};
