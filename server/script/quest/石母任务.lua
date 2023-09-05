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
                    <par>这位侠客，请一定要帮帮我啊！</par>
                    <par><event id="npc_exit">没听见没听见，我走了！</event></par>
                    <par><event id="npc_ask">什么事啊？</event></par>
                </layout>
            ]=])
        end,

        npc_exit = function(uid, args)
            local x, y = uidRemoteCall(uid, [=[ return getMapLoc() ]=])
            plyapi.spaceMove(uid, '比奇县_0', x + 490, y + 360)
        end,

        npc_extra = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>这到底是怎么回事？那就是说女人回来了？</par>
                    <par><event id="%s">问一下这个来路不明的女人！</event></par>
                    <par><event id="%s">还是算了！</event></par>
                </layout>
            ]=], SYS_ENTER, SYS_EXIT)
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
            npcapi.runHandler(getNPCharUID('比奇县_0_003', '石母_1'), uid, {SYS_EPQST, questName}, 'npc_extra')
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

