#include "ReusablePotion.h"
#include "GameTime.h"

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

void ReusablePotionPlayerScript::OnSpellCast(Player* player, Spell* spell, bool /*skipCheck*/)
{
    SpellInfo const* spellInfo = spell->GetSpellInfo();
    if (usedPotion) { // if a potion was used since the onspellcast call was made send a cooldown request for the potion then disable this
        if (lastPlayerPotion && spellStorage) {
            lastPlayerPotion->SendCooldownEvent(spellStorage);
            lastPlayerPotion->SetLastPotionId(0);
        }
        usedPotion = false;
    }
    if (!sConfigMgr->GetOption<bool>("ReusablePotion.Enable", false))
    {
        return;
    }

    if (!sConfigMgr->GetOption<bool>("ReusablePotion.PvPEnable", false))
    {
        bool isInPvPCombat = GetPlayerPvPState(player);

        if (isInPvPCombat)
        {
            return;
        }
    }
    
    auto effect1 = spellInfo->Effects[0].Effect;
    bool many_effects = false;

    if (effect1 == REUSE_SPELL_EFFECT_HEAL && sConfigMgr->GetOption<bool>("ReusablePotion.EnableHealing", false))
    {
        many_effects = true;
    }
    if (effect1 == REUSE_SPELL_EFFECT_MANA && sConfigMgr->GetOption<bool>("ReusablePotion.EnableMana", false))
    {
        many_effects = true;
    }
    if (effect1 == REUSE_SPELL_EFFECT_AURA && sConfigMgr->GetOption<bool>("ReusablePotion.EnableAura", false))
    {
        many_effects = true;
    }

    if (many_effects == false) {
        return;
    }
    bool many_visuals = false;
    auto visual1 = spellInfo->SpellVisual[0];
    if (visual1 == REUSE_SPELL_VISUAL_POTION)
    {
        many_visuals = true;
    }
    if (visual1 == REUSE_SPELL_VISUAL_POTION_OTHER_ONE) {
        many_visuals = true;
    }
    if (visual1 == REUSE_SPELL_VISUAL_POTION_FREE_ACTION) {
        many_visuals = true;
    }
    if (many_visuals == false) {
        return;
    }
    // if a potion is used let the class know and store the spellInfo and player for use when another spell is cast.
    usedPotion = true;
    spellStorage = spellInfo;
    lastPlayerPotion = player;
    return;
}

void AddMyCustomReusablePotionScripts()
{
    new ReusablePotionPlayerScript();
    new ReusablePotionUnitScript();
}
