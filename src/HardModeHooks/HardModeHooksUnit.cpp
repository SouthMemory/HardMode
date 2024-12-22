#include "HardModeHooks/HardModeHooksUnit.h"
#include "HardModeHandler.h"

void HardModeHooksUnitScript::OnAuraApply(Unit* unit, Aura* /*aura*/)
{
    if (!sHardModeHandler->IsHardModeEnabled())
    {
        return;
    }

    if (!unit)
    {
        return;
    }

    if (!unit->IsPlayer())
    {
        return;
    }

    Player* player = unit->ToPlayer();
    if (!player || !player->IsInWorld())
    {
        return;
    }

    if (sHardModeHandler->PlayerHasRestriction(player->GetGUID(), HARDMODE_RESTRICT_SMALLFISH))
    {
        // Schedule due to issues..
        sHardModeHandler->GetScheduler()->Schedule(50ms, [player](TaskContext /*task*/)
        {
            sHardModeHandler->UpdatePlayerScaleSpeed(player, SMALLFISH_SCALE);
        });
    }
}

void HardModeHooksUnitScript::OnAuraRemove(Unit* unit, AuraApplication* /*auraApp*/, AuraRemoveMode mode)
{
    if (!sHardModeHandler->IsHardModeEnabled())
    {
        return;
    }

    if (!unit)
    {
        return;
    }

    if (!unit->IsPlayer())
    {
        return;
    }

    Player* player = unit->ToPlayer();

    if (!player || !player->IsInWorld())
    {
        return;
    }

    if (sHardModeHandler->PlayerHasRestriction(player->GetGUID(), HARDMODE_RESTRICT_SMALLFISH))
    {
        // Schedule due to issues..
        sHardModeHandler->GetScheduler()->Schedule(50ms, [player](TaskContext /*task*/)
        {
            sHardModeHandler->UpdatePlayerScaleSpeed(player, SMALLFISH_SCALE);
        });
    }

    // Don't reapply aura on death, resurrection handles reapplication already.
    if (mode == AURA_REMOVE_BY_DEATH)
    {
        return;
    }

    sHardModeHandler->ValidatePlayerAuras(player);
}

void HardModeHooksUnitScript::OnDamage(Unit* attacker, Unit* target, uint32& damage)
{
    if (!sHardModeHandler->IsHardModeEnabled())
    {
        return;
    }

    if (!attacker)
    {
        return;
    }

    if (!attacker->IsPlayer() && !attacker->IsSummon())
    {
        return;
    }

    Player* player = attacker->ToPlayer();

    if (!player || attacker->IsSummon())
    {
        if (auto owner = attacker->GetOwner())
        {
            player = owner->ToPlayer();
        }
    }

    if (!player)
    {
        return;
    }

    if (sHardModeHandler->PlayerHasRestriction(player->GetGUID(), HARDMODE_RESTRICT_PACIFIST))
    {
        auto modes = sHardModeHandler->GetPlayerModesFromRestriction(player->GetGUID(), HARDMODE_RESTRICT_PACIFIST);
        for (auto mode : modes)
        {
            if (!mode.Enabled)
            {
                continue;
            }

            sHardModeHandler->UpdateModeForPlayer(player->GetGUID(), mode.Id, false);
        }

        // TODO: Update this to alert in the chat so the player is more aware.
        sHardModeHandler->SendAlert(player, "You have failed the pacifist challenge.");
        player->AddAura(HARDMODE_AURA_PACIFIST_FAIL, player);
    }

    auto targetPlayer = target->ToPlayer();

    if (!targetPlayer)
    {
        return;
    }

    if (!sHardModeHandler->HasMatchingModesWithRestriction(player, targetPlayer, HARDMODE_RESTRICT_BLOCK_CROSSPVP))
    {
        damage = 0;
        sHardModeHandler->SendAlert(player, "You cannot damage players in other modes than your own.");
    }
}

void HardModeHooksUnitScript::ModifyPeriodicDamageAurasTick(Unit* target, Unit* attacker, uint32& damage, SpellInfo const* /*spellInfo*/)
{
    if (!sHardModeHandler->IsHardModeEnabled())
    {
        return;
    }

    if (!attacker)
    {
        return;
    }

    if (!attacker->IsPlayer())
    {
        return;
    }

    auto player = attacker->ToPlayer();

    if (!player)
    {
        return;
    }

    if (sHardModeHandler->PlayerHasRestriction(player->GetGUID(), HARDMODE_RESTRICT_SMALLFISH))
    {
        damage = damage * SMALLFISH_SCALE;
    }

    auto targetPlayer = target->ToPlayer();

    if (!targetPlayer)
    {
        return;
    }

    if (!sHardModeHandler->HasMatchingModesWithRestriction(player, targetPlayer, HARDMODE_RESTRICT_BLOCK_CROSSPVP))
    {
        damage = 0;
        sHardModeHandler->SendAlert(player, "You cannot damage players in other modes than your own.");
    }
}

