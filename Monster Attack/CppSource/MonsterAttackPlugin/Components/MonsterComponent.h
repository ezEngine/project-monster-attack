#pragma once

#include <Core/World/Component.h>
#include <Core/World/ComponentManager.h>
#include <MonsterAttackPlugin/MonsterAttackPluginDLL.h>

struct ezMsgDamage;

using ezMonsterComponentManager = ezComponentManagerSimple<class ezMonsterComponent, ezComponentUpdateType::WhenSimulating>;

/// The game logic of a monster.
///
/// The monster walks to the object with the global key "Goal" using an ezAiNavigationComponent.
/// It walks slower over ground with a surface ground type of 2 or higher (tar).
/// When it receives enough damage, it turns into a ragdoll and the player gets the money reward.
class EZ_MONSTERATTACKPLUGIN_DLL ezMonsterComponent : public ezComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezMonsterComponent, ezComponent, ezMonsterComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // ezComponent

public:
  virtual void SerializeComponent(ezWorldWriter& stream) const override;
  virtual void DeserializeComponent(ezWorldReader& stream) override;

protected:
  virtual void OnSimulationStarted() override;

  //////////////////////////////////////////////////////////////////////////
  // ezMonsterComponent

private:
  void Update();
  void CheckGroundType();

  void OnMsgDamage(ezMsgDamage& msg);

  ezInt32 m_iHealthPoints = 100; // [ property ]
  ezInt32 m_iMoneyReward = 100;  // [ property ]

  float m_fGroundWalkSpeed = 1.0f; // speed factor of the ground below the monster (e.g. tar slows it down)
  float m_fBaseWalkSpeed = 0.0f;   // the speed configured on the navigation component
  bool m_bSlowedDown = false;
  ezTime m_LastCheckGround;

  ezGameObjectHandle m_hMoveToTarget;
};
