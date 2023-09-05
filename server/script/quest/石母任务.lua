uidRemoteCall(getNPCharUID('比奇县_0_003', '石母_1'), getUID(), getQuestName(),
[[
    local questUID, questName = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_ENTER] = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>这到底是怎么回事？那就是说女人回来了？</par>
                    <par><event id="%s">退出</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,
    })
]])

uidRemoteCall(getNPCharUID('比奇县_0', '母子石像_1'), getUID(), getQuestName(),
[[
    local questUID, questName = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_CHECKACTIVE] = false,
        [SYS_ENTER] = function(uid, args)
            plyapi.spaceMove(uid, '比奇县_0_003', 28, 35)
            npcapi.runHandler(getNPCharUID('比奇县_0_003', '石母_1'), uid, {SYS_EPQST, questName}, SYS_ENTER)
        end,
    })

    addTrigger(SYS_ON_APPEAR, function(uid)
        if not isPlayer(uid) then
            return
        end

        uidPostXML(uid, questPath,
        [=[
            <layout>
                <par>这石头的样子真奇怪...</par>
                <par><event id="%s">过去看看？</event></par>
                <par><event id="%s">感觉很奇怪，我还是离远点比较好...</event></par>
            </layout>
        ]=], SYS_ENTER, SYS_EXIT)
    end)
]])

