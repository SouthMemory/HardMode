#include "HardModeHooks/HardModeHooksPlayer.h"
#include "HardModeHandler.h"
#include "HardModeTypes.h"

#include "Config.h"
#include "Player.h"
#include "Creature.h"
#include "Log.h"
#include "TaskScheduler.h"

void HardModeHooksPlayerScript::OnGiveXP(Player* player, uint32& amount, Unit* victim, uint8 xpSource)
{
    if (!sHardModeHandler->IsHardModeEnabled())
    {
        return;
    }

    if (!player)
    {
        return;
    }

    // LOG_ERROR("esp.XP", "Player {} received {} XP from {} [OnGiveXP]", player->GetName(), amount, xpSource);
    // LOG_ERROR("esp.XP", "AnyHardModeEnabledForPlayer {}", sHardModeHandler->AnyHardModeEnabledForPlayer(player));

    if (!sHardModeHandler->AnyHardModeEnabledForPlayer(player))
    {
        return;
    }

    // if sHardModeHandler->IsHard

    if (xpSource == PlayerXPSource::XPSOURCE_KILL){
        // mode 4, 击杀对象为普通怪且非Monster时，无法获取经验
        Creature * killed = victim->ToCreature();
        if (sHardModeHandler->IsModeEnabledForPlayer(player->GetGUID(), 4) && killed)
        {
            // log the conditions
            LOG_ERROR("esp.XP", "Creature isMonster {}, isElite {}, isWorldBoss {}, isDungeonBoss {} [OnGiveXP]", killed->IsMonster(), killed->isElite(), killed->isWorldBoss(), killed->IsDungeonBoss());
            if (!(killed->IsMonster() || killed->isElite() || killed->isWorldBoss() || killed->IsDungeonBoss()))
                amount = 0;
        }
    }

    if (sHardModeHandler->PlayerHasRestriction(player->GetGUID(), HARDMODE_RESTRICT_RETAIL_XP))
    {
        switch (xpSource)
        {
            case PlayerXPSource::XPSOURCE_KILL:
                amount = (amount / sConfigMgr->GetOption<float>("Rate.XP.Kill", 1));
                break;

            case PlayerXPSource::XPSOURCE_EXPLORE:
                amount = (amount / sConfigMgr->GetOption<float>("Rate.XP.Explore", 1));
                break;
        }
    }

    if (sHardModeHandler->CanTaintPlayer(player->GetGUID()))
    {
        sHardModeHandler->UpdatePlayerTainted(player->GetGUID(), true);
    }
    LOG_ERROR("esp.XP", "Player {} received {} XP from {} [OnGiveXP]", player->GetName(), amount, xpSource);


    // 当玩家在队伍中，但不在副本中，将玩家移除出队伍
    if (player->GetGroup() && !player->GetMap()->Instanceable() && xpSource == PlayerXPSource::XPSOURCE_KILL)
    {
        // 解散队伍
        player->GetGroup()->Disband();
        
        // 通知玩家
        player->SendSystemMessage("挑战模式下无法在野外组队活动，队伍已解散");
    }
}

void HardModeHooksPlayerScript::OnQuestComputeXP(Player* player, Quest const* /*quest*/, uint32& xpValue)
{
    if (!sHardModeHandler->IsHardModeEnabled())
    {
        return;
    }

    if (!player)
    {
        return;
    }

    if (sHardModeHandler->PlayerHasRestriction(player->GetGUID(), HARDMODE_RESTRICT_RETAIL_XP))
    {
        xpValue = (xpValue / sConfigMgr->GetOption<float>("Rate.XP.Quest", 1));
    }
}

