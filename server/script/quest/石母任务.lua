uidRemoteCall(getNPCharUID('比奇县_0', '母子石像_1'),
[[
    addTrigger(SYS_ON_APPEAR, function(uid)
        if not isPlayer(uid) then
            return
        end

        uidPostXML(uid,
        [=[
            <layout>
                <par>你来找我啦？</par>
                <par><event id="%s">退出</event></par>
            </layout>
        ]=], SYS_EXIT)
    end)
]])
