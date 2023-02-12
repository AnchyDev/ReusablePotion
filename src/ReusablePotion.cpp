#include "ReusablePotion.h"

void SetPlayerPvPState(Player* player, bool state)
{
    uint32 playerGuid = player->GetGUID().GetRawValue();
    auto mapKey = playerPvpMap.find(playerGuid);
    if (mapKey != playerPvpMap.end())
    {
        mapKey->second = state;
    }
    else
    {
        playerPvpMap.emplace(playerGuid, state);
    }
}

bool GetPlayerPvPState(Player* player)
{
    uint32 playerGuid = player->GetGUID().GetRawValue();
    auto mapKey = playerPvpMap.find(playerGuid);

    if (mapKey != playerPvpMap.end())
    {
        return mapKey->second;
    }

    return false;
}

bool ReusablePotionPlayerScript::CanCastItemUseSpell(Player* player, Item* item, SpellCastTargets const& /*targets*/, uint8 /*cast_count*/, uint32 /*glyphIndex*/)
{
    if (!item)
    {
        return true;
    }

    auto itemTemplate = item->GetTemplate();
    if (!itemTemplate)
    {
        return true;
    }

    if (itemTemplate->ItemId != ITEMID_HEALING_POTION)
    {
        return true;
    }

    if (!sConfigMgr->GetOption<bool>("ReusablePotion.Enable", false))
    {
        ChatHandler(player->GetSession()).SendSysMessage("This item is disabled.");
        return true;
    }

    if (!sConfigMgr->GetOption<bool>("ReusablePotion.PvPEnable", false))
    {
        bool isInPvPCombat = GetPlayerPvPState(player);

        if (isInPvPCombat)
        {
            ChatHandler(player->GetSession()).SendSysMessage("This item is disabled in pvp.");
            return true;
        }
    }

    player->CastSpell(player, SPELLID_HEALING_POTION);

    return true;
}

void ReusablePotionPlayerScript::OnPlayerEnterCombat(Player* player, Unit* enemy)
{
    if (!player)
    {
        return;
    }

    if (!enemy && !enemy->ToPlayer())
    {
        return;
    }

    if (!sConfigMgr->GetOption<bool>("ReusablePotion.Enable", false))
    {
        return;
    }

    SetPlayerPvPState(player, true);
    SetPlayerPvPState(enemy->ToPlayer(), true);
}

void ReusablePotionPlayerScript::OnPlayerLeaveCombat(Player* player)
{
    if (!sConfigMgr->GetOption<bool>("ReusablePotion.Enable", false))
    {
        return;
    }

    SetPlayerPvPState(player, false);
}

void ReusablePotionUnitScript::OnDamage(Unit* attacker, Unit* victim, uint32& /*damage*/)
{
    if (!sConfigMgr->GetOption<bool>("ReusablePotion.Enable", false))
    {
        return;
    }

    if (!attacker)
    {
        return;
    }

    auto attPlayer = attacker->ToPlayer();
    if (!attPlayer)
    {
        return;
    }

    if (!victim)
    {
        return;
    }

    auto victPlayer = victim->ToPlayer();
    if (!victPlayer)
    {
        return;
    }

    if (attPlayer->GetGUID() == victPlayer->GetGUID())
    {
        return;
    }

    SetPlayerPvPState(attPlayer, true);
    SetPlayerPvPState(victPlayer, true);
}

void ReusablePotionUnitScript::ModifyHealReceived(Unit* target, Unit* healer, uint32& addHealth, SpellInfo const* spellInfo)
{
    if (!sConfigMgr->GetOption<bool>("ReusablePotion.Enable", false))
    {
        return;
    }

    if (!target || !healer)
    {
        return;
    }

    if (!target->ToPlayer() || !healer->ToPlayer())
    {
        return;
    }

    if (spellInfo->Id != SPELLID_HEALING_POTION)
    {
        return;
    }

    float healPercent = sConfigMgr->GetOption<float>("ReusablePotion.HealPercent", 25.0);
    float healAmount = target->ToPlayer()->CountPctFromMaxHealth(healPercent);
    addHealth = healAmount;
}

void AddMyReusablePotionScripts()
{
    new ReusablePotionPlayerScript();
    new ReusablePotionUnitScript();
}