bool HardModeHooksPlayerScript::CanEquipItem(Player* player, uint8 /*slot*/, uint16& /*dest*/, Item* pItem, bool /*swap*/, bool /*notLoading*/)
{
    if (!sHardModeHandler->IsHardModeEnabled())
    {
        return true;
    }

    if (!player)
    {
        return true;
    }

    if (sHardModeHandler->PlayerHasRestriction(player->GetGUID(), HARDMODE_RESTRICT_SELFCRAFTED))
    {
        auto itemProto = pItem->GetTemplate();

        // Exclude item ids in the `hardmode_selfcraft_exclude` table.
        if (sHardModeHandler->IsSelfCraftItemExcluded(itemProto->ItemId))
        {
            return true;
        }

        // Allow quest items to be equipped.
        if (itemProto->Class == ITEM_CLASS_QUEST)
        {
            return true;
        }

        // If the item is not creator by the player, block equip.
        if (pItem->GetGuidValue(ITEM_FIELD_CREATOR) != player->GetGUID())
        {
            auto restrictedModes = sHardModeHandler->GetPlayerModesFromRestriction(player->GetGUID(), HARDMODE_RESTRICT_SELFCRAFTED);
            std::string alert = Acore::StringFormat("You cannot equip this item while in the {} mode(s).", sHardModeHandler->GetDelimitedModes(restrictedModes, ", "));
            sHardModeHandler->SendAlert(player, alert);

            return false;
        }
    }

    return true;
}

bool HardModeHooksPlayerScript::CanCastItemUseSpell(Player* player, Item* item, SpellCastTargets const& /*targets*/, uint8 /*castCount*/, uint32 /*glyphIndex*/)
{
    if (!sHardModeHandler->IsHardModeEnabled())
    {
        return true;
    }

    if (!player)
    {
        return true;
    }

    if (sHardModeHandler->PlayerHasRestriction(player->GetGUID(), HARDMODE_RESTRICT_SELFCRAFTED))
    {
        // Exclude spell ids in the `hardmode_selfcraft_exclude` table.
        if (sHardModeHandler->IsSelfCraftSpellExcluded(item->GetTemplate()->Spells[0].SpellId))
        {
            return true;
        }

        auto itemProto = item->GetTemplate();

        // Only consider blocking on consumables.
        if (itemProto->Class != ITEM_CLASS_CONSUMABLE)
        {
            return true;
        }

        if (itemProto->SubClass != ITEM_SUBCLASS_FOOD &&
            itemProto->SubClass != ITEM_SUBCLASS_POTION &&
            itemProto->SubClass != ITEM_SUBCLASS_ELIXIR &&
            itemProto->SubClass != ITEM_SUBCLASS_FLASK)
        {
            return true;
        }

        if (item->GetGuidValue(ITEM_FIELD_CREATOR) != player->GetGUID())
        {
            auto restrictedModes = sHardModeHandler->GetPlayerModesFromRestriction(player->GetGUID(), HARDMODE_RESTRICT_SELFCRAFTED);
            std::string alert = Acore::StringFormat("You cannot use this item while in the {} mode(s).", sHardModeHandler->GetDelimitedModes(restrictedModes, ", "));
            sHardModeHandler->SendAlert(player, alert);

            return false;
        }
    }

    return true;
}

void HardModeHooksPlayerScript::OnCreateItem(Player* player, Item* item, uint32 /*count*/)
{
    if (!sHardModeHandler->IsHardModeEnabled())
    {
        return;
    }

    if (!player || !item)
    {
        return;
    }

    if (sHardModeHandler->PlayerHasRestriction(player->GetGUID(), HARDMODE_RESTRICT_SELFCRAFTED))
    {
        if (!sConfigMgr->GetOption<bool>("HardMode.Restrict.SelfCrafted.CreatedBy", true))
        {
            return;
        }

        auto itemProto = item->GetTemplate();

        if (itemProto->Class != ITEM_CLASS_CONSUMABLE)
        {
            return;
        }

        item->SetGuidValue(ITEM_FIELD_CREATOR, player->GetGUID());
    }
}

