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
#include <GameEngine/StateMachine/StateMachineComponent.h>
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

  ezHashedString sName;
  sName.Assign("LevelState");
  m_pLevelState = ezBlackboard::GetOrCreateGlobal(sName);
}

void ezPlayerComponent::OnMsgInputActionTriggered(ezMsgInputActionTriggered& msg)
{
  if (msg.m_TriggerState == ezTriggerState::Activated)
  {
    if (msg.m_sInputAction == ezTempHashedString("StartRound"))
    {
      ezGameObject* pObj;
      if (GetWorld()->TryGetObjectWithGlobalKey("LevelLogic", pObj))
      {
        ezStateMachineComponent* pSM;
        if (pObj->TryGetComponentOfBaseType(pSM))
        {
          pSM->FireTransitionEvent("StartRound");
        }
      }
      return;
    }
    if (msg.m_sInputAction == ezTempHashedString("Select_MagicBullet"))
    {
      m_Action = PlayerAction::ShootMagicBullet;
      m_TrapPlacement = TrapPlacement::None;
      m_iRequiredMoney = 0;
      ClearPrevizObject();
      return;
    }
    if (msg.m_sInputAction == ezTempHashedString("Select_SpikeTrap"))
    {
      m_Action = PlayerAction::PlaceSpikeTrap;
      m_TrapPlacement = TrapPlacement::Floor;
      m_iRequiredMoney = 1000;
      m_hPrevizPrefab = ezResourceManager::LoadResource<ezPrefabResource>("Vis-Trap-Spike");
      m_hPlacePrefab = ezResourceManager::LoadResource<ezPrefabResource>("Trap-Spike");
      ClearPrevizObject();
      return;
    }
    if (msg.m_sInputAction == ezTempHashedString("Select_ArrowTrap"))
    {
      m_Action = PlayerAction::PlaceArrowTrap;
      m_TrapPlacement = TrapPlacement::Wall;
      m_iRequiredMoney = 2000;
      m_hPrevizPrefab = ezResourceManager::LoadResource<ezPrefabResource>("Vis-Trap-Arrow");
      m_hPlacePrefab = ezResourceManager::LoadResource<ezPrefabResource>("Trap-Arrow");
      ClearPrevizObject();
      return;
    }
    if (msg.m_sInputAction == ezTempHashedString("Select_TarTrap"))
    {
      m_Action = PlayerAction::PlaceTarTrap;
      m_TrapPlacement = TrapPlacement::Floor;
      m_iRequiredMoney = 500;
      m_hPrevizPrefab = ezResourceManager::LoadResource<ezPrefabResource>("Vis-Trap-Tar");
      m_hPlacePrefab = ezResourceManager::LoadResource<ezPrefabResource>("Trap-Tar");
      ClearPrevizObject();
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

  if (m_Action == PlayerAction::PlaceSpikeTrap || m_Action == PlayerAction::PlaceArrowTrap || m_Action == PlayerAction::PlaceTarTrap)
  {
    if (msg.m_sInputAction == ezTempHashedString("Shoot") && msg.m_TriggerState == ezTriggerState::Activated)
    {
      ezInt32 iMoney = m_pLevelState->GetEntry("Money")->m_Value.ConvertTo<ezInt32>();

      if (!m_hPrevizObject.IsInvalidated() && iMoney >= m_iRequiredMoney)
      {
        m_pLevelState->SetEntryValue("Money", iMoney - m_iRequiredMoney);

        ClearPrevizObject();

        ezResourceLock<ezPrefabResource> pPrefab(m_hPlacePrefab, ezResourceAcquireMode::BlockTillLoaded_NeverFail);
        pPrefab->InstantiatePrefab(*GetWorld(), ezTransform::Make(m_vPrevizPosition, m_qPrevizRotation), {});
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

  if (m_Action == PlayerAction::PlaceSpikeTrap || m_Action == PlayerAction::PlaceArrowTrap || m_Action == PlayerAction::PlaceTarTrap)
  {
    if (!DetermineTrapPlacement(pCameraObject))
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

bool ezPlayerComponent::DetermineTrapPlacement(const ezGameObject* pCameraObject)
{
  ezPhysicsWorldModuleInterface* pPhysics = GetOwner()->GetWorld()->GetModule<ezPhysicsWorldModuleInterface>();
  if (!pPhysics)
    return false;

  const ezUInt32 uiColFilter = pPhysics->GetCollisionLayerByName("Interaction Raycast");
  EZ_ASSERT_DEBUG(uiColFilter != ezInvalidIndex, "Collision filter is unknown.");

  ezPhysicsQueryParameters params;
  params.m_bIgnoreInitialOverlap = true;
  params.m_uiCollisionLayer = (ezUInt8)uiColFilter;
  params.m_ShapeTypes = ezPhysicsShapeType::Static;

  ezPhysicsCastResult result;
  if (!pPhysics->Raycast(result, pCameraObject->GetGlobalPosition(), pCameraObject->GetGlobalDirForwards(), 10.0f, params))
    return false;

  m_vPrevizPosition = result.m_vPosition;

  if (m_TrapPlacement == TrapPlacement::Floor)
  {
    // only allow flat floors
    if (!result.m_vNormal.IsEqual(ezVec3(0, 0, 1), 0.05f))
      return false;
  }

  if (m_TrapPlacement == TrapPlacement::Ceiling)
  {
    // only allow flat ceilings
    if (!result.m_vNormal.IsEqual(ezVec3(0, 0, -1), 0.05f))
      return false;
  }

  if (m_TrapPlacement == TrapPlacement::Wall)
  {
    // only allow straight up walls
    if (!ezMath::IsZero(result.m_vNormal.z, 0.01f))
      return false;
  }

  if (m_TrapPlacement == TrapPlacement::Floor || m_TrapPlacement == TrapPlacement::Ceiling)
  {
    // place it at the picked height, but on a 2D grid
    m_vPrevizPosition.x = ezMath::Round(m_vPrevizPosition.x);
    m_vPrevizPosition.y = ezMath::Round(m_vPrevizPosition.y);

    m_qPrevizRotation.SetIdentity();
    // TODO: more validation
  }

  if (m_TrapPlacement == TrapPlacement::Wall)
  {
    // TODO: allowed position needs way more validation (walls at any angle ?)
    m_vPrevizPosition.x = ezMath::Round(m_vPrevizPosition.x);
    m_vPrevizPosition.y = ezMath::Round(m_vPrevizPosition.y);
    m_vPrevizPosition.z = ezMath::Round(m_vPrevizPosition.z);

    m_qPrevizRotation = ezQuat::MakeShortestRotation(ezVec3::MakeAxisX(), result.m_vNormal);
  }

  if (m_hPrevizObject.IsInvalidated())
  {
    ezResourceLock<ezPrefabResource> pPreviz(m_hPrevizPrefab, ezResourceAcquireMode::BlockTillLoaded_NeverFail);

    ezHybridArray<ezGameObject*, 2> root;

    ezPrefabInstantiationOptions opt;
    opt.m_bForceDynamic = true;
    opt.m_pCreatedRootObjectsOut = &root;
    pPreviz->InstantiatePrefab(*GetWorld(), ezTransform::Make(m_vPrevizPosition, m_qPrevizRotation), opt);

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

  return true;
}
