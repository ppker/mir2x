#include "netio.hpp"
#include "totype.hpp"
#include "zcompf.hpp"
#include "message.hpp"
#include "fflerror.hpp"

void NetIO::afterReadPacketHeadCode()
{
    switch(m_serverMsg->type()){
        case 0:
            {
                m_msgHandler(m_serverMsg->headCode(), nullptr, 0, m_respIDOpt.value_or(0));
                doReadPacketHeadCode();
                return;
            }
        case 1:
        case 3:
            {
                doReadPacketBodySize(0);
                return;
            }
        case 2:
            {
                doReadPacketBody(0, m_serverMsg->dataLen());
                return;
            }
        default:
            {
                throw fflvalue(m_serverMsg->name());
            }
    }
}

void NetIO::doReadPacketHeadCode()
{
    asio::async_read(m_socket, asio::buffer(m_readSBuf, 1), [this](std::error_code ec, size_t)
    {
        if(ec){
            throw fflerror("network error: %s", ec.message().c_str());
        }

        m_respIDOpt.reset();
        prepareServerMsg(m_readSBuf[0]);

        if(m_serverMsg->hasResp()){
            doReadPacketResp(0);
        }
        else{
            afterReadPacketHeadCode();
        }
    });
}

void NetIO::doReadPacketResp(size_t offset)
{
    asio::async_read(m_socket, asio::buffer(m_readSBuf + offset, 1), [offset, this](std::error_code ec, size_t)
    {
        if(ec){
            throw fflerror("network error: %s", ec.message().c_str());
        }

        if(m_readSBuf[offset] & 0x80){
            if(offset + 1 >= (64 + 6) / 7){ // support 64 bits respID
                throw fflerror("variant packet size uses more than %zu bytes", offset + 1);
            }
            else{
                doReadPacketResp(offset + 1);
            }
        }
        else{
            uint64_t respID = 0;
            for(size_t i = 0; i <= offset; ++i){
                respID = (respID << 7) | (m_readSBuf[offset - i] & 0x7f);
            }

            if(!respID){
                throw fflerror("packet has response encoded but no response id provided");
            }

            m_respIDOpt = respID;
            afterReadPacketHeadCode();
        }
    });
}

void NetIO::doReadPacketBodySize(size_t offset)
{
    switch(m_serverMsg->type()){
        case 1:
            {
                asio::async_read(m_socket, asio::buffer(m_readSBuf + offset, 1), [offset, this](std::error_code ec, size_t)
                {
                    if(ec){
                        throw fflerror("network error: %s", ec.message().c_str());
                    }

                    if(m_readSBuf[offset] & 0x80){
                        if(offset + 1 >= 4){
                            throw fflerror("variant packet size uses more than 4 bytes");
                        }
                        else{
                            doReadPacketBodySize(offset + 1);
                        }
                    }
                    else{
                        uint32_t dataSize = 0;
                        for(size_t i = 0; i <= offset; ++i){
                            dataSize = (dataSize << 7) | (m_readSBuf[offset - i] & 0x7f);
                        }

                        doReadPacketBody(m_serverMsg->maskLen(), dataSize);
                    }
                });
                return;
            }
        case 3:
            {
                asio::async_read(m_socket, asio::buffer(m_readSBuf, 4), [this](std::error_code ec, size_t)
                {
                    if(ec){
                        throw fflerror("network error: %s", ec.message().c_str());
                    }
                    doReadPacketBody(0, as_u32(m_readSBuf));
                });
                return;
            }
        case 0:
        case 2:
        default:
            {
                throw fflvalue(m_serverMsg->name());
            }
    }
}

void NetIO::doReadPacketBody(size_t maskSize, size_t bodySize)
{
    fflassert(m_serverMsg->checkDataSize(maskSize, bodySize));
    switch(m_serverMsg->type()){
        case 1:
            {
                m_readDBuf.resize(maskSize + bodySize + 64 + m_serverMsg->dataLen());
                break;
            }
        case 2:
            {
                m_readDBuf.resize(m_serverMsg->dataLen());
                break;
            }
        case 3:
            {
                m_readDBuf.resize(bodySize);
                break;
            }
        case 0:
        default:
            {
                throw fflvalue(m_serverMsg->name());
            }
    }

    if(const auto totalSize = maskSize + bodySize){
        asio::async_read(m_socket, asio::buffer(m_readDBuf.data(), totalSize), [maskSize, bodySize, this](std::error_code ec, size_t)
        {
            if(ec){
                throw fflerror("network error: %s", ec.message().c_str());
            }

            if(maskSize){
                const bool useWide = m_serverMsg->useXor64();
                const size_t dataCount = zcompf::xorCountMask(m_readDBuf.data(), maskSize);

                fflassert(dataCount == (useWide ? (bodySize + 7) / 8 : bodySize));
                fflassert(bodySize <= m_serverMsg->dataLen());

                const auto maskDataPtr = m_readDBuf.data();
                const auto compDataPtr = m_readDBuf.data() + maskSize;
                /* */ auto origDataPtr = m_readDBuf.data() + ((maskSize + bodySize + 7) / 8) * 8;

                const auto decodedBytes = useWide ? zcompf::xorDecode64(origDataPtr, m_serverMsg->dataLen(), maskDataPtr, compDataPtr)
                                                  : zcompf::xorDecode  (origDataPtr, m_serverMsg->dataLen(), maskDataPtr, compDataPtr);

                fflassert(decodedBytes == bodySize);
            }

            m_msgHandler(m_serverMsg->headCode(), m_readDBuf.data() + (maskSize ? ((maskSize + bodySize + 7) / 8 * 8) : 0), maskSize ? m_serverMsg->dataLen() : bodySize, m_respIDOpt.value_or(0));
            doReadPacketHeadCode();
        });
    }
    else{
        m_msgHandler(m_serverMsg->headCode(), nullptr, 0, m_respIDOpt.value_or(0));
        doReadPacketHeadCode();
    }
}

