#include <MonsterAttackPlugin/MonsterAttackPluginPCH.h>

#include <Core/Input/InputManager.h>
#include <Core/System/Window.h>
#include <Core/World/World.h>
#include <Foundation/Configuration/CVar.h>
#include <Foundation/Logging/Log.h>
#include <MonsterAttackPlugin/GameState/MonsterAttackGameState.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Meshes/MeshComponent.h>

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(MonsterAttackGameState, 1, ezRTTIDefaultAllocator<MonsterAttackGameState>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

MonsterAttackGameState::MonsterAttackGameState() = default;
MonsterAttackGameState::~MonsterAttackGameState() = default;

void MonsterAttackGameState::OnActivation(ezWorld* pWorld, const ezTransform* pStartPosition)
{
  EZ_LOG_BLOCK("GameState::Activate");

  SUPER::OnActivation(pWorld, pStartPosition);

  ezView* pView = nullptr;
  if (ezRenderWorld::TryGetView(m_hMainView, pView))
  {
    pView->SetExtractorProperty("HighlightObjects", "SelectionContext", &m_ObjectsToHighlight);
  }

  ezGameApplication::cvar_AppVSync = true;

  ezHashedString sName;
  sName.Assign("LevelState");
  m_pLevelState = ezBlackboard::GetOrCreateGlobal(sName);
}

void MonsterAttackGameState::OnDeactivation()
{
  EZ_LOG_BLOCK("GameState::Deactivate");

  SUPER::OnDeactivation();
}

void MonsterAttackGameState::AfterWorldUpdate()
{
  SUPER::AfterWorldUpdate();
}

void MonsterAttackGameState::BeforeWorldUpdate()
{
  EZ_LOCK(m_pMainWorld->GetWriteMarker());

  while (m_DeadMonsters.GetCount() > 20)
  {
    m_pMainWorld->DeleteObjectDelayed(m_DeadMonsters.PeekFront());

    m_DeadMonsters.PopFront();
  }
}

ezGameStatePriority MonsterAttackGameState::DeterminePriority(ezWorld* pWorld) const
{
  return ezGameStatePriority::Default;
}

void MonsterAttackGameState::ConfigureMainWindowInputDevices(ezWindow* pWindow)
{
  SUPER::ConfigureMainWindowInputDevices(pWindow);

  // setup devices here
}

static void RegisterInputAction(const char* szInputSet, const char* szInputAction, const char* szKey1, const char* szKey2 = nullptr, const char* szKey3 = nullptr)
{
  ezInputActionConfig cfg;
  cfg.m_bApplyTimeScaling = true;
  cfg.m_sInputSlotTrigger[0] = szKey1;
  cfg.m_sInputSlotTrigger[1] = szKey2;
  cfg.m_sInputSlotTrigger[2] = szKey3;

  ezInputManager::SetInputActionConfig(szInputSet, szInputAction, cfg, true);
}

void MonsterAttackGameState::ConfigureInputActions()
{
  SUPER::ConfigureInputActions();
}

void MonsterAttackGameState::ProcessInput()
{
  SUPER::ProcessInput();

  ezWorld* pWorld = m_pMainWorld;

  m_ObjectsToHighlight.m_Objects.Clear();
}

void MonsterAttackGameState::MonsterReachedGoal()
{
  const ezInt32 iPoints = ezMath::Max(0, m_pLevelState->GetEntry("Points")->m_Value.ConvertTo<ezInt32>() - 1);
  m_pLevelState->SetEntryValue("Points", iPoints).AssertSuccess();

  const ezInt32 iMonsters = ezMath::Max(0, m_pLevelState->GetEntry("Monsters")->m_Value.ConvertTo<ezInt32>() - 1);
  m_pLevelState->SetEntryValue("Monsters", iMonsters).AssertSuccess();
}

void MonsterAttackGameState::AddDeadMonster(ezGameObjectHandle hObject, ezInt32 iMoneyReward)
{
  m_DeadMonsters.PushBack(hObject);

  const ezInt32 iMoney = m_pLevelState->GetEntry("Money")->m_Value.ConvertTo<ezInt32>() + iMoneyReward;
  m_pLevelState->SetEntryValue("Money", iMoney).AssertSuccess();

  const ezInt32 iMonsters = ezMath::Max(0, m_pLevelState->GetEntry("Monsters")->m_Value.ConvertTo<ezInt32>() - 1);
  m_pLevelState->SetEntryValue("Monsters", iMonsters).AssertSuccess();
}

void MonsterAttackGameState::AddMonster()
{
  const ezInt32 iMonsters = m_pLevelState->GetEntry("Monsters")->m_Value.ConvertTo<ezInt32>() + 1;
  m_pLevelState->SetEntryValue("Monsters", iMonsters).AssertSuccess();
}

void MonsterAttackGameState::ConfigureMainCamera()
{
  SUPER::ConfigureMainCamera();

  // do custom camera setup here
}
