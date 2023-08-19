#include <serverobject.hpp>
#include <serverluacoroutinerunner.hpp>

class ServerLuaObject: public ServerObject
{
    protected:
        class ServerObjectLuaThreadRunner final: public ServerLuaCoroutineRunner
        {
            private:
                using LuaThreadHandle = ServerLuaCoroutineRunner::LuaThreadHandle;

            public:
                ServerObjectLuaThreadRunner(ServerLuaObject *);

            public:
                ServerLuaObject *getServerLuaObject() const
                {
                    return static_cast<ServerLuaObject *>(m_actorPod->getSO());
                }
        };

    private:
        std::unique_ptr<ServerObjectLuaThreadRunner> m_luaRunner;

    private:
        uint64_t m_threadKey = 1;

    public:
        ServerLuaObject(uint32_t);

    protected:
        void onActivate() override;

    protected:
        void operateAM(const ActorMsgPack &) override;

    protected:
        void on_AM_METRONOME (const ActorMsgPack &);
        void on_AM_REMOTECALL(const ActorMsgPack &);
};
