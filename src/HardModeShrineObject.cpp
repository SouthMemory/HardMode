#include "HardModeShrineObject.h"
#include "HardModeHandler.h"

#include "Config.h"
#include "Player.h"
#include "ScriptedGossip.h"

bool HardModeShrineObject::OnGossipHello(Player* player, GameObject* go)
{
    if (!sHardModeHandler->IsHardModeEnabled())
    {
        return false;
    }

    auto hardModes = sHardModeHandler->GetHardModes();

    for (auto it = hardModes->begin(); it != hardModes->end(); ++it)
    {
        auto mode = it->second;

        if (!mode.Enabled)
        {
            continue;
        }

        bool flag = sHardModeHandler->IsModeEnabledForPlayer(player->GetGUID(), mode.Id);
        std::string state = flag ? "撤销" : "激活";
        std::string format = Acore::StringFormat("{} {}.", state, mode.Name);

        if (sHardModeHandler->IsPlayerTainted(player->GetGUID()))
        {
            if (flag)
            {
                // 添加确认窗口的格式化信息
                std::string popupFormat = Acore::StringFormat(
                    "Are you sure you want to disable the {} mode?|n|nYou will not be able to re-enable it.",
                    mode.Name);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, format, GOSSIP_SENDER_MAIN, mode.Id, popupFormat, 0, false);
            }
        }
        else
        {
            // 正常启用/禁用模式的确认
            std::string popupFormat = Acore::StringFormat("你是否确定要{} {} ?", state, mode.Name);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, format, GOSSIP_SENDER_MAIN, mode.Id, popupFormat, 0, false);
        }
    }

    if (!sHardModeHandler->IsPlayerTainted(player->GetGUID()))
    {
        SendGossipMenuFor(player, HARDMODE_SHRINE_GREETING, go->GetGUID());
    }
    else
    {
        SendGossipMenuFor(player, HARDMODE_SHRINE_GREETING_TAINTED, go->GetGUID());
    }

    return true;
}

bool HardModeShrineObject::OnGossipSelect(Player *player, GameObject * /*go*/, uint32 /*sender*/, uint32 mode)
{
    if (!sHardModeHandler->IsHardModeEnabled())
    {
        return false;
    }

    HardModeInfo *  modeInfo = sHardModeHandler->GetHardModeFromId(mode);

    bool flag = sHardModeHandler->IsModeEnabledForPlayer(player->GetGUID(), mode);

    if (!flag && sHardModeHandler->IsPlayerTainted(player->GetGUID()))
    {
        // 当玩家受到污染时，发送警告
        sHardModeHandler->SendAlert(player, "你已经无法激活此模式。");
    }
    else
    {
        // 更新玩家模式状态
        sHardModeHandler->UpdateModeForPlayer(player->GetGUID(), mode, !flag);

        // 发送确认消息
        std::string action = flag ? "撤销" : "激活";
        std::string alertMessage = Acore::StringFormat("你已经成功{} {}.", action, modeInfo->Name);
        sHardModeHandler->SendAlert(player, alertMessage);

        if (!flag){
            std::string alertMessage2 = Acore::StringFormat("{}", modeInfo->Description);
            sHardModeHandler->SendAlert(player, alertMessage2);
        }
    }

    CloseGossipMenuFor(player);

    return true;
}
