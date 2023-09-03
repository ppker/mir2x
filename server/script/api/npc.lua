local npc = {}
function npc.getUID(mapName, npcName)
    assertType(mapName, 'string')
    assertType(npcName, 'string')
end
return npc
