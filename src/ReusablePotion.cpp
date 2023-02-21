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
        if (sConfigMgr->GetOption<bool>("ReusablePotion.Verbose", false))
        {
            LOG_INFO("module", "ReusablePotion(CanCastItemUseSpell:Disabled)");
        }

        ChatHandler(player->GetSession()).SendSysMessage("This item is disabled.");

        return false;
    }

    if (!sConfigMgr->GetOption<bool>("ReusablePotion.PvPEnable", false))
    {
        bool isInPvPCombat = GetPlayerPvPState(player);

        if (sConfigMgr->GetOption<bool>("ReusablePotion.Verbose", false))
        {
            LOG_INFO("module", "ReusablePotion(CanCastItemUseSpell:PVPCheck)");
        }

        if (isInPvPCombat)
        {
            ChatHandler(player->GetSession()).SendSysMessage("This item is disabled in pvp.");

            if (sConfigMgr->GetOption<bool>("ReusablePotion.Verbose", false))
            {
                LOG_INFO("module", "ReusablePotion(CanCastItemUseSpell:PvPDisabled)");
            }

            return false;
        }
    }

    if (sConfigMgr->GetOption<bool>("ReusablePotion.Verbose", false))
    {
        LOG_INFO("module", "ReusablePotion(CanCastItemUseSpell)");
    }

    player->CastSpell(player, SPELLID_HEALING_POTION);

    uint32 potionCooldown = sConfigMgr->GetOption<uint32>("ReusablePotion.Cooldown", 15) * IN_MILLISECONDS;

    player->AddSpellCooldown(SPELLID_DUMMY, ITEMID_HEALING_POTION, potionCooldown, true, true);

    WorldPacket cooldownPacket;
    player->BuildCooldownPacket(cooldownPacket, SPELL_COOLDOWN_FLAG_NONE, SPELLID_DUMMY, potionCooldown);
    player->GetSession()->SendPacket(&cooldownPacket);

    WorldPacket itemCooldownPacket(SMSG_ITEM_COOLDOWN, 8 + 4);
    itemCooldownPacket << item->GetGUID();
    itemCooldownPacket << SPELLID_DUMMY;
    player->GetSession()->SendPacket(&itemCooldownPacket);

    player->ModifySpellCooldown(SPELLID_DUMMY, -(ITEM_COOLDOWN_BASE * IN_MILLISECONDS) + potionCooldown);

    return false;
}

void ReusablePotionPlayerScript::OnPlayerEnterCombat(Player* player, Unit* enemy)
{
    if (!player)
    {
        return;
    }

    if (!enemy)
    {
        return;
    }

    if (!enemy->ToPlayer())
    {
        return;
    }

    if (!sConfigMgr->GetOption<bool>("ReusablePotion.Enable", false))
    {
        return;
    }

    if (sConfigMgr->GetOption<bool>("ReusablePotion.Verbose", false))
    {
        LOG_INFO("module", "ReusablePotion(OnPlayerEnterCombat)");
    }

    SetPlayerPvPState(player, true);
    SetPlayerPvPState(enemy->ToPlayer(), true);
}

void ReusablePotionPlayerScript::OnPlayerLeaveCombat(Player* player)
{
    if (!player)
    {
        return;
    }

    if (!sConfigMgr->GetOption<bool>("ReusablePotion.Enable", false))
    {
        return;
    }

    if (sConfigMgr->GetOption<bool>("ReusablePotion.Verbose", false))
    {
        LOG_INFO("module", "ReusablePotion(OnPlayerLeaveCombat)");
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

    if (sConfigMgr->GetOption<bool>("ReusablePotion.Verbose", false))
    {
        LOG_INFO("module", "ReusablePotion(OnDamage)");
    }

    if (attPlayer->GetGUID() == victPlayer->GetGUID())
    {
        if (sConfigMgr->GetOption<bool>("ReusablePotion.Verbose", false))
        {
            LOG_INFO("module", "ReusablePotion(OnDamage:DamageSelf)");
        }
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

    if (sConfigMgr->GetOption<bool>("ReusablePotion.Verbose", false))
    {
        LOG_INFO("module", "ReusablePotion(ModifyHealReceived)");
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
