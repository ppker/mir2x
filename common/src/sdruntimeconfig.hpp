#pragma once
#include <tuple>
#include <string>
#include <string_view>
#include <unordered_map>
#include <cerealf.hpp>

class SDRuntimeConfig
{
    private:
        std::unordered_map<int, std::string> m_config;

    public:
        template<typename Archive> void serialize(Archive & ar)
        {
            ar(m_config);
        }

    public:
        template<int            > friend auto SDRuntimeConfig_getConfig(const SDRuntimeConfig &           );
        template<int, typename T> friend bool SDRuntimeConfig_setConfig(      SDRuntimeConfig &, const T &);

    public:
        bool setConfig(int key, const void *data, size_t length)
        {
            if(auto p = m_config.find(key); p == m_config.end()){
                m_config.emplace(key, std::string((char *)(data), length));
                return true;
            }
            else if(p->second != std::string_view((char *)(data), length)){
                p->second = std::string((char *)(data), length);
                return true;
            }
            else{
                return false;
            }
        }

        bool setConfig(int key, const std::string_view &s)
        {
            return setConfig(key, s.data(), s.size());
        }

        bool setConfig(int key, const std::string &s)
        {
            return setConfig(key, s.data(), s.size());
        }

    public:
        const std::optional<std::string> getConfig(int key) const
        {
            if(auto p = m_config.find(key); p != m_config.end()){
                return p->second;
            }
            return std::nullopt;
        }
};

constexpr int RTCFG_NONE  = 0;
constexpr int RTCFG_BEGIN = 1;

constexpr int _RSVD_rtcfg_add_type_counter_begin = __COUNTER__;
template<int            > auto SDRuntimeConfig_getConfig(const SDRuntimeConfig &           );
template<int, typename T> bool SDRuntimeConfig_setConfig(      SDRuntimeConfig &, const T &);

#define _MACRO_ADD_RTCFG_TYPE(rtCfgKeyType, rtCfgValueType) \
    constexpr int rtCfgKeyType = __COUNTER__ - _RSVD_rtcfg_add_type_counter_begin; \
    \
    template<> inline auto SDRuntimeConfig_getConfig<rtCfgKeyType>(const SDRuntimeConfig &rtCfg) \
    { \
        if(auto p = rtCfg.m_config.find(rtCfgKeyType); p != rtCfg.m_config.end()){ \
            return std::make_optional<rtCfgValueType>(cerealf::deserialize<rtCfgValueType>(p->second)); \
        } \
        else{ \
            return std::optional<rtCfgValueType>(); \
        } \
    } \
    \
    template<> inline bool SDRuntimeConfig_setConfig<rtCfgKeyType, rtCfgValueType>(SDRuntimeConfig &rtCfg, const rtCfgValueType &value) \
    { \
        return rtCfg.setConfig(rtCfgKeyType, cerealf::serialize<rtCfgValueType>(value)); \
    } \

    /**/ // begin of runtime config types
    /**/ // each line provides two definitions, take RTCFG_IME as example:
    /**/ //
    /**/ //     RTCFG_IME           : an integer used as key
    /**/ //     RTCFG_IME_t         : an type contains: RTCFG_IME_t::value and RTCFG_IME_t::type
    /**/ //
    /**/ // don't put any other code except the macro defines and type aligns
    /**/
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_IME, bool)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_BGM, bool)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_BGMVALUE, float)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_SEFF, bool)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_SEFFVALUE, float)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_SHOWFPS, bool)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_FULLSCREEN, bool)
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_ATTACKMODE, int)
    /**/
    /**/ using _RSVD_helper_type_RTCFG_WINDOWSIZE_t = std::pair<int, int>;
    /**/ _MACRO_ADD_RTCFG_TYPE(RTCFG_WINDOWSIZE, _RSVD_helper_type_RTCFG_WINDOWSIZE_t)
    /**/
    /**/ // end of runtime config types
    /**/ // any config types should be put inside above region

#undef _MACRO_ADD_RTCFG_TYPE
constexpr int RTCFG_END = __COUNTER__ - _RSVD_rtcfg_add_type_counter_begin;
