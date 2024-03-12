#include <MonsterAttackPlugin/MonsterAttackPluginPCH.h>

#include <AiPlugin/Navigation/Components/NavigationComponent.h>
#include <AiPlugin/Navigation/NavMeshWorldModule.h>
#include <AiPlugin/Navigation/Steering.h>
#include <Core/GameState/GameStateBase.h>
#include <Core/Messages/CommonMessages.h>
#include <Core/Messages/SetColorMessage.h>
#include <Core/Physics/SurfaceResource.h>
#include <GameEngine/Animation/Skeletal/AnimationControllerComponent.h>
#include <GameEngine/GameApplication/GameApplication.h>
#include <GameEngine/GameState/GameState.h>
#include <GameEngine/Gameplay/BlackboardComponent.h>
#include <GameEngine/Gameplay/InputComponent.h>
#include <GameEngine/Gameplay/SpawnComponent.h>
#include <GameEngine/Messages/DamageMessage.h>
#include <GameEngine/Physics/CharacterControllerComponent.h>
#include <GameEngine/Physics/CollisionFilter.h>
#include <JoltPlugin/Components/JoltRagdollComponent.h>
#include <MonsterAttackPlugin/Components/MonsterComponent.h>
#include <MonsterAttackPlugin/GameState/MonsterAttackGameState.h>

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(ezMonsterComponent, 4, ezComponentMode::Dynamic)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_MEMBER_PROPERTY("Health", m_iHealthPoints)->AddAttributes(new ezDefaultValueAttribute(100)),
    EZ_MEMBER_PROPERTY("MoneyReward", m_iMoneyReward)->AddAttributes(new ezDefaultValueAttribute(100)),
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

  s << m_iHealthPoints;
  s << m_iMoneyReward;
}

void ezMonsterComponent::DeserializeComponent(ezWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);
  const ezUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());

  auto& s = stream.GetStream();

  s >> m_iHealthPoints;
  s >> m_iMoneyReward;
}

void ezMonsterComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();

  ezGameObject* pMoveToTarget;
  if (GetWorld()->TryGetObjectWithGlobalKey("Goal", pMoveToTarget))
  {
    m_hMoveToTarget = pMoveToTarget->GetHandle();

    ezAiNavigationComponent* pNav = nullptr;
    if (GetOwner()->TryGetComponentOfBaseType(pNav))
    {
      pNav->SetDestination(pMoveToTarget->GetGlobalPosition(), true);
    }
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

  ezGameObject* pMoveToTarget = nullptr;
  if (GetWorld()->TryGetObject(m_hMoveToTarget, pMoveToTarget))
  {
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

  CheckGroundType();

  if (auto pBoard = ezBlackboardComponent::FindBlackboard(GetOwner()))
  {
    pBoard->SetEntryValue("State", 1); // "walk" animation
    pBoard->SetEntryValue("MoveSpeed", ezMath::Clamp(GetOwner()->GetLinearVelocity().GetLength() * 0.5f, 0.0f, 2.0f));
  }

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

  auto pBoard = ezBlackboardComponent::FindBlackboard(GetOwner());

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

  if (pBoard)
  {
    pBoard->SetEntryValue("React-Hit", true); // play additive hit reaction animation
  }

  m_iHealthPoints -= iDamage;

  if (m_iHealthPoints <= 0)
  {
    if (MonsterAttackGameState* pGameState = ezDynamicCast<MonsterAttackGameState*>(ezGameApplication::GetGameApplicationInstance()->GetActiveGameState()))
    {
      pGameState->AddDeadMonster(GetOwner()->GetHandle(), m_iMoneyReward);
    }

    if (pBoard)
    {
      pBoard->SetEntryValue("State", 2); // "die" animation
    }

    ezJoltRagdollComponent* pRD = nullptr;
    if (GetOwner()->TryGetComponentOfBaseType(pRD))
    {
      pRD->SetActiveFlag(true);

      // we need to disable the animation controller, so that it doesn't override the result of the ragdoll
      ezAnimationControllerComponent* pAnim = nullptr;
      if (GetOwner()->TryGetComponentOfBaseType(pAnim))
      {
        pAnim->SetActiveFlag(false);
      }
    }

    if (ezGameObject* pPresenceBody = GetOwner()->FindChildByName("PresenceBody"))
    {
      pPresenceBody->SetActiveFlag(false);
    }

    ezAiNavigationComponent* pNav = nullptr;
    if (GetOwner()->TryGetComponentOfBaseType(pNav))
    {
      // not only cancel navigation, but disable it completely, to also stop the step up/down function
      pNav->SetActiveFlag(false);
    }
  }
}
