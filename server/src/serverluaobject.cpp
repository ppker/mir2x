#include "serverobject.hpp"
#include "serverluaobject.hpp"

ServerLuaObject::ServerObjectLuaThreadRunner::ServerObjectLuaThreadRunner(ServerLuaObject *serverLuaObjectPtr)
    : ServerLuaCoroutineRunner(serverLuaObjectPtr->m_actorPod)
{
    fflassert(dynamic_cast<ServerLuaObject *>(m_actorPod->getSO()));
    fflassert(dynamic_cast<ServerLuaObject *>(m_actorPod->getSO()) == serverLuaObjectPtr);
}

ServerLuaObject::ServerLuaObject(uint32_t luaObjIndex)
    : ServerObject(uidf::getServerLuaObjectUID(luaObjIndex))
{}

void ServerLuaObject::onActivate()
{
    ServerObject::onActivate();
    m_luaRunner = std::make_unique<ServerObjectLuaThreadRunner>(this);
}
