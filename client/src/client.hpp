#pragma once
#include <atomic>
#include <string>
#include <type_traits>
#include <SDL2/SDL.h>

#include "netio.hpp"
#include "process.hpp"
#include "message.hpp"
#include "sdldevice.hpp"
#include "raiitimer.hpp"

class ProcessRun;
class Client final
{
    private:
        struct SMProcMonitor
        {
            uint64_t procTick  = 0;
            uint32_t recvCount = 0;
        };

        struct ClientMonitor
        {
            std::array<SMProcMonitor, SM_END> SMProcMonitorList;
            ClientMonitor()
                : SMProcMonitorList()
            {}
        };

    private:
        struct ResponseHandler
        {
            uint64_t timeout;
            std::function<void(uint8_t, const uint8_t *, size_t)> handler;
        };

    private:
        std::string m_token;

    private:
        ClientMonitor m_clientMonitor;

    private:
        hres_timer m_clientTimer;

    private:
        double m_serverDelay;
        double m_netPackTick;

    private:
        NetIO m_netIO;
        uint64_t m_respHandlerIndex = 1;
        std::unordered_map<uint64_t, ResponseHandler> m_respHandlers;

    private:
        int m_requestProcess = PROCESSID_NONE;
        std::unique_ptr<Process> m_currentProcess;

    private:
        std::optional<SMOnlineOK> m_smOOK;

    private:
        std::string m_clipboardBuf;

    public:
        Client();

    public:
        ~Client();

    public:
        void mainLoop();

    public:
        void Clipboard(const std::string &szInfo)
        {
            m_clipboardBuf = szInfo;
        }

        std::string Clipboard()
        {
            return m_clipboardBuf;
        }

    public:
        void requestProcess(int processID)
        {
            m_requestProcess = processID;
        }

    private:
        void switchProcess();
        void switchProcess(int);
        void switchProcess(int, int);

    public:
        void initASIO();

    private:
        void onServerMessage(uint8_t, const uint8_t *, size_t);

    private:
        void eventDelay(double);
        void processEvent();

    private:
        void draw()
        {
            if(m_currentProcess){
                m_currentProcess->draw();
            }
        }

        void update(double fUpdateTime)
        {
            if(m_currentProcess){
                m_currentProcess->update(fUpdateTime);
                switchProcess();
            }
        }

    public:
        Process *ProcessValid(int processID)
        {
            return (m_currentProcess && m_currentProcess->id() == processID) ? m_currentProcess.get() : nullptr;
        }

    public:
        template<typename T> auto getProcess() const
        {
            return m_currentProcess ? dynamic_cast<T *>(m_currentProcess.get()) : nullptr;
        }

    public:
        void setOnlineOK(const SMOnlineOK &smOOK)
        {
            m_smOOK = smOOK;
        }

    private:
        void sendCMsgLog(uint8_t);
        void sendSMsgLog(uint8_t);

    public:
        void send(uint8_t headCode, const void *buf, size_t bufSize, std::function<void(uint8_t, const uint8_t *, size_t)> fnOp = nullptr)
        {
            if(fnOp){
                if(m_respHandlers.empty()){
                    m_respHandlerIndex = 1;
                }

                m_respHandlers.emplace(m_respHandlerIndex, ResponseHandler{hres_tstamp::localtime() + 1000, std::move(fnOp)});
                m_netIO.send(headCode, static_cast<const uint8_t *>(buf), bufSize, m_respHandlerIndex);
                m_respHandlerIndex++;
            }
            else{
                m_netIO.send(headCode, static_cast<const uint8_t *>(buf), bufSize, 0);
            }

            sendCMsgLog(headCode);
        }

    public:
        void send(uint8_t headCode, std::function<void(uint8_t, const uint8_t *, size_t)> fnOp = nullptr)
        {
            send(headCode, nullptr, 0, std::move(fnOp));
        }

        void send(uint8_t headCode, const std::string &buf, std::function<void(uint8_t, const uint8_t *, size_t)> fnOp = nullptr)
        {
            send(headCode, buf.data(), buf.size(), std::move(fnOp));
        }

        void send(uint8_t headCode, const std::u8string &buf, std::function<void(uint8_t, const uint8_t *, size_t)> fnOp = nullptr)
        {
            send(headCode, buf.data(), buf.size(), std::move(fnOp));
        }

        template<typename T> void send(uint8_t headCode, const T &t, std::function<void(uint8_t, const uint8_t *, size_t)> fnOp = nullptr)
        {
            static_assert(std::is_trivially_copyable_v<T>);
            send(headCode, &t, sizeof(t), std::move(fnOp));
        }

    public:
        void PrintMonitor() const;

    public:
        void setToken(std::string token)
        {
            m_token = std::move(token);
        }

        const std::string &getToken() const
        {
            return m_token;
        }
};
