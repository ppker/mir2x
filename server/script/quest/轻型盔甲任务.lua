_G.minQuestLevel = 11

setQuestFSMTable(
{
    [SYS_ENTER] = function(uid, args)
        setupNPCQuestBehavior('比奇县_0', '怡美_1', uid,
        [[
            return getQuestName()
        ]],
        [[
            local questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_ENTER] = function(uid, args)
                end,
            }
        ]])

        setupNPCQuestBehavior('比奇县_0', '杂货商_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>最近天气异常的炎热，苍蝇拍的库存货都全部卖光了！...你能帮我找些做苍蝇拍的材料来吗？</par>
                            <par><event id="npc_accept">好的！</event></par>
                        </layout>
                    ]=])
                end,

                npc_accept = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>找到苍蝇拍的材料的话我就会帮你做苍蝇拍！苍蝇拍所需的材料是牛毛和竹棍。牛毛可以从牛身上弄到，竹棍或许能从钉耙猫那儿弄到！</par>
                            <par><event id="%s">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    qstapi.setState(questUID, {uid=uid, state='quest_start_collection'})
                end,
            }
        ]])

        setupNPCQuestBehavior('道馆_1', '万事通_1', uid,
        [[
            return
            {
                [SYS_HIDE] = true,
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid,
                    [=[
                        <layout>
                            <par>按照金氏的吩咐，去杂货商那儿看看吧！杂货商(450:413)就在金氏店铺右边摆小摊呢！</par>
                            <par><event id="%s">返回</event></par>
                        </layout>
                    ]=], SYS_ENTER)
                end,
            }
        ]])
    end,

    quest_start_collection = function(uid, args)
        setQuestDesp{uid=uid, '杂货商希望你帮他找到制作苍蝇拍的材料竹棍和牛毛，这些东西可以从钉耙猫和牛身上找到。'}
        uidRemoteCall(uid, uid, getUID(),
        [[
            local playerUID, questUID = ...
            addTrigger(SYS_ON_GAINITEM, function(itemID, seqID)
                local has_cowHair    = hasItem(getItemID('牛毛'), 0, 1)
                local has_bambooPole = hasItem(getItemID('竹棍'), 0, 1)

                if has_cowHair and has_bambooPole then
                    postString('已经收集到牛毛和竹棍了，快回去找杂货商吧！')
                    uidRemoteCall(questUID, playerUID,
                    [=[
                        local playerUID = ...
                        setQuestDesp{uid=playerUID, '已经收集到制作苍蝇拍所需要的竹棍和牛毛，把他们交给杂货商吧。'}
                        setQuestState{uid=playerUID, state='quest_complete_collection'}
                    ]=])
                    return true
                end

                if has_cowHair then
                    postString('已经收集到牛毛了，快去猎杀钉耙猫获得竹棍吧！')
                    uidRemoteCall(questUID, playerUID,
                    [=[
                        local playerUID = ...
                        setQuestDesp{uid=playerUID, '获得了牛毛，再去找竹棍，一起送给杂货商。'}
                    ]=])
                end

                if has_bambooPole then
                    postString('已经收集到竹棍了，快去猎杀牛获得牛毛吧！')
                    uidRemoteCall(questUID, playerUID,
                    [=[
                        local playerUID = ...
                        setQuestDesp{uid=playerUID, '获得了竹棍，再去找牛毛，一起送给杂货商。'}
                    ]=])
                end
            end)
        ]])
    end,

    quest_complete_collection = function(uid, args)
        setupNPCQuestBehavior('比奇县_0', '杂货商_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_ENTER] = function(uid, args)
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哦！材料全部找到了啊！请稍等一下...</par>
                        </layout>
                    ]=])

                    pause(500)

                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>哦！材料全部找到了啊！请稍等一下...</par>
                            <par>给你！</par>
                            <par><event id="%s">结束</event></par>
                        </layout>
                    ]=], SYS_EXIT)

                    plyapi.addItem(uid, getItemID('苍蝇拍'), 1)
                    qstapi.setState(questUID, {uid=uid, state='quest_get_fly_swatter'})
                end,
            }
        ]])
    end,

    quest_get_fly_swatter = function(uid, args)
        setQuestDesp{uid=uid, '把材料转给杂货商后，获得了苍蝇拍，把苍蝇拍给肉店金氏送去吧。'}
        setupNPCQuestBehavior('比奇县_0', '金氏_1', uid,
        [[
            return getUID(), getQuestName()
        ]],
        [[
            local questUID, questName = ...
            local questPath = {SYS_EPUID, questName}

            return
            {
                [SYS_ENTER] = function(uid, args)
                    if plyapi.hasItem(uid, getItemID('苍蝇拍'), 0, 1) then
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>噢...真是太感谢了，现在可以对付这些该死的苍蝇了！</par>
                                <par>这是一些对你帮助的奖励，请你不要客气。</par>
                                <par><event id="%s">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)

                        plyapi.removeItem(uid, getItemID('苍蝇拍'), 0, 1)
                        plyapi.   addItem(uid, getItemID('蝉翼刀'), 1)

                        qstapi.setState(questUID, {uid=uid, state=SYS_DONE})
                    else
                        uidPostXML(uid, questPath,
                        [=[
                            <layout>
                                <par>你给我带的苍蝇拍呢？</par>
                                <par><event id="%s">结束</event></par>
                            </layout>
                        ]=], SYS_EXIT)
                    end
                end,
            }
        ]])
    end,
})

