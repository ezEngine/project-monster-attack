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
#include <GameEngine/Physics/CollisionFilter.h>
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
  if (msg.m_TriggerState == ezTriggerState::Activated)
  {
    if (msg.m_sInputAction == ezTempHashedString("Select_MagicBullet"))
    {
      m_Action = PlayerAction::ShootMagicBullet;
      return;
    }
    if (msg.m_sInputAction == ezTempHashedString("Select_SpikeTrap"))
    {
      m_Action = PlayerAction::PlaceSpikeTrap;
      return;
    }
  }

  if (m_Action == PlayerAction::ShootMagicBullet)
  {
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

  if (m_Action == PlayerAction::PlaceSpikeTrap)
  {
    if (msg.m_sInputAction == ezTempHashedString("Shoot") && msg.m_TriggerState == ezTriggerState::Activated)
    {
      if (!m_hPrevizObject.IsInvalidated())
      {
        ClearPrevizObject();

        ezPrefabResourceHandle hPrefab = ezResourceManager::LoadResource<ezPrefabResource>("Trap-Spike");
        ezResourceLock<ezPrefabResource> pPrefab(hPrefab, ezResourceAcquireMode::BlockTillLoaded_NeverFail);
        pPrefab->InstantiatePrefab(*GetWorld(), ezTransform::Make(m_vPrevizPosition), {});
      }
    }
  }
}

void ezPlayerComponent::Update()
{
  ezInputComponent* pInput = nullptr;
  if (!GetOwner()->TryGetComponentOfBaseType(pInput))
    return;

  ezGameObject* pCameraObject = GetOwner()->FindChildByName("Camera", true);
  if (!pCameraObject)
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

    GetOwner()->SendMessage(msg);
  }

  // look up / down
  ezHeadBoneComponent* pHeadBone = nullptr;
  if (pCameraObject->TryGetComponentOfBaseType(pHeadBone))
  {
    float up = pInput->GetCurrentInputState("LookUp", false);
    float down = pInput->GetCurrentInputState("LookDown", false);

    pHeadBone->ChangeVerticalRotation(down - up);
  }

  if (m_Action == PlayerAction::ShootMagicBullet)
  {
    ClearPrevizObject();
  }

  ezPhysicsWorldModuleInterface* pPhysics = GetOwner()->GetWorld()->GetModule<ezPhysicsWorldModuleInterface>();
  if (!pPhysics)
    return;

  if (m_Action == PlayerAction::PlaceSpikeTrap)
  {
    const ezInt32 iColFilter = pPhysics->GetCollisionFilterConfig().GetFilterGroupByName("Interaction Raycast");
    EZ_ASSERT_DEBUG(iColFilter >= 0, "Collision filter is unknown.");

    ezPhysicsQueryParameters params;
    params.m_bIgnoreInitialOverlap = true;
    params.m_uiCollisionLayer = (ezUInt8)iColFilter;
    params.m_ShapeTypes = ezPhysicsShapeType::Static | ezPhysicsShapeType::Dynamic | ezPhysicsShapeType::Query;

    ezPhysicsCastResult result;
    if (pPhysics->Raycast(result, pCameraObject->GetGlobalPosition(), pCameraObject->GetGlobalDirForwards(), 10.0f, params))
    {
      m_vPrevizPosition = result.m_vPosition;

      m_vPrevizPosition.x = ezMath::Round(m_vPrevizPosition.x);
      m_vPrevizPosition.y = ezMath::Round(m_vPrevizPosition.y);
      m_vPrevizPosition.z = ezMath::Round(m_vPrevizPosition.z);

      if (m_hPrevizObject.IsInvalidated())
      {
        ezPrefabResourceHandle hPreviz = ezResourceManager::LoadResource<ezPrefabResource>("Vis-Trap-Spike");
        ezResourceLock<ezPrefabResource> pPreviz(hPreviz, ezResourceAcquireMode::BlockTillLoaded_NeverFail);

        ezHybridArray<ezGameObject*, 2> root;

        ezPrefabInstantiationOptions opt;
        opt.m_bForceDynamic = true;
        opt.m_pCreatedRootObjectsOut = &root;
        pPreviz->InstantiatePrefab(*GetWorld(), ezTransform::Make(m_vPrevizPosition), opt);

        if (!root.IsEmpty())
        {
          m_hPrevizObject = root[0]->GetHandle();
        }
      }
      else
      {
        ezGameObject* pPreviz = nullptr;
        if (GetWorld()->TryGetObject(m_hPrevizObject, pPreviz))
        {
          pPreviz->SetGlobalPosition(m_vPrevizPosition);
        }
      }
    }
    else
    {
      ClearPrevizObject();
    }
  }
}

void ezPlayerComponent::ClearPrevizObject()
{
  if (m_hPrevizObject.IsInvalidated())
    return;

  GetWorld()->DeleteObjectDelayed(m_hPrevizObject);
  m_hPrevizObject.Invalidate();
}
