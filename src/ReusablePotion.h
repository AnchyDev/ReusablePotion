#ifndef MODULE_REUSABLE_POTION_H
#define MODULE_REUSABLE_POTION_H

#include "ScriptMgr.h"
#include "Player.h"
#include "StatBoostMgr.h"
#include "Spell.h"
#include "Config.h"

enum ReusablePotionConstants
{
    ITEMID_HEALING_POTION = 44728,
    SPELLID_HEALING_POTION = 53144
};

std::unordered_map<uint64, bool> playerPvpMap;
void SetPlayerPvPState(Player* /*player*/, bool /*state*/);
bool GetPlayerPvPState(Player* /*player*/);

class ReusablePotionPlayerScript : public PlayerScript
{
public:
    ReusablePotionPlayerScript() : PlayerScript("ReusablePotionPlayerScript") { }

    bool CanCastItemUseSpell(Player* /*player*/, Item* /*item*/, SpellCastTargets const& /*targets*/, uint8 /*cast_count*/, uint32 /*glyphIndex*/) override;
    void OnPlayerEnterCombat(Player* /*player*/, Unit* /*enemy*/) override;
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