void HardModeHooksPlayerScript::OnPlayerResurrect(Player* player, float /*restorePercent*/, bool /*applySickness*/)
{
    if (!sHardModeHandler->IsHardModeEnabled())
    {
        return;
    }

    if (!player)
    {
        return;
    }

    if (!sHardModeHandler->AnyHardModeEnabledForPlayer(player))
    {
        return;
    }

    auto playerSettings = sHardModeHandler->GetPlayerSetting(player->GetGUID());

    LOG_ERROR("esp.HardMode", "Player has {} lives Remaining [OnPlayerResurrect]", playerSettings->LivesRemaining);

    // 如果生命值为 0，启用 ShadowBan
    if (playerSettings->LivesRemaining <= 0)
    {
        // sHardModeHandler->TryShadowBanPlayer(player->GetGUID());
        // CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(178);
        // player->SetCurrentTitle(titleEntry);
    } else {
        // if mode 4 is enabled, reset the player's talents
        // bool IsModeEnabledForPlayer(ObjectGuid guid, uint8 mode);
        if (sHardModeHandler->IsModeEnabledForPlayer(player->GetGUID(), 4))
        {
            // 定义常量范围，避免硬编码
            constexpr uint32 TITLE_START = 179;
            constexpr uint32 TITLE_LENGTH= 108;
            constexpr uint32 TITLE_SAVED_DB = 36;

            std::stringstream ss;

            // 确保 LivesRemaining 在有效范围内
            uint32 remainingLives = std::clamp(playerSettings->LivesRemaining, uint8(1), uint8(TITLE_LENGTH)); // 限制在 1 到 108 之间

            // 根据剩余生命值设置当前头衔
            if (CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(TITLE_START + remainingLives - 1))
            {
                if (remainingLives <= TITLE_SAVED_DB)
                    player->SetTitle(titleEntry); // 添加到数据库

                ss.str("");
                ss << Acore::StringFormat("|cffFFFF00吾乃梁山好汉 {}是也！\n现借你肉身，除暴安良！|r", titleEntry->nameMale[4]);
                sHardModeHandler->SendAlert(player, ss.str());

                ss.str("");
                ss << Acore::StringFormat("|cffFFFF00你已被梁山好汉 {} 附身！|r", titleEntry->nameMale[4]);
                // send system message
                player->SendSystemMessage(ss.str());

                player->SetCurrentTitle(titleEntry);
            }
        }
    }

    // 如果玩家有 Permadeath 限制并且生命值为 0，则启用 ShadowBan
    if (playerSettings && sHardModeHandler->PlayerHasRestriction(player->GetGUID(), HARDMODE_RESTRICT_PERMADEATH))
    {
        if (playerSettings->LivesRemaining <= 0)
        {
            sHardModeHandler->TryShadowBanPlayer(player->GetGUID());
        }
    }

    sHardModeHandler->ValidatePlayerAuras(player);
}

void HardModeHooksPlayerScript::OnPlayerReleasedGhost(Player* player)
{
    if (!sHardModeHandler->IsHardModeEnabled())
    {
        return;
    }

    if (!player)
    {
        return;
    }

    if (!sHardModeHandler->AnyHardModeEnabledForPlayer(player))
    {
        return;
    }

    auto playerSettings = sHardModeHandler->GetPlayerSetting(player->GetGUID());
    LOG_ERROR("esp.HardMode", "Player has {} lives Remaining [OnPlayerReleasedGhost]", playerSettings->LivesRemaining);

    if (playerSettings && sHardModeHandler->PlayerHasRestriction(player->GetGUID(), HARDMODE_RESTRICT_PERMADEATH))
    {
        if (playerSettings->LivesRemaining == 0)
        {
            sHardModeHandler->TryShadowBanPlayer(player->GetGUID());
        }
    }
}

bool HardModeHooksPlayerScript::CanRepopAtGraveyard(Player* player)
{
    if (!sHardModeHandler->IsHardModeEnabled())
    {
        return true;
    }

    if (!player)
    {
        return true;
    }

    if (sHardModeHandler->IsPlayerShadowBanned(player->GetGUID()))
    {
        return false;
    }

    return true;
}

bool HardModeHooksPlayerScript::OnBeforeTeleport(Player* player, uint32 mapId, float /*x*/, float /*y*/, float /*z*/, float /*orientation*/, uint32 /*options*/, Unit* /*target*/)
{
    if (!sHardModeHandler->IsHardModeEnabled())
    {
        return true;
    }

    if (sHardModeHandler->IsPlayerShadowBanned(player->GetGUID()))
    {
        return (mapId == HARDMODE_AREA_AZSHARACRATER); // Only allow teleports for Shadowban players if it's to the azshara crater / shadow tomb.
    }

    return true;
}