uidRemoteCall(getNPCharUID('比奇县_0', '怡美_1'), getUID(), getQuestName(), minQuestLevel,
[[
    local questUID, questName, minQuestLevel = ...
    local questPath = {SYS_EPQST, questName}

    setQuestHandler(questName,
    {
        [SYS_CHECKACTIVE] = function(uid)
            return qstapi.getState(questUID, {uid=uid, fsm=SYS_QSTFSM}) == nil
        end,

        [SYS_ENTER] = function(uid, args)
            if plyapi.getLevel(uid) < minQuestLevel then
                uidPostXML(uid, questPath,
                [=[
                    <layout>
                        <par>您就是最近为比奇商会四处游说的<t color="red">%s</t>吧！久仰久仰！</par>
                        <par>尽管你和王大人的关系十分熟，但是我跟你这种等级没有达到%d的毛头小子没什么话可说。想让我完全信任你的话还是再去好好修炼一下再来找我吧！</par>
                        <par></par>
                        <par><event id="%s">结束</event></par>
                    </layout>
                ]=], plyapi.getName(uid), minQuestLevel, SYS_EXIT)

            else
                local dressName = plyapi.getDressName(uid)
                if not dressName then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>您就是最近为比奇商会四处游说的<t color="red">%s</t>吧！久仰久仰！</par>
                            <par>不过...你这样的侠客，却不穿衣服在这里招摇过市真的好吗？这种打扮实在是让我难以置信啊！</par>
                            <par>本店特色商品轻型盔甲，一直备受各路闯荡江湖的侠客青睐，你感兴趣吗？</par>
                            <par><event id="npc_query">嗯？有这种东西？很感兴趣！</event></par>
                            <par><event id="%s">不感兴趣。</event></par>
                        </layout>
                    ]=], plyapi.getName(uid), SYS_EXIT)

                elif string.match(dressName, '布衣.*') then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>您就是最近为比奇商会四处游说的<t color="red">%s</t>吧！久仰久仰！</par>
                            <par>不过...看起来你今天的穿着很是稀松平常，说实话这种打扮实在难以让我信任啊！</par>
                            <par>不知你是否知道我们店里所卖的轻型盔甲呢？</par>
                            <par><event id="npc_query">嗯？有这种东西？很感兴趣！</event></par>
                            <par><event id="%s">不感兴趣。</event></par>
                        </layout>
                    ]=], plyapi.getName(uid), SYS_EXIT)

                elif string.match(dressName, '轻型盔甲.+') then
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>您就是最近为比奇商会四处游说的<t color="red">%s</t>吧！久仰久仰！</par>
                            <par>噢！已经穿上轻型盔甲了！的确与众不同啊！不过这衣服好像还是有点太平常了！</par>
                            <par><event id="npc_query">嗯？有这种东西？很感兴趣！</event></par>
                            <par>我现在有点忙，所以如果你能够帮我我一把话，我就会为你做一套更漂亮的轻型盔甲。</par>
                            <par><event id="%s">不感兴趣。</event></par>
                        </layout>
                    ]=], plyapi.getName(uid), SYS_EXIT)

                else
                    uidPostXML(uid, questPath,
                    [=[
                        <layout>
                            <par>您就是最近为比奇商会四处游说的<t color="red">%s</t>吧！久仰久仰！</par>
                            <par>您穿上这身衣服果然是器宇不凡！</par>
                            <par>本店特色商品轻型盔甲，一直备受各路闯荡江湖的侠客青睐，你感兴趣吗？</par>
                            <par><event id="npc_query">嗯？有这种东西？很感兴趣！</event></par>
                            <par><event id="%s">不感兴趣。</event></par>
                        </layout>
                    ]=], plyapi.getName(uid), SYS_EXIT)
                end
            end
        end,

        npc_query = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par><t color="red">轻型盔甲</t>是等级达到11级之后才可以穿上的防御服。</par>
                    <par>主要部分都是用钢铁打造的，所以比起布衣要重的多，但是防御力也特别的好。</par>
                    <par>我有一件关于这种盔甲的事情要拜托你...</par>
                    <par><event id="npc_accept">有什么要拜托的事情请您尽管说。</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        npc_ask_when_not_interested = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>既然您不感兴趣，那我就不再介绍了...</par>
                    <par>其实我并非想向您推销轻型盔甲，而是有一件关于这种盔甲的事情要拜托你...</par>
                    <par><event id="%s">有什么要拜托的事情请您尽管说。</event></par>
                </layout>
            ]=], plyapi.getName(uid), SYS_EXIT)
        end,

        npc_skip_introduce = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>那我就不必给你说明啦！</par>
                    <par>其实，我有一件关于这种盔甲的事情要拜托你...</par>
                    <par></par>
                    <par><event id="%s">有什么要拜托的事情请您尽管说。</event></par>
                    <par><event id="%s">知道了。</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        npc_accept = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>那就太谢谢了！是这样的，最近来买轻型盔甲的顾客非常多，可是用来做辅强剂的铁矿不够用了。</par>
                    <par>所以您要是能够给我找来5个纯度13以上的铁矿的话，我就会为你特别制作一套轻型盔甲。</par>
                    <par></par>
                    <par><event id="%s">到哪儿去找铁矿呢？</event></par>
                    <par><event id="%s">知道了。</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        npc_where_to_get_iron_mine = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>先去武器店或铁匠铺买把鹤嘴锄，再去<event id="npc_nearest_iron_mine">矿山</event>就可以挖到各种矿石！</par>
                    <par>挑出我所需要的纯度在13以上的铁矿之后，剩下的还可以卖给武器店赚到很多钱。</par>
                    <par>如果你觉得麻烦不想去矿山挖矿的话也可以去武器店购买铁矿，但是那样的话就有点亏本哦！</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        npc_nearest_iron_mine = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>离这里最近的矿山是比奇矿区，可能去764:206附近就能找到入口。</par>
                    <par></par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        npc_refuse = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>是吗？嗯...这真是郁闷啊，真愁人啊！</par>
                    <par></par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        npc_refuse = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>还没有带来我要的铁矿啊！</par>
                    <par></par>
                    <par><event id="%s">去哪儿才能找到铁矿呢？</event></par>
                    <par><event id="%s">请再给我一些时间。</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        npc_patience = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>那我等你的好消息。</par>
                    <par></par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        npc_lost_way = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>不知道就说不知道嘛！嗨...</par>
                    <par>先去武器店或铁匠铺买把鹤嘴锄，再去<矿山/@Guyonggap_start_20>就可以挖到各种矿石。</par>
                    <par>从中挑出{FCOLOR/10}5个纯度在13以上的铁矿{FCOLOR/12}带给我就行。</par>
                    <par>离这里最近的矿山是比奇矿区，可能去764:206附近就能找到入口。</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        npc_got_iron = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>拿来铁矿了啊！</par>
                    <par>嘻嘻，现在不用担心原料不足，可以稳定的供应顾客的需求啦！</par>
                    <par>太谢谢你啦!这是我说好为你定做的特制轻型盔甲。</par>
                    <par>这可是我特地为您精心制作的哦，所以要像我一样珍惜爱护它啊，呵呵呵！</par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,

        npc_refuse = function(uid, args)
            uidPostXML(uid, questPath,
            [=[
                <layout>
                    <par>但愿我送你的轻型盔甲能够带给你很大帮助，呵呵呵！</par>
                    <par></par>
                    <par><event id="%s">结束</event></par>
                </layout>
            ]=], SYS_EXIT)
        end,
    })
]])
