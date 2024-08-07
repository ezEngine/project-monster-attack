#pragma once

#include <Core/Collection/CollectionResource.h>
#include <Core/Input/Declarations.h>
#include <Core/World/Declarations.h>
#include <GameEngine/GameApplication/GameApplication.h>
#include <GameEngine/GameState/FallbackGameState.h>
#include <GameEngine/GameState/GameState.h>
#include <MonsterAttackPlugin/MonsterAttackPluginDLL.h>
#include <RendererCore/Pipeline/Extractor.h>

class MonsterAttackGameState : public ezGameState
{
  EZ_ADD_DYNAMIC_REFLECTION(MonsterAttackGameState, ezGameState);

public:
  MonsterAttackGameState();
  ~MonsterAttackGameState();

  virtual void ProcessInput() override;

  ezSelectedObjectsContext m_ObjectsToHighlight;

  void MonsterReachedGoal();
  void AddDeadMonster(ezGameObjectHandle hObject, ezInt32 iMoneyReward);
  void AddMonster();

protected:
  virtual void ConfigureMainWindowInputDevices(ezWindow* pWindow) override;
  virtual void ConfigureInputActions() override;
  virtual void ConfigureMainCamera() override;

private:
  virtual void OnActivation(ezWorld* pWorld, ezStringView sStartPosition, const ezTransform* pStartPosition) override;
  virtual void OnDeactivation() override;
  virtual void BeforeWorldUpdate() override;
  virtual void AfterWorldUpdate() override;

  ezSharedPtr<ezBlackboard> m_pLevelState;
  ezDeque<ezGameObjectHandle> m_DeadMonsters;
};