void HardModeHooksPlayerScript::OnPlayerLearnTalents(Player* player, uint32 /*talentId*/, uint32 /*talentRank*/, uint32 /*spellId*/)
{
    if (!player)
    {
        return;
    }

    if (sHardModeHandler->PlayerHasRestriction(player->GetGUID(), HARDMODE_RESTRICT_INTERACT_TALENTS))
    {
        auto restrictedModes = sHardModeHandler->GetPlayerModesFromRestriction(player->GetGUID(), HARDMODE_RESTRICT_INTERACT_TALENTS);
        std::string alert = Acore::StringFormat("You cannot use talent points while in the {} mode(s).", sHardModeHandler->GetDelimitedModes(restrictedModes, ", "));
        sHardModeHandler->SendAlert(player, alert);

        player->resetTalents(true);
    }
}

bool HardModeHooksPlayerScript::CanInitTrade(Player* player, Player* target)
{
    if (!sHardModeHandler->IsHardModeEnabled())
    {
        return true;
    }

    if (!player || !target)
    {
        return true;
    }

    if (sHardModeHandler->PlayerHasRestriction(player->GetGUID(), HARDMODE_RESTRICT_INTERACT_TRADE))
    {
        player->GetSession()->SendTradeStatus(TRADE_STATUS_TRADE_CANCELED);
        target->GetSession()->SendTradeStatus(TRADE_STATUS_TRADE_CANCELED);

        auto restrictedModes = sHardModeHandler->GetPlayerModesFromRestriction(player->GetGUID(), HARDMODE_RESTRICT_INTERACT_TRADE);
        std::string alert = Acore::StringFormat("You cannot trade players while in the {} mode(s).", sHardModeHandler->GetDelimitedModes(restrictedModes, ", "));
        sHardModeHandler->SendAlert(player, alert);

        return false;
    }

    if (sHardModeHandler->PlayerHasRestriction(target->GetGUID(), HARDMODE_RESTRICT_INTERACT_TRADE))
    {
        player->GetSession()->SendTradeStatus(TRADE_STATUS_TRADE_CANCELED);
        target->GetSession()->SendTradeStatus(TRADE_STATUS_TRADE_CANCELED);

        auto restrictedModes = sHardModeHandler->GetPlayerModesFromRestriction(target->GetGUID(), HARDMODE_RESTRICT_INTERACT_TRADE);
        std::string alert = Acore::StringFormat("You cannot trade players in the {} mode(s).", sHardModeHandler->GetDelimitedModes(restrictedModes, ", "));
        sHardModeHandler->SendAlert(player, alert);

        return false;
    }

    if (sHardModeHandler->CanTaintPlayer(player->GetGUID()))
    {
        sHardModeHandler->UpdatePlayerTainted(player->GetGUID(), true);
    }

    if (sHardModeHandler->CanTaintPlayer(target->GetGUID()))
    {
        sHardModeHandler->UpdatePlayerTainted(target->GetGUID(), true);
    }

    return true;
}

bool HardModeHooksPlayerScript::CanSendMail(Player* player, ObjectGuid receiverGuid, ObjectGuid /*mailbox*/, std::string& /*subject*/, std::string& /*body*/, uint32 /*money*/, uint32 /*cod*/, Item* /*item*/)
{
    if (!sHardModeHandler->IsHardModeEnabled())
    {
        return true;
    }

    if (!player)
    {
        return true;
    }

    if (sHardModeHandler->PlayerHasRestriction(player->GetGUID(), HARDMODE_RESTRICT_INTERACT_MAIL_SEND))
    {
        auto restrictedModes = sHardModeHandler->GetPlayerModesFromRestriction(player->GetGUID(), HARDMODE_RESTRICT_INTERACT_MAIL_SEND);
        std::string alert = Acore::StringFormat("You cannot send mail to other players while in the {} mode(s).", sHardModeHandler->GetDelimitedModes(restrictedModes, ", "));
        sHardModeHandler->SendAlert(player, alert);
        return false;
    }

    if (sHardModeHandler->PlayerHasRestriction(receiverGuid, HARDMODE_RESTRICT_INTERACT_MAIL_RECEIVE))
    {
        auto restrictedModes = sHardModeHandler->GetPlayerModesFromRestriction(receiverGuid, HARDMODE_RESTRICT_INTERACT_MAIL_RECEIVE);
        std::string alert = Acore::StringFormat("You cannot send mail to players in the {} mode(s).", sHardModeHandler->GetDelimitedModes(restrictedModes, ", "));
        sHardModeHandler->SendAlert(player, alert);
        return false;
    }

    if (sHardModeHandler->CanTaintPlayer(player->GetGUID()))
    {
        sHardModeHandler->UpdatePlayerTainted(player->GetGUID(), true);
    }

    if (sHardModeHandler->CanTaintPlayer(receiverGuid))
    {
        sHardModeHandler->UpdatePlayerTainted(receiverGuid, true);
    }

    return true;
}

