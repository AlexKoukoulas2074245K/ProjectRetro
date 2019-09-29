//
//  PokeMartTransactionDialogOverworldFlowState.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 27/09/2019.
//

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef PokeMartTransactionDialogOverworldFlowState_h
#define PokeMartTransactionDialogOverworldFlowState_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "BaseFlowState.h"
#include "../../common/utils/MathUtils.h"
#include "../../common/utils/StringUtils.h"
#include "../../ECS.h"

#include <string>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class PokeMartTransactionDialogOverworldFlowState final: public BaseFlowState
{
public:
    PokeMartTransactionDialogOverworldFlowState(ecs::World&);
    
    void VUpdate(const float dt) override;
    
private:
    void UpdateIntroFlow();
    void UpdateItemMenu(const float dt);
    void UpdateItemQuantityMenu();
    void UpdateConfirmationDialog();
    void UpdateConfirmationYesNoDialog();
    void UpdateNotEnoughMoneyFlow();
    void UpdateUniqueItemSaleFlow();
    void UpdateSuccessfulPurchaseFlow();
    void UpdateCancellingFlow();
    void CancelDialog();
    void CreateAndPopulateItemMenu();
    void RedrawItemMenu();
    void SaveLastFramesCursorRow();    
    void DisplayItemsInMenuForCurrentOffset() const;
    
    static const glm::vec3 YES_NO_TEXTBOX_POSITION;
    static const std::string TEXTBOX_CLICK_SFX_NAME;
    static const std::string PURCHASE_SFX_NAME;
    static const float FIRST_OVERLAID_CHATBOX_Z;
    static const float SECOND_OVERLAID_CHATBOX_Z;
    static const int MIN_ITEM_QUANTITY;
    static const int MAX_ITEM_QUANTITY;
    
    enum class BuyDialogState
    {
        INTRO,
        ITEM_MENU,
        ITEM_QUANTITY,
        CONFIRMATION,
        CONFIRMATION_YES_NO,
        NOT_ENOUGH_MONEY,
        UNIQUE_ITEM_SALE,
        SUCCESSFUL_PURCHASE,
        CANCELLING
    };
    
    StringId mSelectedItemName;
    BuyDialogState mBuyDialogState;
    int mBagItemIndex;
    int mSelectedQuantity;
    
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* PokeMartTransactionDialogOverworldFlowState_h */