void HardModeHooksUnitScript::ModifyMeleeDamage(Unit* target, Unit* attacker, uint32& damage)
{
    if (!sHardModeHandler->IsHardModeEnabled())
    {
        return;
    }

    if (!attacker)
    {
        return;
    }

    if (!attacker->IsPlayer() && !attacker->IsSummon())
    {
        return;
    }

    Player* player = attacker->ToPlayer();

    if (!player || attacker->IsSummon())
    {
        if (auto owner = attacker->GetOwner())
        {
            player = owner->ToPlayer();
        }
    }

    if (!player)
    {
        return;
    }

    if (sHardModeHandler->PlayerHasRestriction(player->GetGUID(), HARDMODE_RESTRICT_SMALLFISH))
    {
        damage = damage * SMALLFISH_SCALE;
    }

    auto targetPlayer = target->ToPlayer();

    if (!targetPlayer)
    {
        return;
    }

    if (!sHardModeHandler->HasMatchingModesWithRestriction(player, targetPlayer, HARDMODE_RESTRICT_BLOCK_CROSSPVP))
    {
        damage = 0;
        sHardModeHandler->SendAlert(player, "You cannot damage players in other modes than your own.");
    }
}

void HardModeHooksUnitScript::ModifySpellDamageTaken(Unit* target, Unit* attacker, int32& damage, SpellInfo const* /*spellInfo*/)
{
    if (!sHardModeHandler->IsHardModeEnabled())
    {
        return;
    }

    if (!attacker)
    {
        return;
    }

    if (!attacker->IsPlayer() && !attacker->IsSummon())
    {
        return;
    }

    Player* player = attacker->ToPlayer();

    if (!player || attacker->IsSummon())
    {
        if (auto owner = attacker->GetOwner())
        {
            player = owner->ToPlayer();
        }
    }

    if (!player)
    {
        return;
    }

    if (sHardModeHandler->PlayerHasRestriction(player->GetGUID(), HARDMODE_RESTRICT_SMALLFISH))
    {
        damage = damage * SMALLFISH_SCALE;
    }

    auto targetPlayer = target->ToPlayer();

    if (!targetPlayer)
    {
        return;
    }

    if (!sHardModeHandler->HasMatchingModesWithRestriction(player, targetPlayer, HARDMODE_RESTRICT_BLOCK_CROSSPVP))
    {
        damage = 0;
        sHardModeHandler->SendAlert(player, "You cannot damage players in other modes than your own.");
    }
}

void HardModeHooksUnitScript::OnUnitDeath(Unit* unit, Unit* killer)
{
    if (!sHardModeHandler->IsHardModeEnabled())
    {
        return;
    }

    if (!sConfigMgr->GetOption<bool>("HardMode.Restrict.Permadeath.Announce", true))
    {
        return;
    }

    if (!unit)
    {
        return;
    }

    auto player = unit->ToPlayer();
    if (!player)
    {
        return;
    }

    // 检查玩家是否有硬核限制
    if (!sHardModeHandler->PlayerHasRestriction(player->GetGUID(), HARDMODE_RESTRICT_PERMADEATH))
    {
        return;
    }

    auto playerSettings = sHardModeHandler->GetPlayerSetting(player->GetGUID());
    uint8 OriginalLivesRemaining = playerSettings->LivesRemaining;

    // 减少玩家生命值
    if (playerSettings && playerSettings->LivesRemaining >= 1)
    {
        playerSettings->LivesRemaining--;

        // sHardModeHandler->UpdatePlayerSettings(player->GetGUID(), playerSettings);
    }

    // 如果玩家被 ShadowBan，则不再发送公告
    if (sHardModeHandler->IsPlayerShadowBanned(player->GetGUID()))
    {
        return;
    }

    // 构建公告内容
    std::stringstream ss;

    if (killer)
    {
        ss << Acore::StringFormat("|cffFF0000玩家 {} 死于 {} {} 之手。",
                                player->GetName(),
                                killer->ToPlayer() ? "玩家" : "生物",
                                killer->GetName());
    }
    else
    {
        ss << Acore::StringFormat("|cffFF0000玩家 {} 死亡。", player->GetName());
    }

    // 添加剩余生命值的信息
    if (playerSettings)
    {
        ss << Acore::StringFormat("剩余生命值：{}。|r", playerSettings->LivesRemaining);

        // 如果生命值耗尽，添加永久死亡的说明
        if (playerSettings->LivesRemaining <= 0)
        {
            ss << "\n|cffFF0000此次死亡是永恒的，请铭记他的努力。|r";
        }
    }

    // 发送服务器公告
    sWorld->SendServerMessage(SERVER_MSG_STRING, ss.str());

    if (sHardModeHandler->IsModeEnabledForPlayer(player->GetGUID(), 4)){
        constexpr uint32 TITLE_START = 179;
        constexpr uint32 TITLE_LENGTH= 108;
        constexpr uint32 TITLE_SAVED_DB = 36;

        CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(TITLE_START + playerSettings->LivesRemaining + 1 - 1);
        if (titleEntry) {
            ss.str("");
            ss << Acore::StringFormat("|cffFFFF00梁山好汉 {} 已重归天界！|r", titleEntry->nameMale[4]);
            // send system message
            player->SendSystemMessage(ss.str());
        }

        // 删除所有头衔
        for (uint32 i = TITLE_START; i < TITLE_START + TITLE_LENGTH; ++i)
        {
            // 安全检查头衔是否存在
            if (CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(i))
            {
                player->SetTitle(titleEntry, true); // 删除头衔
            }
        }

        sHardModeHandler->RewardItemsOnDeath(player, 60000, (108 - playerSettings->LivesRemaining)*2);
    }
}
