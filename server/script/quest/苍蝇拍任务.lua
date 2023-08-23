setQuestFSMTable(
{
    [SYS_ENTER] = function(uid, args)
        setupNPCQuestBehavior('比奇县_0', '金氏_1', uid,
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
                            <par>哎！这些该死的苍蝇，害得我没法儿做生意。要去重新买一个苍蝇拍吧，偏偏这个时候苍蝇拍材料没有了，啧...</par>
                            </par>啊！正好，你去杂货商那儿帮他找些苍蝇拍的材料，然后把做好的苍蝇拍带过来行吗？</par>
                            <par><event id="npc_refuse">我有点忙...</event></par>
                            <par><event id="npc_accept">您是说去杂货店吗？我去一趟吧！</event></par>
                        </layout>
                    ]=])
                end,

                npc_refuse = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>啊，这样啊...一小会儿就行的...唉，真是没辙了！</par>
                            <par><event id="%s">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,

                npc_accept = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>那就拜托您了！杂货店就是在右边能看到的那个地方。准确位置是<t color="red">450,413</t>。</par>
                            <par><event id="%s">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)
                end,
            }
        ]])
    end,
})
