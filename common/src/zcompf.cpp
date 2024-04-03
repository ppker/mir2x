#include "totype.hpp"
#include "zcompf.hpp"

size_t zcompf::countMask(const uint8_t *buf, size_t bufLen)
{
    if(bufLen > 0){
        fflassert(buf);
        return popcnt(buf, bufLen);
    }
    return 0;
}

size_t zcompf::countData(const uint8_t *buf, size_t bufLen)
{
    size_t count = 0;
    for(size_t i = 0; i < bufLen; ++i){
        if(buf[i]){
            count++;
        }
    }
    return count;
}

size_t zcompf::xorEncode(uint8_t *dst, const uint8_t *buf, size_t bufLen)
{
    fflassert(dst);
    fflassert(buf);
    fflassert(bufLen);

    const auto maskLen = (bufLen + 7) / 8;
    const auto maskPtr = dst;
    const auto compPtr = dst + maskLen;
    std::memset(maskPtr, 0, maskLen);

    size_t dataCount = 0;
    for(size_t i = 0; i < bufLen; ++i){
        if(buf[i]){
            maskPtr[i / 8] |= (0x01 << (i % 8));
            compPtr[dataCount++] = buf[i];
        }
    }
    return dataCount;
}

size_t zcompf::xorDecode(uint8_t *dst, size_t bufLen, const uint8_t *maskPtr, const uint8_t *compPtr)
{
    fflassert(dst);
    fflassert(bufLen);

    fflassert(maskPtr);
    fflassert(compPtr);

    size_t decodeCount = 0;
    for(size_t i = 0; i < bufLen; ++i){
        if(maskPtr[i / 8] & (0x01 << (i % 8))){
            dst[i] = compPtr[decodeCount++];
        }
        else{
            dst[i] = 0;
        }
    }
    return decodeCount;
}

size_t zcompf::countData64(const uint8_t *buf, size_t bufLen)
{
    size_t count = 0;
    for(size_t i = 0; i < bufLen; i += 8){
        if(as_u64(buf + i)){
            count++;
        }
    }
    return count;
}

size_t zcompf::xorEncode64(uint8_t *dst, const uint8_t *buf, size_t bufLen)
{
    fflassert(dst);
    fflassert(buf);
    fflassert(bufLen);

    const auto maskLen = (bufLen + 63) / 64;
    const auto maskPtr = dst;
    const auto compPtr = dst + maskLen;
    std::memset(maskPtr, 0, maskLen);

    size_t dataCount = 0;
    for(size_t i = 0; i < bufLen; i += 8){
        if(as_u64(buf + i)){
            maskPtr[i / 64] |= (0x01 << ((i % 64) / 8));
            std::memcpy(compPtr + dataCount * 8, buf + i, 8);
            dataCount++;
        }
    }
    return dataCount;
}

size_t zcompf::xorDecode64(uint8_t *dst, size_t bufLen, const uint8_t *maskPtr, const uint8_t *compPtr)
{
    fflassert(dst);
    fflassert(bufLen);

    fflassert(maskPtr);
    fflassert(compPtr);

    size_t decodeCount = 0;
    for(size_t i = 0; i < bufLen; i += 8){
        if(maskPtr[i / 64] & (0x01 << ((i % 64) / 8))){
            std::memcpy(dst + i, compPtr + decodeCount * 8, 8);
            decodeCount++;
        }
        else{
            std::memset(dst + i, 0, 8);
        }
    }
    return decodeCount;
}
