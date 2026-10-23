#pragma once

#include <Core/World/Component.h>
#include <Core/World/ComponentManager.h>
#include <MonsterAttackPlugin/MonsterAttackPluginDLL.h>

struct ezMsgTriggerTriggered;

using ezLevelPortalComponentManager = ezComponentManager<class ezLevelPortalComponent, ezBlockStorageType::Compact>;

/// A doorway in the hub that leads into a level.
///
/// The component reacts to trigger messages from child objects:
///   "Approach": Shows the name of the level on the HUD while the player stands in front of the portal.
///   "Enter":    Loads the level.
///
/// Which levels were won is stored in the global 'Progress' blackboard.
/// If the target level was won, a child object named "Completed" gets activated.
class EZ_MONSTERATTACKPLUGIN_DLL ezLevelPortalComponent : public ezComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezLevelPortalComponent, ezComponent, ezLevelPortalComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // ezComponent

public:
  virtual void SerializeComponent(ezWorldWriter& stream) const override;
  virtual void DeserializeComponent(ezWorldReader& stream) override;

protected:
  virtual void OnSimulationStarted() override;

  //////////////////////////////////////////////////////////////////////////
  // ezLevelPortalComponent

public:
  ezLevelPortalComponent();
  ~ezLevelPortalComponent();

  ezString m_sLevelName;   // [ property ]
  ezString m_sTargetScene; // [ property ]

  /// Returns whether the level with the given scene was won before.
  static bool WasLevelWon(ezStringView sScene);

  /// Remembers that the level that was entered last was won.
  static void MarkCurrentLevelWon();

  /// Forgets which level was entered last, e.g. when going back to the hub.
  static void ClearCurrentLevel();

private:
  void OnMsgTriggerTriggered(ezMsgTriggerTriggered& msg);

  void ShowLevelName();
  void HideLevelName();

  static ezSharedPtr<ezBlackboard> GetProgress();
};
