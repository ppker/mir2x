uidExecute(getNPCharUID('比奇县_0', '王大人_1'), getQuestName(),
[[
    local questName = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_ENTER] = function(uid, value)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>这是一个测试</par>
                    <par><event id="%s">退出</event></par>
                </layout>
            ]=], SYS_EXIT)
        end
    })
]])
