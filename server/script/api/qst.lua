local qst = {}

function qst.getQuestName(questUID)
    return uidRemoteCall(questUID, [[ return getQuestName() ]])
end

function qst.getQuestState(questUID, playerUID, fsmName)
    return uidRemoteCall(questUID, playerUID, fsmName,
    [[
        local playerUID, fsmName = ...
        return dbGetUIDQuestState(playerUID, fsmName)
    ]])
end

return qst
