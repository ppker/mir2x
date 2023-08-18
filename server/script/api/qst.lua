local qst = {}

function qst.getQuestName(uid) return uidRemoteCall(uid, [[ return getQuestName() ]]) end

return qst
