setQuestFSMTable(
{
    [SYS_ENTER] = function(uid, args)
        setupNPCQuestBehavior('比奇县_0', '世玉_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPQST, questName}

            return
            {
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>对不起，本店不接受这种证书。</par>
                            <par></par>
                            <par><event id="npc_ask">为什么？</event></par>
                        </layout>
                    ]=])
                end,

                npc_ask = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>看来你不了解给你证书的人。洪气霖一生坎坷，他家经营的店铺原本在比奇县一带很有名望，但自从他们家的货船失事以后，他们家族就没落了。虽然我知道洪家的处境很艰难，但我们也不能因此而蒙受损失啊，所以我们不能接受这种证书。</par>
                            <par></par>
                            <par><event id="npc_who_accept">那这证书怎么办？</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                npc_who_accept = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>嗯...装饰品店所蒙受的损失少一点，说不定他们会接受。</par>
                            <par></par>
                            <par><event id="%s">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    qstapi.setState(questUID, {uid=uid, state=SYS_ENTER})
                end,
            }
        ]])
    end,
})

uidRemoteCall(getNPCharUID('比奇县_0', '世玉_1'), getUID(), getQuestName(),
[[
    local questUID, questName, minQuestLevel = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_CHECKACTIVE] = function(uid)
            if not plyapi.hasItem(uid, getItemID('气霖证书'), 0, 1) then
                return false
            end

            return qstapi.getState(questUID, {uid=uid, fsm=SYS_QSTFSM}) == nil
        end,

        [SYS_ENTER] = function(uid, args)
            qstapi.setState(questUID, {uid=uid, state=SYS_ENTER})
        end,
    })
]])