bool HardModeHooksPlayerScript::CanJoinLfg(Player* player, uint8 /*roles*/, lfg::LfgDungeonSet& /*dungeons*/, const std::string& /*comment*/)
{
    if (!sHardModeHandler->IsHardModeEnabled())
    {
        return true;
    }

    if (sHardModeHandler->PlayerHasRestriction(player->GetGUID(), HARDMODE_RESTRICT_INTERACT_LFG))
    {
        auto restrictedModes = sHardModeHandler->GetPlayerModesFromRestriction(player->GetGUID(), HARDMODE_RESTRICT_INTERACT_LFG);
        std::string alert = Acore::StringFormat("You cannot join looking for group while in the {} mode(s).", sHardModeHandler->GetDelimitedModes(restrictedModes, ", "));
        sHardModeHandler->SendAlert(player, alert);
        return false;
    }

    return true;
}

bool HardModeHooksPlayerScript::CanGroupInvite(Player* player, std::string& memberName)
{
    if (!sHardModeHandler->IsHardModeEnabled())
    {
        return true;
    }

    Player* target = ObjectAccessor::FindPlayerByName(memberName);

    if (!target)
    {
        return true;
    }

    if (sHardModeHandler->PlayerHasRestriction(target->GetGUID(), HARDMODE_RESTRICT_INTERACT_GROUP_CROSSPLAY))
    {
        if (!sHardModeHandler->HasMatchingModesWithRestriction(player, target, HARDMODE_RESTRICT_INTERACT_GROUP_CROSSPLAY))
        {
            auto restrictedModes = sHardModeHandler->GetPlayerModesFromRestriction(target->GetGUID(), HARDMODE_RESTRICT_INTERACT_GROUP_CROSSPLAY);
            std::string alert = Acore::StringFormat("You cannot invite players if you aren't in the cross-play {} mode(s).", sHardModeHandler->GetDelimitedModes(restrictedModes, ", "));
            sHardModeHandler->SendAlert(player, alert);
            return false;
        }
    }

    if (sHardModeHandler->PlayerHasRestriction(player->GetGUID(), HARDMODE_RESTRICT_INTERACT_GROUP_CROSSPLAY))
    {
        if (!sHardModeHandler->HasMatchingModesWithRestriction(player, target, HARDMODE_RESTRICT_INTERACT_GROUP_CROSSPLAY))
        {
            auto restrictedModes = sHardModeHandler->GetPlayerModesFromRestriction(player->GetGUID(), HARDMODE_RESTRICT_INTERACT_GROUP_CROSSPLAY);
            std::string alert = Acore::StringFormat("You cannot invite players that aren't in the cross-play {} mode(s).", sHardModeHandler->GetDelimitedModes(restrictedModes, ", "));
            sHardModeHandler->SendAlert(player, alert);
            return false;
        }
    }

    if (sHardModeHandler->PlayerHasRestriction(player->GetGUID(), HARDMODE_RESTRICT_INTERACT_GROUP))
    {
        auto restrictedModes = sHardModeHandler->GetPlayerModesFromRestriction(player->GetGUID(), HARDMODE_RESTRICT_INTERACT_GROUP);
        std::string alert = Acore::StringFormat("You cannot invite players while in the {} mode(s).", sHardModeHandler->GetDelimitedModes(restrictedModes, ", "));
        sHardModeHandler->SendAlert(player, alert);
        return false;
    }

    if (sHardModeHandler->PlayerHasRestriction(target->GetGUID(), HARDMODE_RESTRICT_INTERACT_GROUP))
    {
        auto restrictedModes = sHardModeHandler->GetPlayerModesFromRestriction(target->GetGUID(), HARDMODE_RESTRICT_INTERACT_GROUP);
        std::string alert = Acore::StringFormat("You cannot invite players in the {} mode(s).", sHardModeHandler->GetDelimitedModes(restrictedModes, ", "));
        sHardModeHandler->SendAlert(player, alert);
        return false;
    }

    if (sHardModeHandler->PlayerHasRestriction(player->GetGUID(), HARDMODE_RESTRICT_INTERACT_GROUP_RANGE) ||
        sHardModeHandler->PlayerHasRestriction(target->GetGUID(), HARDMODE_RESTRICT_INTERACT_GROUP_RANGE))
    {
        uint32 pLevel = player->GetLevel();
        uint32 tLevel = target->GetLevel();

        uint32 range = sConfigMgr->GetOption<uint32>("HardMode.Restrict.Interact.Group.Range", 3);
        uint32 result = std::abs(int32(pLevel - tLevel));
        if (result > range)
        {
            auto restrictedModes = sHardModeHandler->GetPlayerModesFromRestriction(target->GetGUID(), HARDMODE_RESTRICT_INTERACT_GROUP);
            std::string alert = Acore::StringFormat("You cannot invite players who are further than {} levels from you.", range);
            sHardModeHandler->SendAlert(player, alert);
            return false;
        }
    }

    return true;
}

