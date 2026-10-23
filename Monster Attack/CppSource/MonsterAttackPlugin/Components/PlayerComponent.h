#pragma once

#include <Core/World/Component.h>
#include <Core/World/ComponentManager.h>
#include <GameEngine/Gameplay/InputComponent.h>
#include <MonsterAttackPlugin/MonsterAttackPluginDLL.h>

using ezPlayerComponentManager = ezComponentManagerSimple<class ezPlayerComponent, ezComponentUpdateType::WhenSimulating>;

/// The game logic of the player.
///
/// Reads the input from an ezInputComponent on the same object and forwards movement to the character controller.
/// The player can switch between the magic gun and traps. While a trap is selected, a preview of it is shown
/// where the player looks at, and shooting places the trap there, if the player has enough money.
class EZ_MONSTERATTACKPLUGIN_DLL ezPlayerComponent : public ezComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezPlayerComponent, ezComponent, ezPlayerComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // ezComponent

protected:
  virtual void OnSimulationStarted() override;

  //////////////////////////////////////////////////////////////////////////
  // ezPlayerComponent

private:
  void OnMsgInputActionTriggered(ezMsgInputActionTriggered& msg);

  void Update();
  void ClearPrevizObject();
  bool DetermineTrapPlacement(const ezGameObject* pCameraObject);

  void SelectItem(ezUInt32 uiItem);
  void PlaceTrap();
  ezInt32 GetItemCost(ezUInt32 uiItem) const;

  /// Shows a short message on the HUD, e.g. when the player can't afford a trap.
  void ShowNotice(ezStringView sText);

  enum class PlayerAction
  {
    ShootMagicBullet,
    PlaceSpikeTrap,
    PlaceArrowTrap,
    PlaceTarTrap,
  };

  enum class TrapPlacement
  {
    None,
    Floor,
    Wall,
    Ceiling,
  };

  struct ItemDesc
  {
    PlayerAction m_Action;
    TrapPlacement m_Placement;
    const char* m_szSelectAction; ///< Input action that selects this item.
    const char* m_szCostKey;      ///< Blackboard entry that holds the price, nullptr if the item is free.
    ezInt32 m_iDefaultCost;       ///< Used when the blackboard has no price entry.
    const char* m_szPrevizPrefab;
    const char* m_szPlacePrefab;
  };

  static const ItemDesc s_Items[];
  static const ezUInt32 s_uiNumItems;

  ezUInt32 m_uiActiveItem = 0;

  ezGameObjectHandle m_hPrevizObject;
  ezVec3 m_vPrevizPosition = ezVec3::MakeZero();
  ezQuat m_qPrevizRotation = ezQuat::MakeIdentity();

  ezPrefabResourceHandle m_hPrevizPrefab;
  ezPrefabResourceHandle m_hPlacePrefab;
  ezSharedPtr<ezBlackboard> m_pLevelState;

  ezTime m_NoticeEndTime;
};
