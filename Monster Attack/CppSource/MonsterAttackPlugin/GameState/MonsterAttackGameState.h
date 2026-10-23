#pragma once

#include <Core/Utils/Blackboard.h>
#include <Core/World/Declarations.h>
#include <GameEngine/GameState/GameState.h>
#include <MonsterAttackPlugin/MonsterAttackPluginDLL.h>

class ezMainMenuComponent;

/// The game state of Monster Attack.
///
/// The game state lives across scene changes. It owns everything that isn't part of a single level:
///   * Starting in the hub (Scenes/Main) and going back there once a level is won or lost.
///   * Opening the main menu on ESC, instead of quitting.
///   * Writing data for the HUD into the global 'LevelState' blackboard (e.g. the current key bindings).
///   * Bookkeeping for monsters (money rewards, lives lost, cleaning up dead bodies).
class EZ_MONSTERATTACKPLUGIN_DLL MonsterAttackGameState : public ezGameState
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
  /// Loads the scene given with "-scene" (e.g. when launched from the editor), otherwise the hub in Scenes/Main.ezScene.
  virtual void GetStartupOptions(ezString& out_sScene, ezString& out_sPreloadCollection) override;
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
  virtual void BeforeWorldUpdate() override;

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
