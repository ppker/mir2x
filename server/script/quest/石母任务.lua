setQuestFSMTable(
{
    [SYS_ENTER] = function(uid, args)
        setupNPCQuestBehavior('比奇县_0_003', '石母_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>还没有找到我孩子吗？</par>
                            <par><event id="%s">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])
    end,
})

uidRemoteCall(getNPCharUID('比奇县_0_003', '石母_1'), getUID(), getQuestName(),
[[
    local questUID, questName = ...
    local questPath = {SYS_EPQST, questName}

    local fnLeaveMap = function(uid)
        local x, y = uidRemoteCall(uid, [=[ return getMapLoc() ]=])
        plyapi.spaceMove(uid, '比奇县_0', x + 490, y + 360)
    end

    setQuestHandler(questName,
    {
        [SYS_ENTER] = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>这位侠客，请一定要帮帮我啊！</par>
                    <par><event id="npc_leave">假装没听见，我走了！</event></par>
                    <par><event id="npc_ask">什么事啊？</event></par>
                </layout>
            ]=])
        end,

        npc_leave = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>请一定要帮帮我...啊？太无情了！</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]=], SYS_EXIT)

            fnLeaveMap(uid)
        end,

        npc_ask = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>呜呜...不久前我的孩子在夜市被商人给抢走了。一定要帮我找回孩子啊！</par>
                    <par>不知道他把孩子带走到底想干什么...一定要...拜托您了！</par>
                    <par><event id="npc_accept">知道了，我会去帮你找回孩子的！</event></par>
                    <par><event id="npc_refuse">我没有这闲工夫。</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        npc_accept = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>多谢了！听说夜市的商人在(452,297)，请一定要帮我找回孩子啊！</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]=], SYS_EXIT)

            fnLeaveMap(uid)
            qstapi.setState(questUID, {uid=uid, state=SYS_ENTER})
        end,

        npc_refuse = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>这样啊...呜呜...天下之大，竟然没有同情失去孩儿母亲心的侠客吗？...</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]=], SYS_EXIT)

            fnLeaveMap(uid)
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
