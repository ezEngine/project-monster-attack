#include <MonsterAttackPlugin/MonsterAttackPluginPCH.h>

#include <Core/GameState/GameStateBase.h>
#include <Core/Messages/CommonMessages.h>
#include <GameEngine/GameApplication/GameApplication.h>
#include <GameEngine/Gameplay/BlackboardComponent.h>
#include <GameEngine/Gameplay/InputComponent.h>
#include <GameEngine/Gameplay/SpawnComponent.h>
#include <GameEngine/Physics/CharacterControllerComponent.h>
#include <GameEngine\Messages\DamageMessage.h>
#include <MonsterAttackPlugin/Components/MonsterComponent.h>
#include <MonsterAttackPlugin/GameState/MonsterAttackGameState.h>

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(ezMonsterComponent, 1, ezComponentMode::Dynamic)
{
  //EZ_BEGIN_PROPERTIES
  //{
  //}
  //EZ_END_PROPERTIES;
  EZ_BEGIN_MESSAGEHANDLERS
  {
    EZ_MESSAGE_HANDLER(ezMsgDamage, OnMsgDamage),
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

void ezMonsterComponent::SerializeComponent(ezWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();
}

void ezMonsterComponent::DeserializeComponent(ezWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);

  auto& s = stream.GetStream();
}

void ezMonsterComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();
}

void ezMonsterComponent::Update()
{
  // const ezRTTI* pCharType = ezRTTI::FindTypeByName("ezJoltCharacterControllerComponent");

  // ezComponent* pCC = nullptr;
  // if (!GetOwner()->TryGetComponentOfBaseType(pCharType, pCC))
  //   return;

  // character controller update
  {
    // ezMsgMoveCharacterController msg;
    // msg.m_bJump = pInput->GetCurrentInputState("Jump", true) > 0.5;
    // msg.m_fMoveForwards = pInput->GetCurrentInputState("MoveForwards", false);
    // msg.m_fMoveBackwards = pInput->GetCurrentInputState("MoveBackwards", false);
    // msg.m_fStrafeLeft = pInput->GetCurrentInputState("StrafeLeft", false);
    // msg.m_fStrafeRight = pInput->GetCurrentInputState("StrafeRight", false);
    // msg.m_fRotateLeft = pInput->GetCurrentInputState("RotateLeft", false);
    // msg.m_fRotateRight = pInput->GetCurrentInputState("RotateRight", false);
    // msg.m_bRun = pInput->GetCurrentInputState("Run", false) > 0.5;
    // pCC->SendMessage(msg);
  }
}

void ezMonsterComponent::OnMsgDamage(ezMsgDamage& msg)
{
  if (m_iHealthPoints <= 0)
    return;

  m_iHealthPoints -= (ezInt32)msg.m_fDamage;

  if (m_iHealthPoints <= 0)
  {
    if (auto pBoard = ezBlackboardComponent::FindBlackboard(GetOwner()))
    {
      pBoard->SetEntryValue("State", 2).IgnoreResult(); // "die" animation
    }
  }
}
