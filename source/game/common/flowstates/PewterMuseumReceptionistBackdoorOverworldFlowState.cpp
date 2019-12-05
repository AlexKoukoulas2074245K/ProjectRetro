//
//  PewterMuseumReceptionistBackdoorOverworldFlowState.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 10/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "PewterMuseumReceptionistBackdoorOverworldFlowState.h"
#include "../components/CursorComponent.h"
#include "../components/DirectionComponent.h"
#include "../components/GuiStateSingletonComponent.h"
#include "../components/PlayerStateSingletonComponent.h"
#include "../utils/TextboxUtils.h"
#include "../../input/components/InputStateSingletonComponent.h"
#include "../../input/utils/InputUtils.h"
#include "../../overworld/components/MovementStateComponent.h"
#include "../../overworld/components/OverworldFlowStateSingletonComponent.h"
#include "../../overworld/utils/OverworldUtils.h"
#include "../../rendering/components/RenderableComponent.h"
#include "../../rendering/utils/AnimationUtils.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const glm::vec3 PewterMuseumReceptionistBackdoorOverworldFlowState::YES_NO_TEXTBOX_POSITION = glm::vec3(0.481498629f, -0.065f, -0.4f);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

PewterMuseumReceptionistBackdoorOverworldFlowState::PewterMuseumReceptionistBackdoorOverworldFlowState(ecs::World& world)
    : BaseOverworldFlowState(world)
{    
    CreateYesNoTextbox(mWorld, YES_NO_TEXTBOX_POSITION);
}

void PewterMuseumReceptionistBackdoorOverworldFlowState::VUpdate(const float)
{
    const auto& guiStateComponent = mWorld.GetSingletonComponent<GuiStateSingletonComponent>();

    // Yes/No textbox active
    if (guiStateComponent.mActiveTextboxesStack.size() == 2)
    {
        const auto& inputStateComponent  = mWorld.GetSingletonComponent<InputStateSingletonComponent>();
        const auto& cursorComponent      = mWorld.GetComponent<CursorComponent>(GetActiveTextboxEntityId(mWorld));
        const auto& playerStateComponent = mWorld.GetSingletonComponent<PlayerStateSingletonComponent>();
        const auto& npcComponent         = mWorld.GetComponent<NpcAiComponent>(GetNpcEntityIdFromLevelIndex(playerStateComponent.mLastNpcLevelIndexSpokenTo, mWorld));

        const auto yesNoTextboxCursorRow = cursorComponent.mCursorRow;

        if (IsActionTypeKeyTapped(VirtualActionType::A_BUTTON, inputStateComponent))
        {
            // Destroy Yes/No textbox
            DestroyActiveTextbox(mWorld);

            // Destroy main chatbox
            DestroyActiveTextbox(mWorld);

            // Yes Selected
            if (yesNoTextboxCursorRow == 0)
            {
                const auto mainChatbox = CreateChatbox(mWorld);
                QueueDialogForChatbox(mainChatbox, npcComponent.mSideDialogs[0], mWorld);
            }
            // No Selected
            else if (yesNoTextboxCursorRow == 1)
            {
                const auto mainChatbox = CreateChatbox(mWorld);
                QueueDialogForChatbox(mainChatbox, npcComponent.mSideDialogs[1], mWorld);
            }
        }
        else if (IsActionTypeKeyTapped(VirtualActionType::B_BUTTON, inputStateComponent))
        {
            // Destroy Yes/No textbox
            DestroyActiveTextbox(mWorld);

            // Destroy main chatbox
            DestroyActiveTextbox(mWorld);

            const auto mainChatbox = CreateChatbox(mWorld);
            QueueDialogForChatbox(mainChatbox, npcComponent.mSideDialogs[1], mWorld);
        }
    }
    else if (GetActiveTextboxEntityId(mWorld) == ecs::NULL_ENTITY_ID)
    {
        CompleteOverworldFlow();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


