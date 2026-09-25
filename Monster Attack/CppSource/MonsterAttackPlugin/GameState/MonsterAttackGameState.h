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
  virtual void ConfigureMainWindowInputDevices(ezWindow* pWindow) override;
  virtual void ConfigureInputActions() override;
  virtual void ConfigureMainCamera() override;
  virtual void OnChangedMainWorld(ezWorld* pPrevWorld, ezWorld* pNewWorld, ezStringView sStartPosition, const ezTransform& startPositionOffset) override;

  /// Returns nullptr if the world has no main menu. The world must be locked by the caller.
  ezMainMenuComponent* GetMainMenu();
  bool IsMainMenuOpen();

private:
  virtual void OnActivation(ezWorld* pWorld, ezStringView sStartPosition, const ezTransform& startPositionOffset) override;
  virtual void OnDeactivation() override;
  virtual void BeforeWorldUpdate() override;
  virtual void AfterWorldUpdate() override;

  ezSharedPtr<ezBlackboard> m_pLevelState;
  ezDeque<ezGameObjectHandle> m_DeadMonsters;

  ezComponentHandle m_hMainMenu;
  bool m_bSearchedMainMenu = false;
};
