#include <MonsterAttackPlugin/MonsterAttackPluginPCH.h>

#include <Core/Input/InputManager.h>
#include <Core/System/Window.h>
#include <Core/World/World.h>
#include <Foundation/Configuration/CVar.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
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

  UpdateHudState();
}

void MonsterAttackGameState::UpdateHudState()
{
  // the HUD only binds to entries that exist when it is created, so we only update entries that the level's blackboard template provides
  if (m_pLevelState == nullptr || !m_pLevelState->HasEntry("MenuOpen"))
    return;

  m_pLevelState->SetEntryValue("MenuOpen", IsMainMenuOpen());

  // the game logic decides how long a message stays, 0 means until it gets replaced
  if (const ezBlackboard::Entry* pMessage = m_pLevelState->GetEntry("Message"))
  {
    const ezTime tNow = m_pMainWorld->GetClock().GetAccumulatedTime();

    if (pMessage->m_uiChangeCounter != m_uiMessageChangeCounter)
    {
      m_uiMessageChangeCounter = pMessage->m_uiChangeCounter;

      const ezBlackboard::Entry* pDuration = m_pLevelState->GetEntry("MessageDuration");
      const float fDuration = pDuration ? pDuration->m_Value.ConvertTo<float>() : 0.0f;
      m_MessageHideTime = fDuration > 0.0f ? tNow + ezTime::MakeFromSeconds(fDuration) : ezTime::MakeZero();
    }

    const bool bTimedOut = m_MessageHideTime.IsPositive() && tNow >= m_MessageHideTime;
    m_pLevelState->SetEntryValue("MessageVisible", !pMessage->m_Value.ConvertTo<ezString>().IsEmpty() && !bTimedOut);
  }

  // show the prompts for whatever device was used last
  {
    const ezStringView sPressed = ezInputManager::GetPressedInputSlot(ezInputSlotFlags::None, ezInputSlotFlags::None);

    if (sPressed.StartsWith("controller"))
      m_bUsingController = true;
    else if (sPressed.StartsWith("keyboard_") || sPressed.StartsWith("mouse_"))
      m_bUsingController = false;

    m_pLevelState->SetEntryValue("UsingController", m_bUsingController);
  }

  struct HudKey
  {
    const char* m_szAction;
    bool m_bPreferMouse;
  };

  // clang-format off
  static constexpr HudKey s_HudKeys[] = {
    {"Select_MagicBullet", false},
    {"Select_SpikeTrap",   false},
    {"Select_ArrowTrap",   false},
    {"Select_TarTrap",     false},
    {"Select_Prev",        false},
    {"Select_Next",        false},
    {"StartRound",         false},
    {"Shoot",              true},
  };
  // clang-format on

  ezStringBuilder sKey, sRml, sName;

  for (const HudKey& key : s_HudKeys)
  {
    // keys can be rebound in the main menu, so always show what is currently configured
    const ezInputActionConfig cfg = ezInputManager::GetInputActionConfig("Default", key.m_szAction);

    ezStringView sSlot;
    for (ezUInt32 i = 0; i < ezInputActionConfig::MaxInputSlotAlternatives; ++i)
    {
      const ezStringView sTrigger = cfg.m_sInputSlotTrigger[i];

      if (m_bUsingController)
      {
        if (sTrigger.StartsWith("controller"))
        {
          sSlot = sTrigger;
          break;
        }

        continue;
      }

      if (sTrigger.StartsWith("mouse_") && key.m_bPreferMouse)
      {
        sSlot = sTrigger;
        break;
      }

      if (sSlot.IsEmpty() && (sTrigger.StartsWith("keyboard_") || sTrigger.StartsWith("mouse_")))
      {
        sSlot = sTrigger;

        if (!key.m_bPreferMouse)
          break;
      }
    }

    sName = sSlot.IsEmpty() ? ezStringView("-") : ezInputManager::GetInputSlotDisplayName(sSlot);

    // the HUD inserts this as RML, either an input prompt image or the name of the key
    const ezStringView sIcon = GetInputPromptIcon(sSlot);
    if (!sIcon.IsEmpty())
    {
      sRml.SetFormat("<img class=\"key-icon\" src=\"{}\" />", sIcon);
    }
    else
    {
      sRml = sName;
      sRml.ReplaceAll("&", "&amp;");
      sRml.ReplaceAll("<", "&lt;");
      sRml.ReplaceAll(">", "&gt;");
      sRml.PrependFormat("<span class=\"key-text\">");
      sRml.Append("</span>");
    }

    sKey.SetFormat("Key_{}", key.m_szAction);
    m_pLevelState->SetEntryValue(sKey, sName.GetView());

    sKey.SetFormat("KeyRml_{}", key.m_szAction);
    m_pLevelState->SetEntryValue(sKey, sRml.GetView());
  }
}

ezStringView MonsterAttackGameState::GetInputPromptIcon(ezStringView sInputSlot)
{
  if (sInputSlot.IsEmpty())
    return {};

  bool bExisted = false;
  auto it = m_InputPromptIcons.FindOrAdd(sInputSlot, &bExisted);

  if (!bExisted)
  {
    // the icons are named after the input slots, but not every slot has one
    ezStringBuilder sPath;
    sPath.SetFormat("Kenney.nl/InputPrompts/{}.png", sInputSlot);

    if (ezFileSystem::ExistsFile(sPath))
    {
      it.Value() = sPath;
    }
  }

  return it.Value();
}

void MonsterAttackGameState::ConfigureMainWindowInputDevices(ezWindowPlatformShared* pWindow)
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
  m_uiMessageChangeCounter = ezInvalidIndex;

  SUPER::OnChangedMainWorld(pPrevWorld, pNewWorld, sStartPosition, startPositionOffset);
}

void MonsterAttackGameState::MonsterReachedGoal()
{
  const ezInt32 iLives = ezMath::Max(0, m_pLevelState->GetEntry("Lives")->m_Value.ConvertTo<ezInt32>() - 1);
  m_pLevelState->SetEntryValue("Lives", iLives);

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
