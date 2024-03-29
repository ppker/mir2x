#pragma once
#include <cstring>
#include <string>
#include <stdexcept>
#include <execinfo.h>
#include "strf.hpp"

#define fflerror(...) std::runtime_error(str_ffl() + ": " + str_printf(__VA_ARGS__))

inline std::string _fflerror_helper(size_t index)
{
    return str_printf("[%zu]: NA", index);
}

template<typename T> std::string _fflerror_helper(size_t index, const T & t)
{
    return str_printf("[%zu]: %s", index, str_any(t).c_str());
}

template<typename T, typename ... Args> std::string _fflerror_helper(size_t index, const T & t, Args && ... args)
{
    return _fflerror_helper(index, t) + ", " + _fflerror_helper(index + 1, std::forward<Args>(args)...);
}

template<typename ... Args> constexpr size_t _fflerror_count_helper(Args && ...)
{
    return sizeof...(Args);
}

#define fflreach() fflerror("fflreach")
#define fflvalue(...) fflerror("%s", _fflerror_helper(0, __VA_ARGS__).c_str())
#define fflassert(cond, ...) do{if(cond){}else{throw fflerror("assertion failed: %s%s", #cond, (_fflerror_count_helper(__VA_ARGS__) == 0) ? "" : (std::string(", ") + _fflerror_helper(0, ##__VA_ARGS__)).c_str());}}while(0)

// Dump stack trace:
// fnOutput: output function
//      fnOutput(-1, "fatal message")
//      fnOutput( i, "i-th stack trace")
// maxDump: max stack trace depth
inline void _ffl_dump_stack(const std::string &errloc, std::function<void(int, const char *)> fnOutput, int maxDump = 100)
{
    if(maxDump <= 0){
        fnOutput(-1, (errloc + str_printf("invalid dumpstack depth: %d", maxDump)).c_str());
        return;
    }

    void *sbuf[100];
    std::vector<void *> dbuf;

    auto buf = [&sbuf, &dbuf, maxDump]() -> void **
    {
        if(maxDump > 100){
            dbuf.resize(maxDump);
            return dbuf.data();
        }
        else{
            return sbuf;
        }
    }();

    const int nptrs = backtrace(buf, maxDump);
    if(nptrs < 0 || nptrs > maxDump){ // zero is possible when ARM platform doesn't compiled with -funwind-tables
        fnOutput(-1, (errloc + str_printf("backtrace() returns %d, max depth %d", nptrs, maxDump)).c_str());
        return;
    }

    const auto strings = backtrace_symbols(buf, nptrs);
    if(!strings){
        fnOutput(-1, (errloc + "backtrace_symbols() failed").c_str());
        return;
    }

    for(int j = 0; j < nptrs; ++j){
        fnOutput(j, strings[j]);
    }
    free(strings);
}

#define ffldumpstack(...) _ffl_dump_stack(str_ffl() + ": ", __VA_ARGS__)
