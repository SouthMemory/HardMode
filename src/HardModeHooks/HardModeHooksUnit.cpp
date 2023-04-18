#include "HardModeHooks/HardModeHooksUnit.h"
#include "HardModeHandler.h"

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

    // Don't reapply aura on death, resurrection handles reapplication already.
    if (mode == AURA_REMOVE_BY_DEATH)
    {
        return;
    }

    Player* player = unit->ToPlayer();
    if (!player || !player->IsInWorld())
    {
        return;
    }

    sHardModeHandler->ValidatePlayerAuras(player);
}
