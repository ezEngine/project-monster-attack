#pragma once

#include <Core/Input/Declarations.h>
#include <Core/World/Declarations.h>
#include <GameEngine/GameApplication/GameApplication.h>
#include <GameEngine/GameState/FallbackGameState.h>
#include <GameEngine/GameState/GameState.h>
#include <MonsterAttackPlugin/MonsterAttackPluginDLL.h>
#include <RendererCore/Pipeline/Extractor.h>

class MonsterAttackGameState : public ezFallbackGameState
{
  EZ_ADD_DYNAMIC_REFLECTION(MonsterAttackGameState, ezFallbackGameState);

public:
  MonsterAttackGameState();
  ~MonsterAttackGameState();

  virtual ezGameStatePriority DeterminePriority(ezWorld* pWorld) const override;

  virtual void ProcessInput() override;

  ezSelectedObjectsContext m_ObjectsToHighlight;

  void MonsterReachedGoal();

protected:
  virtual void ConfigureMainWindowInputDevices(ezWindow* pWindow) override;
  virtual void ConfigureInputActions() override;
  virtual void ConfigureMainCamera() override;

private:
  virtual void OnActivation(ezWorld* pWorld, const ezTransform* pStartPosition) override;
  virtual void OnDeactivation() override;
  virtual void BeforeWorldUpdate() override;
  virtual void AfterWorldUpdate() override;

  ezDeque<ezGameObjectHandle> m_SpawnedObjects;

  ezUInt32 m_uiMonstersReachedGoal = 0;
};