void NetIO::doSendBuf()
{
    if(m_currSendBuf.empty()){
        return;
    }

    asio::async_write(m_socket, asio::buffer(m_currSendBuf.data(), m_currSendBuf.size()), [this](std::error_code ec, size_t)
    {
        if(ec){
            throw fflerror("network error: %s", ec.message().c_str());
        }

        m_currSendBuf.clear();
        if(m_nextSendBuf.empty()){
            return;
        }

        std::swap(m_currSendBuf, m_nextSendBuf);
        doSendBuf();
    });
}

void NetIO::send(uint8_t headCode, const uint8_t *buf, size_t bufSize, uint64_t respID)
{
    if(headCode & 0x80){
        throw fflerror("invalid head code 0x%02d", to_d(headCode));
    }

    const ClientMsg cmsg(headCode);
    fflassert(cmsg.checkData(buf, bufSize));

    if(respID){
        uint8_t respBuf[16];
        const size_t respEncSize = msgf::encodeLength(respBuf, 16, respID);

        m_nextSendBuf.push_back(headCode | 0x80);
        m_nextSendBuf.insert(m_nextSendBuf.end(), respBuf, respBuf + respEncSize);
    }
    else{
        m_nextSendBuf.push_back(headCode);
    }

    switch(cmsg.type()){
        case 0:
            {
                break;
            }
        case 1:
            {
                const bool useWide = cmsg.useXor64();
                const size_t dataCount = useWide ? zcompf::xorCountData64(buf, bufSize) : zcompf::xorCountData(buf, bufSize);

                uint8_t encodeBuf[4];
                const size_t sizeEncLength = msgf::encodeLength(encodeBuf, 4, dataCount);
                m_nextSendBuf.insert(m_nextSendBuf.end(), encodeBuf, encodeBuf + sizeEncLength);

                const size_t dataStartOff = m_nextSendBuf.size();
                m_nextSendBuf.resize(m_nextSendBuf.size() + cmsg.maskLen() + cmsg.dataLen());

                const size_t encodedBytes = useWide ? zcompf::xorEncode64(m_nextSendBuf.data() + dataStartOff, buf, bufSize)
                                                    : zcompf::xorEncode  (m_nextSendBuf.data() + dataStartOff, buf, bufSize);

                fflassert(encodedBytes == dataCount);
                m_nextSendBuf.resize(dataStartOff + cmsg.maskLen() + encodedBytes);
                break;
            }
        case 2:
            {
                m_nextSendBuf.insert(m_nextSendBuf.end(), buf, buf + bufSize);
                break;
            }
        case 3:
            {
                fflassert(bufSize <= 0xffffffff, bufSize);
                const uint32_t bufSizeU32 = bufSize;
                const   size_t dataStartOff = m_nextSendBuf.size();

                m_nextSendBuf.resize(m_nextSendBuf.size() + bufSize + 4);

                std::memcpy(m_nextSendBuf.data() + dataStartOff, &bufSizeU32, 4);
                std::memcpy(m_nextSendBuf.data() + dataStartOff + 4, buf, bufSize);
                break;
            }
        default:
            {
                throw fflvalue(cmsg.name());
            }
    }

    if(m_currSendBuf.empty()){
        std::swap(m_currSendBuf, m_nextSendBuf);
        doSendBuf();
    }
}

void NetIO::start(const char *ipStr, const char *portStr, std::function<void(uint8_t, const uint8_t *, size_t, uint64_t)> msgHandler)
{
    fflassert(str_haschar(  ipStr));
    fflassert(str_haschar(portStr));

    fflassert(msgHandler);
    m_msgHandler = std::move(msgHandler);

    asio::async_connect(m_socket, m_resolver.resolve({ipStr, portStr}), [this](std::error_code ec, asio::ip::tcp::resolver::iterator)
    {
        if(ec){
            throw fflerror("network error: %s", ec.message().c_str());
        }
        else{
            doReadPacketHeadCode();
        }
    });
}
