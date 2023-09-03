_G.questName_basic    = '初级任务'

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

local quest_config = {
    [questName_mujun] = {
        level = 3,
        level_failure = function(uid)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始完成乞丐任务呢！</par>
                    <par>这个任务是去帮助在比奇省东海客栈工作的客栈店员遇到的麻烦。</par>
                    <par>可惜你现在还没有能力帮助她啊！先去把<t color="red">等级</t>修炼提高到<t color="red">3</t>以上再说吧！</par>
                    <par><event id="npc_show_quest_list">前一步</event></par>
                </layout>
            ]])
        end,

        on_start = function(uid, args)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始完成乞丐任务呢！</par>
                    <par>在比奇省东海客栈工作的客栈店员最近好像有点棘手的事情，去看看吧！</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,

        on_done = function(uid, args)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你辛苦了。</par>
                    <par>像你这样热心帮助别人的好心人，一定有好报的...</par>
                    <par></par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,
    },

    [questName_pariche] = {
        level = 5,
        level_failure = function(uid, args)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始苍蝇拍任务呢！</par>
                    <par>这是一个为在比奇省经营肉铺店的金氏找苍蝇拍的任务！</par>
                    <par>可惜你现在还没有帮助他的能力。先去把等级提高到<t color="red">5</t>以上吧！</par>
                    <par><event id="npc_show_quest_list">前一步</event></par>
                </layout>
            ]])
        end,

        on_start = function(uid, args)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始苍蝇拍任务呢！</par>
                    <par>比奇省经营肉铺店的金氏正在因为没有苍蝇拍的事儿而苦恼呢！去看看怎么回事吧！</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,

        on_done = function(uid, args)
            uidPostXML(uid,
            [[
                <layout>
                    <par>金氏拿到苍蝇拍以后，对你非常感激。</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,
    },

    [questName_dolumi] = {
        level = 7,
        level_failure = function(uid, args)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始石母任务呢！</par>
                    <par>石母任务是安抚冤魂的任务。可惜你现在还没有能力帮助她，首先去把等级修炼提高到7以上吧！</par>
                    <par><event id="npc_show_quest_list">前一步</event></par>
                </layout>
            ]])
        end,

        on_start = function(uid, args)
            uidPostXML(uid,
            [[
                <layout>
                    <par>你还没有开始石母任务呢！</par>
                    <par>最近，在比奇省东边的公园常传出隐约的抽泣声。前去调查一番吧！</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,

        on_done = function(uid, args)
            uidPostXML(uid,
            [[
                <layout>
                    <par>哦？原来石母就是那童子像的母亲石啊！</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]], SYS_EXIT)
        end,
    }
}

setEventHandler(
{
    [SYS_ENTER] = function(uid, args)
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

    npc_show_quest_list = function(uid, args)
        local fn_create_tag = function(questName)
            return string.format([[<event id="npc_run_quest" args="%s">%s</event>]], questName, questName)
        end

        uidPostXML(uid,
        [[
            <layout>
                <par>来吧，你有什么任务？</par>
                <par>%s，%s，%s</par>
                <par></par>

                <par>（等级 9）</par>
                <par>%s，%s，%s</par>
                <par></par>

                <par>（等级 11）</par>
                <par>%s，%s</par>
                <par></par>

                <par>（等级 16）</par>
                <par>%s，%s，%s</par>
                <par></par>

                <par>（等级 20）</par>
                <par>%s</par>
                <par></par>

                <par><event id="%s">前一步</event></par>
                <par><event id="%s">关闭</event></par>
            </layout>
        ]],

        fn_create_tag('乞丐任务'),
        fn_create_tag('苍蝇拍任务'),
        fn_create_tag('石母任务'),

        fn_create_tag('王大人任务'),
        fn_create_tag('比奇省任务'),
        fn_create_tag('药剂师任务'),

        fn_create_tag('轻型盔甲任务'),
        fn_create_tag('半兽人任务'),

        fn_create_tag('被盗的灵魂任务'),
        fn_create_tag('千年毒蛇任务'),
        fn_create_tag('堕落道士任务'),

        fn_create_tag('沃玛教主任务'),

        SYS_ENTER, SYS_EXIT)
    end,

    npc_daily_quest = function(uid, args)
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
            require('npc.include.dailyquest').setQuest(0, uid, args)
        end
    end,

    npc_run_quest = function(uid, args)
        if not quest_config[args] then
            uidPostXML(uid,
            [[
                <layout>
                    <par>我对<t color="red">%s</t>一无所知。</par>
                    <par></par>
                    <par><event id="npc_show_quest_list">前一步</event></par>
                </layout>
            ]], args)

        elseif quest_config[args].prequest and plyapi.getQuestState(uid, quest_config[args].prequest) ~= SYS_DONE then
            if quest_config[args].prequest_failure then
                quest_config[args].prequest_failure(uid)

            else
                uidPostXML(uid,
                [[
                    <layout>
                        <par>你还没有开始完成<t color="red">%s</t>呢！</par>
                        <par></par>
                        <par><event id="npc_show_quest_list">前一步</event></par>
                    </layout>
                ]], quest_config[args].prequest)
            end

        elseif quest_config[args].level and plyapi.getLevel(uid) < quest_config[args].level then
            if quest_config[args].level_failure then
                quest_config[args].level_failure(uid)

            else
                uidPostXML(uid,
                [[
                    <layout>
                        <par>你还没有达到<t color="red">%d</t>级呢！</par>
                        <par></par>
                        <par><event id="npc_show_quest_list">前一步</event></par>
                    </layout>
                ]], quest_config[args].leve)
            end

        else
            local questState = plyapi.getQuestState(uid, args)
            if questState == nil then
                if quest_config[args].on_start then
                    quest_config[args].on_start(uid, args)

                else
                    uidPostXML(uid,
                    [[
                        <layout>
                            <par>暂无关于<t color="red">%s</t>的更多信息...</par>
                            <par></par>
                            <par><event id="npc_show_quest_list">前一步</event></par>
                        </layout>
                    ]], args)
                end

            elseif questState == SYS_DONE then
                if quest_config[args].on_done then
                    quest_config[args].on_done(uid, args)

                else
                    uidPostXML(uid,
                    [[
                        <layout>
                            <par>你已经完成了<t color="red">%s</t></par>
                            <par></par>
                            <par><event id="npc_show_quest_list">前一步</event></par>
                        </layout>
                    ]], args)
                end

            else
                runEventHandler(uid, {SYS_EPUID, args}, SYS_ENTER)
            end
        end
    end,
})
