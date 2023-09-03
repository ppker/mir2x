#pragma once
#include "serverluacoroutinerunner.hpp"

class ServerObject;
class ServerObjectLuaThreadRunner: public ServerLuaCoroutineRunner
{
    public:
        ServerObjectLuaThreadRunner(ServerObject *);

    public:
        ServerObject *getSO() const
        {
            return m_actorPod->getSO();
        }
};
