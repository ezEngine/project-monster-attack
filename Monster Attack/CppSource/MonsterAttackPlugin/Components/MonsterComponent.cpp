#include <MonsterAttackPlugin/MonsterAttackPluginPCH.h>

#include <AiPlugin/Navigation/NavMeshWorldModule.h>
#include <AiPlugin/Navigation/Steering.h>
#include <Core/GameState/GameStateBase.h>
#include <Core/Messages/CommonMessages.h>
#include <Core/Messages/SetColorMessage.h>
#include <Core/Physics/SurfaceResource.h>
#include <GameEngine/GameApplication/GameApplication.h>
#include <GameEngine/GameState/GameState.h>
#include <GameEngine/Gameplay/BlackboardComponent.h>
#include <GameEngine/Gameplay/InputComponent.h>
#include <GameEngine/Gameplay/SpawnComponent.h>
#include <GameEngine/Messages/DamageMessage.h>
#include <GameEngine/Physics/CharacterControllerComponent.h>
#include <GameEngine/Physics/CollisionFilter.h>
#include <MonsterAttackPlugin/Components/MonsterComponent.h>
#include <MonsterAttackPlugin/GameState/MonsterAttackGameState.h>

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(ezMonsterComponent, 3, ezComponentMode::Dynamic)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Health", m_iHealthPoints)->AddAttributes(new ezDefaultValueAttribute(100)),
    EZ_MEMBER_PROPERTY("MoneyReward", m_iMoneyReward)->AddAttributes(new ezDefaultValueAttribute(100)),
    EZ_MEMBER_PROPERTY("WalkSpeed", m_fWalkSpeed)->AddAttributes(new ezDefaultValueAttribute(4.0f)),
    EZ_MEMBER_PROPERTY("NavmeshConfig", m_sNavmeshConfig)->AddAttributes(new ezDynamicStringEnumAttribute("AiNavmeshConfig")),
    EZ_MEMBER_PROPERTY("PathSearchConfig", m_sPathSearchConfig)->AddAttributes(new ezDynamicStringEnumAttribute("AiPathSearchConfig")),
  }
  EZ_END_PROPERTIES;
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

  s << m_sNavmeshConfig;
  s << m_sPathSearchConfig;
  s << m_iHealthPoints;
  s << m_fWalkSpeed;
  s << m_iMoneyReward;
}

void ezMonsterComponent::DeserializeComponent(ezWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);
  const ezUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());

  auto& s = stream.GetStream();

  s >> m_sNavmeshConfig;
  s >> m_sPathSearchConfig;

  if (uiVersion >= 2)
  {
    s >> m_iHealthPoints;
    s >> m_fWalkSpeed;
  }

  if (uiVersion >= 3)
  {
    s >> m_iMoneyReward;
  }
}

void ezMonsterComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();

  ezGameObject* pMoveToTarget;
  if (GetWorld()->TryGetObjectWithGlobalKey("Goal", pMoveToTarget))
  {
    m_hMoveToTarget = pMoveToTarget->GetHandle();
  }

  if (MonsterAttackGameState* pGameState = ezDynamicCast<MonsterAttackGameState*>(ezGameApplication::GetGameApplicationInstance()->GetActiveGameState()))
  {
    pGameState->AddMonster();
  }
}

