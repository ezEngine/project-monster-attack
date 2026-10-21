#pragma once

#include <Core/Collection/CollectionResource.h>
#include <Core/Input/Declarations.h>
#include <Core/World/Declarations.h>
#include <GameEngine/GameApplication/GameApplication.h>
#include <GameEngine/GameState/FallbackGameState.h>
#include <GameEngine/GameState/GameState.h>
#include <MonsterAttackPlugin/MonsterAttackPluginDLL.h>
#include <RendererCore/Pipeline/Extractor.h>

class ezMainMenuComponent;

class MonsterAttackGameState : public ezGameState
{
  EZ_ADD_DYNAMIC_REFLECTION(MonsterAttackGameState, ezGameState);

public:
  MonsterAttackGameState();
  ~MonsterAttackGameState();

  virtual void ProcessInput() override;

  /// If the world contains an ezMainMenuComponent, pressing ESC ("dev-esc" and "editor-esc") opens it instead of quitting.
  virtual void RequestQuit(ezStringView sRequestedBy) override;

  void MonsterReachedGoal();
  void AddDeadMonster(ezGameObjectHandle hObject, ezInt32 iMoneyReward);
  void AddMonster();

protected:
  virtual void ConfigureMainWindowInputDevices(ezWindowPlatformShared* pWindow) override;
  virtual void ConfigureInputActions() override;
  virtual void ConfigureMainCamera() override;
  virtual void OnChangedMainWorld(ezWorld* pPrevWorld, ezWorld* pNewWorld, ezStringView sStartPosition, const ezTransform& startPositionOffset) override;

  /// Returns nullptr if the world has no main menu. The world must be locked by the caller.
  ezMainMenuComponent* GetMainMenu();
  bool IsMainMenuOpen();

  /// Writes the state that the HUD displays, but isn't owned by the game logic (e.g. key bindings) into the level blackboard.
  void UpdateHudState();

  /// Returns the path to the Kenney input prompt image for the given input slot, or an empty string if there is none.
  ezStringView GetInputPromptIcon(ezStringView sInputSlot);

  /// Once a level is won or lost, goes back to the hub after a short delay.
  ///
  /// Levels define where to return to through an ezSceneTransitionComponent on an object with the global key "LevelExit".
  /// Levels without such an object just stay in the final phase.
  void UpdateLevelExit();

private:
  virtual void OnActivation(ezWorld* pWorld, ezStringView sStartPosition, const ezTransform& startPositionOffset) override;
  virtual void OnDeactivation() override;
  virtual void BeforeWorldUpdate() override;
  virtual void AfterWorldUpdate() override;

  ezSharedPtr<ezBlackboard> m_pLevelState;
  ezDeque<ezGameObjectHandle> m_DeadMonsters;

  ezComponentHandle m_hMainMenu;
  bool m_bSearchedMainMenu = false;

  ezMap<ezString, ezString> m_InputPromptIcons;
  bool m_bUsingController = false;

  ezUInt32 m_uiMessageChangeCounter = ezInvalidIndex;
  ezTime m_MessageHideTime;

  ezUInt32 m_uiPhaseChangeCounter = ezInvalidIndex;
  ezTime m_LevelExitTime;
  ezInt32 m_iLevelExitCountdown = 0;
};
