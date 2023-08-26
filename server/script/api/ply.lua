local ply = {}

function ply.getLevel(uid) return uidRemoteCall(uid, [[ return getLevel() ]]) end
function ply.getGold (uid) return uidRemoteCall(uid, [[ return getGold () ]]) end
function ply.getGener(uid) return uidRemoteCall(uid, [[ return getGener() ]]) end
function ply.getName (uid) return uidRemoteCall(uid, [[ return getName () ]]) end

function ply.getTeamLeader    (uid) return uidRemoteCall(uid, [[ return getTeamLeader    () ]]) end
function ply.getTeamMemberList(uid) return uidRemoteCall(uid, [[ return getTeamMemberList() ]]) end

function ply.dbHasFlag   (uid, flag) return uidRemoteCall(uid, flag, [[ return dbHasFlag   (...) ]]) end
function ply.dbAddFlag   (uid, flag) return uidRemoteCall(uid, flag, [[ return dbAddFlag   (...) ]]) end
function ply.dbRemoveFlag(uid, flag) return uidRemoteCall(uid, flag, [[ return dbRemoveFlag(...) ]]) end

function ply.postString(uid, msg, ...)
    local args = table.pack(...)
    uidRemoteCall(uid, msg, args,
    [[
        local msg, args = ...
        return postString(msg, table.unpack(args, 1, args.n))
    ]])
end

function ply.addItem(uid, item, count)
    return uidRemoteCall(uid, item, count,
    [[
        local item, count = ...
        return addItem(item, count)
    ]])
end

function ply.removeItem(uid, item, seq, count)
    return uidRemoteCall(uid, item, seq, count,
    [[
        local item, seq, count = ...
        return removeItem(item, seq, count)
    ]])
end

function ply.hasItem(uid, item, seq, count)
    return uidRemoteCall(uid, item, seq, count,
    [[
        local item, seq, count = ...
        return hasItem(item, seq, count)
    ]])
end

function ply.getQuestState(uid, ...)
    return uidRemoteCall(uid, table.pack(...),
    [[
        local args = ...
        return getQuestState(table.unpack(args, 1, args.n))
    ]])
end

function ply.addTrigger(playerUID, triggerType, funcStr)
    assertType(playerUID, 'integer')
    assert(isPlayer(playerUID))

    assertType(triggerType, 'integer')
    assertType(funcStr, 'string')

    return uidRemoteCall(playerUID, triggerType, funcStr,
    [[
        local triggerType, funcStr = ...
        return addTrigger(triggerType, load(funcStr))
    ]])
end

return ply
