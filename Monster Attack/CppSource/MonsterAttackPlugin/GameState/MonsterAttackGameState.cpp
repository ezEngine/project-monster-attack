#include <MonsterAttackPlugin/MonsterAttackPluginPCH.h>

#include <Core/Input/InputManager.h>
#include <Core/System/Window.h>
#include <Core/World/World.h>
#include <Foundation/Configuration/CVar.h>
#include <Foundation/Logging/Log.h>
#include <GameEngine/UI/MainMenuComponent.h>
#include <MonsterAttackPlugin/GameState/MonsterAttackGameState.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Meshes/MeshComponent.h>

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(MonsterAttackGameState, 1, ezRTTIDefaultAllocator<MonsterAttackGameState>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

MonsterAttackGameState::MonsterAttackGameState() = default;
MonsterAttackGameState::~MonsterAttackGameState() = default;

void MonsterAttackGameState::OnActivation(ezWorld* pWorld, ezStringView sStartPosition, const ezTransform& startPositionOffset)
{
  EZ_LOG_BLOCK("GameState::Activate");

  SUPER::OnActivation(pWorld, sStartPosition, startPositionOffset);

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

  if (ezInputManager::GetExclusiveInputSet().IsEmpty() || ezInputManager::GetExclusiveInputSet() == "ezMainMenu")
  {
    if (IsMainMenuOpen())
    {
      // keeps the scene from getting any input while the menu is open
      ezInputManager::SetExclusiveInputSet("ezMainMenu");
      return;
    }
    else if (ezInputManager::GetExclusiveInputSet() == "ezMainMenu")
    {
      ezInputManager::SetExclusiveInputSet("");
    }
  }
}

ezMainMenuComponent* MonsterAttackGameState::GetMainMenu()
{
  if (m_pMainWorld == nullptr || IsInLoadingScreen())
    return nullptr;

  if (!m_bSearchedMainMenu)
  {
    m_bSearchedMainMenu = true;
    m_hMainMenu = ezMainMenuComponent::FindInWorld(*m_pMainWorld);
  }

  ezMainMenuComponent* pMenu = nullptr;
  if (!m_pMainWorld->TryGetComponent(m_hMainMenu, pMenu))
    return nullptr;

  return pMenu;
}

bool MonsterAttackGameState::IsMainMenuOpen()
{
  if (m_pMainWorld == nullptr)
    return false;

  EZ_LOCK(m_pMainWorld->GetWriteMarker());

  ezMainMenuComponent* pMenu = GetMainMenu();
  return pMenu != nullptr && pMenu->IsMenuOpen();
}

void MonsterAttackGameState::RequestQuit(ezStringView sRequestedBy)
{
  if (m_pMainWorld != nullptr && (sRequestedBy == "dev-esc" || sRequestedBy == "editor-esc"))
  {
    EZ_LOCK(m_pMainWorld->GetWriteMarker());

    if (ezMainMenuComponent* pMenu = GetMainMenu())
    {
      pMenu->OpenMenu();
      return;
    }
  }

  SUPER::RequestQuit(sRequestedBy);
}

void MonsterAttackGameState::OnChangedMainWorld(ezWorld* pPrevWorld, ezWorld* pNewWorld, ezStringView sStartPosition, const ezTransform& startPositionOffset)
{
  m_hMainMenu.Invalidate();
  m_bSearchedMainMenu = false;

  SUPER::OnChangedMainWorld(pPrevWorld, pNewWorld, sStartPosition, startPositionOffset);
}

void MonsterAttackGameState::MonsterReachedGoal()
{
  const ezInt32 iPoints = ezMath::Max(0, m_pLevelState->GetEntry("Points")->m_Value.ConvertTo<ezInt32>() - 1);
  m_pLevelState->SetEntryValue("Points", iPoints);

  const ezInt32 iMonsters = ezMath::Max(0, m_pLevelState->GetEntry("Monsters")->m_Value.ConvertTo<ezInt32>() - 1);
  m_pLevelState->SetEntryValue("Monsters", iMonsters);
}

void MonsterAttackGameState::AddDeadMonster(ezGameObjectHandle hObject, ezInt32 iMoneyReward)
{
  m_DeadMonsters.PushBack(hObject);

  const ezInt32 iMoney = m_pLevelState->GetEntry("Money")->m_Value.ConvertTo<ezInt32>() + iMoneyReward;
  m_pLevelState->SetEntryValue("Money", iMoney);

  const ezInt32 iMonsters = ezMath::Max(0, m_pLevelState->GetEntry("Monsters")->m_Value.ConvertTo<ezInt32>() - 1);
  m_pLevelState->SetEntryValue("Monsters", iMonsters);
}

void MonsterAttackGameState::AddMonster()
{
  const ezInt32 iMonsters = m_pLevelState->GetEntry("Monsters")->m_Value.ConvertTo<ezInt32>() + 1;
  m_pLevelState->SetEntryValue("Monsters", iMonsters);
}

void MonsterAttackGameState::ConfigureMainCamera()
{
  SUPER::ConfigureMainCamera();

  // do custom camera setup here
}
