#include <Core/World/Component.h>
#include <Core/World/ComponentManager.h>
#include <GameEngine/Gameplay/InputComponent.h>

using ezPlayerComponentManager = ezComponentManagerSimple<class ezPlayerComponent, ezComponentUpdateType::WhenSimulating>;

class ezPlayerComponent : public ezComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezPlayerComponent, ezComponent, ezPlayerComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // ezComponent

public:
  virtual void SerializeComponent(ezWorldWriter& stream) const override;
  virtual void DeserializeComponent(ezWorldReader& stream) override;

protected:
  virtual void OnSimulationStarted() override;

  //////////////////////////////////////////////////////////////////////////
  // ezPlayerComponent

public:
  void OnMsgInputActionTriggered(ezMsgInputActionTriggered& msg);

private:
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
