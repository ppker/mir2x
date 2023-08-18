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

    public:
        ServerLuaObject(uint32_t);

    public:
        virtual ~ServerLuaObject() = default;
};
