#ifndef MODULE_REUSABLE_POTION_H
#define MODULE_REUSABLE_POTION_H

#include "ScriptMgr.h"
#include "Player.h"
#include "Spell.h"
#include "Chat.h"
#include "Config.h"

#include <vector>

std::unordered_map<uint64, bool> playerPvpMap;
void SetPlayerPvPState(Player* /*player*/, bool /*state*/);
bool GetPlayerPvPState(Player* /*player*/);

enum ReusablePotConstants
{
    REUSE_SPELL_EFFECT_HEAL = 10,
    REUSE_SPELL_EFFECT_TARGET_SELF = 1,
    REUSE_SPELL_VISUAL_POTION = 147,
    REUSE_SPELL_COOLDOWN_BASE = 60
};

class ReusablePotionPlayerScript : public PlayerScript
{
public:
    ReusablePotionPlayerScript() : PlayerScript("ReusablePotionPlayerScript") { }

    void OnPlayerLeaveCombat(Player* /*player*/) override;
};

class ReusablePotionUnitScript : public UnitScript
{
public:
    ReusablePotionUnitScript() : UnitScript("ReusablePotionUnitScript") { }
    void ModifyHealReceived(Unit* /*target*/, Unit* /*healer*/, uint32& /*addHealth*/, SpellInfo const* /*spellInfo*/) override;
    void OnDamage(Unit* /*attacker*/, Unit* /*victim*/, uint32& /*damage*/) override;
};

#endif // MODULE_REUSABLE_POTION_H
