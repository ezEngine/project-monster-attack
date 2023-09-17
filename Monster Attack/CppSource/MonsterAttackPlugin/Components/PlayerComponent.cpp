#include <MonsterAttackPlugin/MonsterAttackPluginPCH.h>

#include <Core/GameState/GameStateBase.h>
#include <Core/Messages/CommonMessages.h>
#include <GameComponentsPlugin/Gameplay/HeadBoneComponent.h>
#include <GameEngine/GameApplication/GameApplication.h>
#include <GameEngine/Gameplay/BlackboardComponent.h>
#include <GameEngine/Gameplay/GrabbableItemComponent.h>
#include <GameEngine/Gameplay/InputComponent.h>
#include <GameEngine/Gameplay/SpawnComponent.h>
#include <GameEngine/Physics/CharacterControllerComponent.h>
#include <MonsterAttackPlugin/Components/PlayerComponent.h>
#include <MonsterAttackPlugin/GameState/MonsterAttackGameState.h>

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(ezPlayerComponent, 1, ezComponentMode::Dynamic)
{
  //EZ_BEGIN_PROPERTIES
  //{
  //}
  //EZ_END_PROPERTIES;
  EZ_BEGIN_MESSAGEHANDLERS
  {
    EZ_MESSAGE_HANDLER(ezMsgInputActionTriggered, OnMsgInputActionTriggered),
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

void ezPlayerComponent::SerializeComponent(ezWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();
}

void ezPlayerComponent::DeserializeComponent(ezWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);

  auto& s = stream.GetStream();
}

void ezPlayerComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();
}

void ezPlayerComponent::OnMsgInputActionTriggered(ezMsgInputActionTriggered& msg)
{
  if (msg.m_TriggerState == ezTriggerState::Continuing)
    return;

  if (msg.m_sInputAction == ezTempHashedString("Shoot") && msg.m_TriggerState == ezTriggerState::Activated)
  {
    if (ezGameObject* pSpawnBulletObj = GetOwner()->FindChildByName("Spawn_MagicBullet", true))
    {
      ezSpawnComponent* pSpawnBulletComp = nullptr;
      if (pSpawnBulletObj->TryGetComponentOfBaseType(pSpawnBulletComp))
      {
        if (pSpawnBulletComp->CanTriggerManualSpawn())
        {
          pSpawnBulletComp->TriggerManualSpawn();
        }
      }
    }
  }
}

void ezPlayerComponent::Update()
{
  ezInputComponent* pInput = nullptr;
  if (!GetOwner()->TryGetComponentOfBaseType(pInput))
    return;

  const ezRTTI* pCharType = ezRTTI::FindTypeByName("ezJoltCharacterControllerComponent");

  ezComponent* pCC = nullptr;
  if (!GetOwner()->TryGetComponentOfBaseType(pCharType, pCC))
    return;

  ezGameObject* pCameraObject = GetOwner()->FindChildByName("Camera", true);

  if (!pCameraObject)
    return;

  ezHeadBoneComponent* pHeadBone = nullptr;
  if (!pCameraObject->TryGetComponentOfBaseType(pHeadBone))
    return;

  // character controller update
  {
    ezMsgMoveCharacterController msg;

    msg.m_bJump = pInput->GetCurrentInputState("Jump", true) > 0.5;
    msg.m_fMoveForwards = pInput->GetCurrentInputState("MoveForwards", false);
    msg.m_fMoveBackwards = pInput->GetCurrentInputState("MoveBackwards", false);
    msg.m_fStrafeLeft = pInput->GetCurrentInputState("StrafeLeft", false);
    msg.m_fStrafeRight = pInput->GetCurrentInputState("StrafeRight", false);
    msg.m_fRotateLeft = pInput->GetCurrentInputState("RotateLeft", false);
    msg.m_fRotateRight = pInput->GetCurrentInputState("RotateRight", false);
    msg.m_bRun = pInput->GetCurrentInputState("Run", false) > 0.5;

    pCC->SendMessage(msg);
  }

  // look up / down
  {
    float up = pInput->GetCurrentInputState("LookUp", false);
    float down = pInput->GetCurrentInputState("LookDown", false);

    pHeadBone->ChangeVerticalRotation(down - up);
  }

  if (ezPhysicsWorldModuleInterface* pPhysics = GetOwner()->GetWorld()->GetModule<ezPhysicsWorldModuleInterface>())
  {
    ezPhysicsQueryParameters params;
    params.m_uiCollisionLayer = 8;
    params.m_ShapeTypes = ezPhysicsShapeType::Static | ezPhysicsShapeType::Dynamic | ezPhysicsShapeType::Query;

    ezPhysicsCastResult result;
    if (pPhysics->Raycast(result, pCameraObject->GetGlobalPosition(), pCameraObject->GetGlobalDirForwards(), 2.0f, params))
    {
      ezGameObject* pActor = nullptr;
      if (GetOwner()->GetWorld()->TryGetObject(result.m_hActorObject, pActor))
      {
        ezGrabbableItemComponent* pGrabbable = nullptr;
        if (pActor->TryGetComponentOfBaseType(pGrabbable))
        {
          if (ezGameStateBase* pGameState = ezGameApplication::GetGameApplicationInstance()->GetActiveGameState())
          {
            MonsterAttackGameState* pTestGameState = ezDynamicCast<MonsterAttackGameState*>(pGameState);
            pTestGameState->m_ObjectsToHighlight.AddObjectAndChildren(*GetWorld(), pActor);
          }
        }
      }
    }
  }
}
