local dq = require('npc.include.dailyquest')

_G.questName_mujun    = '乞丐任务'
_G.questName_pariche  = '苍蝇拍任务'
_G.questName_dolumi   = '石母任务'

_G.questName_wang     = '王大人任务'
_G.questName_bichun   = '比奇省任务'
_G.questName_yaksa    = '药剂师任务'

_G.questName_kyunggap = '轻型盔甲任务'
_G.questName_oma      = '半兽人任务'

_G.questName_younghon = '被盗灵魂任务'
_G.questName_bamgol   = '千年毒蛇任务'
_G.questName_tarak    = '堕落道士任务'

_G.questName_umyun    = '沃玛教主任务'

setEventHandler(
{
    [SYS_ENTER] = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>江湖上的朋友都叫我万拍子，不是我吹，你不了解的任务我都可以给你解答。你有什么想问的吗？</par>
                <par></par>
                <par><event id="npc_show_quest_list">询问一般的任务</event></par>
                <par><event id="npc_daily_quest">对今日的任务进行了解</event></par>
                <par><event id="%s">结束</event></par>
            </layout>
        ]], SYS_EXIT)
    end,

    npc_show_quest_list = function(uid, value)
        uidPostXML(uid,
        [[
            <layout>
                <par>来吧，你有什么任务？</par>
                <par><event id="npc_goto_3">乞丐任务</event>，<event id="npc_goto_pariche">苍蝇拍任务</event>，<event id="npc_goto_5">石母任务</event></par>
                <par></par>

                <par>（等级 9）</par>
                <par><event id="npc_goto_6">王大人任务</event>，<event id="npc_goto_7">比奇省任务</event>，<event id="npc_goto_8">药剂师任务</event></par>
                <par></par>

                <par>（等级 11）</par>
                <par><event id="npc_goto_9">轻型盔甲任务</event>，<event id="npc_goto_10">半兽人任务</event></par>
                <par></par>

                <par>（等级 16）</par>
                <par><event id="npc_goto_11">被盗的灵魂任务</event>，<event id="npc_goto_12">千年毒蛇任务</event>，<event id="npc_goto_13">堕落道士任务</event></par>
                <par></par>

                <par>（等级 20）</par>
                <par><event id="npc_goto_14">沃玛教主任务</event></par>
                <par></par>

                <par><event id="%s">前一步</event></par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]], SYS_ENTER, SYS_EXIT)
    end,

    npc_daily_quest = function(uid, value)
        if uidQueryLevel(uid) < 7 then
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有足够能力执行此任务，修炼到7级后，再来找我吧。</par>
                    <par></par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        else
            dq.setQuest(0, uid, value)
        end
    end,

    npc_goto_pariche = function(uid, value)
        if plyapi.getLevel(uid) < 5 then
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始苍蝇拍任务呢！</par>
                    <par>这是一个为在比奇省经营肉铺店的<t color="red">金氏(446:405)</t>找苍蝇拍的任务！</par>
                    <par>可惜你现在还没有帮助他的能力。先去把等级提高到<t color="red">5</t>以上吧！</par>
                    <par><event id="npc_show_quest_list">前一步</event></par>
                </layout>
            ]])
        else
            local questState = plyapi.getQuestState(uid, questName_pariche)
            if questState == nil then
                uidPostXML(uid,
                [[
                    <layout>
                        <par>你还没有开始苍蝇拍任务呢！</par>
                        <par>比奇省经营肉铺店的<t color="red">金氏(446:405)</t>正在因为没有苍蝇拍的事儿而苦恼呢！去看看怎么回事吧！</par>
                        <par><event id="%s">结束</event></par>
                    </layout>
                ]], SYS_EXIT)
            elseif questState == SYS_DONE then
                uidPostXML(uid,
                [[
                    <layout>
                        <par>金氏拿到苍蝇拍以后，对你非常感激。</par>
                        <par><event id="%s">结束</event></par>
                    </layout>
                ]], SYS_EXIT)
            else
                runEventHandler(uid, {SYS_EPUID, questName_pariche}, SYS_ENTER)
            end
        end
    end,
})