void ezMonsterComponent::Update()
{
  if (m_iHealthPoints <= 0)
    return;

  if (ezAiNavMeshWorldModule* pNavMeshModule = GetWorld()->GetOrCreateModule<ezAiNavMeshWorldModule>())
  {
    m_Navigation.SetNavmesh(*pNavMeshModule->GetNavMesh(m_sNavmeshConfig));
    m_Navigation.SetQueryFilter(pNavMeshModule->GetPathSearchFilter(m_sPathSearchConfig));
  }

  ezGameObject* pMoveToTarget = nullptr;
  if (GetWorld()->TryGetObject(m_hMoveToTarget, pMoveToTarget))
  {
    m_Navigation.SetCurrentPosition(GetOwner()->GetGlobalPosition());
    m_Navigation.SetTargetPosition(pMoveToTarget->GetGlobalPosition());

    if ((pMoveToTarget->GetGlobalPosition() - GetOwner()->GetGlobalPosition()).GetLengthSquared() < ezMath::Square(1.5f))
    {
      // reached the goal

      GetWorld()->DeleteObjectDelayed(GetOwner()->GetHandle());

      if (MonsterAttackGameState* pGameState = ezDynamicCast<MonsterAttackGameState*>(ezGameApplication::GetGameApplicationInstance()->GetActiveGameState()))
      {
        pGameState->MonsterReachedGoal();
      }
    }
  }
  else
  {
    m_Navigation.CancelNavigation();
  }

  m_Navigation.Update();

  const bool bVisualizePathCorridor = false;
  const bool bVisualizePathLine = false;

  if (bVisualizePathCorridor)
  {
    m_Navigation.DebugDrawPathCorridor(GetWorld(), ezColor::Aquamarine.WithAlpha(0.2f));
  }
  if (bVisualizePathLine)
  {
    m_Navigation.DebugDrawPathLine(GetWorld(), ezColor::Lime);
  }

  if (m_Navigation.GetState() != ezAiNavigation::State::FullPathFound)
    return;

  CheckGroundType();

  if (m_fGroundWalkSpeed >= 1.0f)
  {
    ezMsgSetColor msg;
    msg.m_Color = ezColor::White;
    GetOwner()->SendMessageRecursive(msg);
  }
  else
  {
    ezMsgSetColor msg;
    msg.m_Color = ezColor::DarkGrey;
    GetOwner()->SendMessageRecursive(msg);
  }

  if (m_fWalkSpeed > 0)
  {
    ezVec2 vForwardDir = GetOwner()->GetGlobalDirForwards().GetAsVec2();
    vForwardDir.NormalizeIfNotZero(ezVec2(1, 0)).IgnoreResult();

    ezAiSteering steering;

    steering.m_vPosition = GetOwner()->GetGlobalPosition();
    steering.m_qRotation = GetOwner()->GetGlobalRotation();
    steering.m_vVelocity = GetOwner()->GetLinearVelocity();
    steering.m_fMaxSpeed = m_fWalkSpeed * m_fGroundWalkSpeed;
    steering.m_MinTurnSpeed = ezAngle::MakeFromDegree(180);
    steering.m_fAcceleration = 5;
    steering.m_fDecceleration = 10;

    const float fBrakingDistance = 1.2f * (ezMath::Square(steering.m_fMaxSpeed) / (2.0f * steering.m_fDecceleration));

    m_Navigation.ComputeSteeringInfo(steering.m_Info, vForwardDir, fBrakingDistance);
    steering.Calculate(GetWorld()->GetClock().GetTimeDiff().AsFloatInSeconds(), GetWorld());

    steering.m_vPosition.z = m_Navigation.GetCurrentElevation();

    GetOwner()->SetGlobalPosition(steering.m_vPosition);
    GetOwner()->SetGlobalRotation(steering.m_qRotation);

    if (auto pBoard = ezBlackboardComponent::FindBlackboard(GetOwner()))
    {
      pBoard->SetEntryValue("State", 1); // "walk" animation
      pBoard->SetEntryValue("MoveSpeed", ezMath::Clamp(steering.m_vVelocity.GetLength() * 0.5f, 0.0f, 2.0f));
    }
  }
}

void ezMonsterComponent::CheckGroundType()
{
  const ezTime tNow = GetWorld()->GetClock().GetAccumulatedTime();

  if (tNow - m_LastCheckGround < ezTime::MakeFromSeconds(0.1f))
    return;

  m_LastCheckGround = tNow;
  m_fGroundWalkSpeed = 1.0f;

  ezPhysicsWorldModuleInterface* pPhysics = GetOwner()->GetWorld()->GetModule<ezPhysicsWorldModuleInterface>();
  if (!pPhysics)
    return;

  const ezUInt32 uiColFilter = pPhysics->GetCollisionLayerByName("Ground Raycast");
  EZ_ASSERT_DEBUG(uiColFilter != ezInvalidIndex, "Collision filter is unknown.");

  ezPhysicsQueryParameters params;
  params.m_bIgnoreInitialOverlap = true;
  params.m_uiCollisionLayer = (ezUInt8)uiColFilter;
  params.m_ShapeTypes = ezPhysicsShapeType::Static | ezPhysicsShapeType::Query;

  ezPhysicsCastResult result;
  if (!pPhysics->Raycast(result, GetOwner()->GetGlobalPosition() + ezVec3(0, 0, 1), ezVec3(0, 0, -1), 2.0f, params))
    return;

  if (!result.m_hSurface.IsValid())
    return;

  ezResourceLock<ezSurfaceResource> pSurface(result.m_hSurface, ezResourceAcquireMode::AllowLoadingFallback_NeverFail);
  if (pSurface->GetDescriptor().m_iGroundType >= 2)
  {
    m_fGroundWalkSpeed = 0.25f;
  }
}

void ezMonsterComponent::OnMsgDamage(ezMsgDamage& msg)
{
  if (m_iHealthPoints <= 0)
    return;

  const ezInt32 iDamage = (ezInt32)msg.m_fDamage;

  if (iDamage >= 5)
  {
    // loaded the prefab by a "nice name" only works because the prefab is part of a "collection" where it is given that name
    // and the collection is part of our scene (there is a collection component referencing it and registering the names)
    ezPrefabResourceHandle hPrefab = ezResourceManager::LoadResource<ezPrefabResource>("FX-Wound");
    ezResourceLock<ezPrefabResource> pPrefab(hPrefab, ezResourceAcquireMode::BlockTillLoaded);

    ezPrefabInstantiationOptions opt;
    opt.m_RandomSeedMode = ezPrefabInstantiationOptions::RandomSeedMode::CompletelyRandom; // particle effects should be randomized
    pPrefab->InstantiatePrefab(*GetWorld(), GetOwner()->GetGlobalTransform(), opt);
  }

  m_iHealthPoints -= iDamage;

  if (m_iHealthPoints <= 0)
  {
    if (MonsterAttackGameState* pGameState = ezDynamicCast<MonsterAttackGameState*>(ezGameApplication::GetGameApplicationInstance()->GetActiveGameState()))
    {
      pGameState->AddDeadMonster(GetOwner()->GetHandle(), m_iMoneyReward);
    }

    if (auto pBoard = ezBlackboardComponent::FindBlackboard(GetOwner()))
    {
      pBoard->SetEntryValue("State", 2); // "die" animation
    }
  }
}
