#include <MonsterAttackPlugin/MonsterAttackPluginPCH.h>

#include <Core/GameApplication/GameApplicationBase.h>
#include <Core/Messages/TriggerMessage.h>
#include <MonsterAttackPlugin/Components/LevelPortalComponent.h>
#include <RendererCore/Components/BlackboardComponent.h>

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(ezLevelPortalComponent, 1, ezComponentMode::Static)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("LevelName", m_sLevelName),
    EZ_MEMBER_PROPERTY("TargetScene", m_sTargetScene)->AddAttributes(new ezAssetBrowserAttribute("CompatibleAsset_Scene", ezDependencyFlags::Package)),
  }
  EZ_END_PROPERTIES;
  EZ_BEGIN_MESSAGEHANDLERS
  {
    EZ_MESSAGE_HANDLER(ezMsgTriggerTriggered, OnMsgTriggerTriggered),
  }
  EZ_END_MESSAGEHANDLERS;
  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("MonsterAttack"),
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_COMPONENT_TYPE;
// clang-format on

ezLevelPortalComponent::ezLevelPortalComponent() = default;
ezLevelPortalComponent::~ezLevelPortalComponent() = default;

void ezLevelPortalComponent::SerializeComponent(ezWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();
  s << m_sLevelName;
  s << m_sTargetScene;
}

void ezLevelPortalComponent::DeserializeComponent(ezWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);

  auto& s = stream.GetStream();
  s >> m_sLevelName;
  s >> m_sTargetScene;
}

void ezLevelPortalComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();

  if (ezGameObject* pCompleted = GetOwner()->FindChildByName("Completed"))
  {
    pCompleted->SetActiveFlag(WasLevelWon(m_sTargetScene));
  }
}

ezSharedPtr<ezBlackboard> ezLevelPortalComponent::GetProgress()
{
  ezHashedString sName;
  sName.Assign("Progress");
  return ezBlackboard::GetOrCreateGlobal(sName);
}

bool ezLevelPortalComponent::WasLevelWon(ezStringView sScene)
{
  ezStringBuilder sKey;
  sKey.SetFormat("Won:{}", sScene);

  return GetProgress()->GetEntryValue(ezTempHashedString(sKey), false).ConvertTo<bool>();
}

void ezLevelPortalComponent::MarkCurrentLevelWon()
{
  ezSharedPtr<ezBlackboard> pProgress = GetProgress();

  const ezString sScene = pProgress->GetEntryValue("CurrentLevel").ConvertTo<ezString>();
  if (sScene.IsEmpty())
    return;

  ezStringBuilder sKey;
  sKey.SetFormat("Won:{}", sScene);
  pProgress->SetEntryValue(sKey, true);
}

void ezLevelPortalComponent::ClearCurrentLevel()
{
  GetProgress()->SetEntryValue("CurrentLevel", "");
}

void ezLevelPortalComponent::OnMsgTriggerTriggered(ezMsgTriggerTriggered& msg)
{
  if (msg.m_sMessage == ezTempHashedString("Approach"))
  {
    if (msg.m_TriggerState == ezTriggerState::Activated)
      ShowLevelName();
    else if (msg.m_TriggerState == ezTriggerState::Deactivated)
      HideLevelName();

    return;
  }

  if (msg.m_sMessage == ezTempHashedString("Enter") && msg.m_TriggerState == ezTriggerState::Activated)
  {
    // the level may get won later, so remember where the player went
    GetProgress()->SetEntryValue("CurrentLevel", m_sTargetScene);

    if (ezGameState* pGameState = ezDynamicCast<ezGameState*>(ezGameApplicationBase::GetGameApplicationBaseInstance()->GetActiveGameState()))
    {
      pGameState->LoadScene(m_sTargetScene, {}, {}, ezTransform::MakeIdentity());
    }
  }
}

void ezLevelPortalComponent::ShowLevelName()
{
  ezSharedPtr<ezBlackboard> pLevelState = ezBlackboardComponent::FindBlackboard(*GetOwner(), "LevelState");
  if (pLevelState == nullptr)
    return;

  ezStringBuilder sText = m_sLevelName;
  if (WasLevelWon(m_sTargetScene))
  {
    sText.Append(" (completed)");
  }

  pLevelState->SetEntryValue("MessageDuration", 0);
  pLevelState->SetEntryValue("Message", sText.GetView());
}

void ezLevelPortalComponent::HideLevelName()
{
  ezSharedPtr<ezBlackboard> pLevelState = ezBlackboardComponent::FindBlackboard(*GetOwner(), "LevelState");
  if (pLevelState == nullptr)
    return;

  // another portal may already show its name
  if (pLevelState->GetEntryValue("Message").ConvertTo<ezString>().StartsWith(m_sLevelName))
  {
    pLevelState->SetEntryValue("Message", "");
  }
}