void HardModeHooksPlayerScript::OnLogin(Player* player)
{
    if (!sHardModeHandler->IsHardModeEnabled())
    {
        return;
    }

    if (!player)
    {
        return;
    }

    sHardModeHandler->LoadPlayerSettings(player->GetGUID());

    if (sHardModeHandler->IsModeEnabledForPlayer(player->GetGUID(), 4)){
        // 定义常量范围，避免硬编码
        constexpr uint32 TITLE_START = 179;
        constexpr uint32 TITLE_LENGTH= 108;
        constexpr uint32 TITLE_SAVED_DB = 36;

        auto playerSettings = sHardModeHandler->GetPlayerSetting(player->GetGUID());

        // 确保 LivesRemaining 在有效范围内
        uint32 remainingLives = std::clamp(playerSettings->LivesRemaining, uint8(1), uint8(TITLE_LENGTH)); // 限制在 1 到 108 之间

        // 根据剩余生命值设置当前头衔
        if (CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(TITLE_START + remainingLives - 1))
        {
            std::stringstream ss;
            ss << Acore::StringFormat("|cffFFFF00你已被梁山好汉 {} 附身！|r", titleEntry->nameMale[4]);
            // send system message
            sHardModeHandler->SendAlert(player, ss.str());
            player->SetCurrentTitle(titleEntry);
        }
    }

    // // 更新玩家的 LivesRemaining 为当前启用模式的最小值
    // if (playerSettings)
    // {
    //     uint8 minLives = UINT8_MAX;
    //     for (uint8 mode : playerSettings->Modes)
    //     {
    //         const HardModeInfo* modeInfo = sHardModeHandler->GetHardModeInfo(mode);
    //         if (modeInfo)
    //         {
    //             minLives = std::min(minLives, modeInfo->MaxLives);
    //         }
    //     }

    //     playerSettings->LivesRemaining = playerSettings->Modes.empty() ? 0 : minLives;
    // }

    if (sHardModeHandler->PlayerHasRestriction(player->GetGUID(), HARDMODE_RESTRICT_SMALLFISH))
    {
        // Schedule due to issues..
        sHardModeHandler->GetScheduler()->Schedule(1s, [player](TaskContext /*task*/)
        {
            sHardModeHandler->UpdatePlayerScaleSpeed(player, SMALLFISH_SCALE);
        });
    }

    sHardModeHandler->ValidatePlayerAuras(player);

    if (sHardModeHandler->IsPlayerShadowBanned(player->GetGUID()))
    {
        sHardModeHandler->UpdatePlayerShadowBanned(player->GetGUID(), false);
        sHardModeHandler->TryShadowBanPlayer(player->GetGUID());
    }
}

void HardModeHooksPlayerScript::OnDelete(ObjectGuid guid, uint32 accountId)
{
    if (!sHardModeHandler->IsHardModeEnabled())
    {
        return;
    }

    sHardModeHandler->DeletePlayerSetting(guid.GetRawValue());
}
