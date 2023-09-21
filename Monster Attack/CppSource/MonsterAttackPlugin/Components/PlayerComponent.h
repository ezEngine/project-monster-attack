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

  PlayerAction m_Action = PlayerAction::ShootMagicBullet;

  ezGameObjectHandle m_hPrevizObject;
  ezVec3 m_vPrevizPosition = ezVec3::MakeZero();
  ezQuat m_qPrevizRotation = ezQuat::MakeIdentity();

  TrapPlacement m_TrapPlacement = TrapPlacement::None;
  ezPrefabResourceHandle m_hPrevizPrefab;
  ezPrefabResourceHandle m_hPlacePrefab;
};
