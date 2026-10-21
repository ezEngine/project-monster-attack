#include <MonsterAttackPlugin/MonsterAttackPluginPCH.h>

#include <Core/GameState/GameStateBase.h>
#include <Core/Messages/CommonMessages.h>
#include <GameComponentsPlugin/Gameplay/HeadBoneComponent.h>
#include <GameEngine/GameApplication/GameApplication.h>
#include <GameEngine/Gameplay/GrabbableItemComponent.h>
#include <GameEngine/Gameplay/InputComponent.h>
#include <GameEngine/Gameplay/SpawnComponent.h>
#include <GameEngine/Physics/CharacterControllerComponent.h>
#include <GameEngine/Physics/CollisionFilter.h>
#include <GameEngine/StateMachine/StateMachineComponent.h>
#include <MonsterAttackPlugin/Components/PlayerComponent.h>
#include <MonsterAttackPlugin/GameState/MonsterAttackGameState.h>
#include <RendererCore/Components/BlackboardComponent.h>

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

// clang-format off
const ezPlayerComponent::ItemDesc ezPlayerComponent::s_Items[] =
{
  {PlayerAction::ShootMagicBullet, TrapPlacement::None,  "Select_MagicBullet", nullptr,          0,    nullptr,          nullptr},
  {PlayerAction::PlaceSpikeTrap,   TrapPlacement::Floor, "Select_SpikeTrap",   "Cost_SpikeTrap", 1000, "Vis-Trap-Spike", "Trap-Spike"},
  {PlayerAction::PlaceArrowTrap,   TrapPlacement::Wall,  "Select_ArrowTrap",   "Cost_ArrowTrap", 2000, "Vis-Trap-Arrow", "Trap-Arrow"},
  {PlayerAction::PlaceTarTrap,     TrapPlacement::Floor, "Select_TarTrap",     "Cost_TarTrap",   500,  "Vis-Trap-Tar",   "Trap-Tar"},
};
// clang-format on

const ezUInt32 ezPlayerComponent::s_uiNumItems = EZ_ARRAY_SIZE(ezPlayerComponent::s_Items);

void ezPlayerComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();

  ezHashedString sName;
  sName.Assign("LevelState");
  m_pLevelState = ezBlackboard::GetOrCreateGlobal(sName);

  SelectItem(0);
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

    if (msg.m_sInputAction == ezTempHashedString("Select_Next"))
    {
      SelectItem((m_uiActiveItem + 1) % s_uiNumItems);
      return;
    }

    if (msg.m_sInputAction == ezTempHashedString("Select_Prev"))
    {
      SelectItem((m_uiActiveItem + s_uiNumItems - 1) % s_uiNumItems);
      return;
    }

    for (ezUInt32 i = 0; i < s_uiNumItems; ++i)
    {
      if (msg.m_sInputAction == ezTempHashedString(s_Items[i].m_szSelectAction))
      {
        SelectItem(i);
        return;
      }
    }

    if (msg.m_sInputAction == ezTempHashedString("Shoot"))
    {
      if (s_Items[m_uiActiveItem].m_Action == PlayerAction::ShootMagicBullet)
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
      else
      {
        PlaceTrap();
      }
    }
  }
}

void ezPlayerComponent::SelectItem(ezUInt32 uiItem)
{
  const ItemDesc& item = s_Items[uiItem];

  // there is nothing to defend in the hub
  if (item.m_Placement != TrapPlacement::None && m_pLevelState && m_pLevelState->GetEntryValue("Phase").ConvertTo<ezString>() == "Hub")
    return;

  m_uiActiveItem = uiItem;

  if (item.m_szPrevizPrefab != nullptr)
  {
    m_hPrevizPrefab = ezResourceManager::LoadResource<ezPrefabResource>(item.m_szPrevizPrefab);
    m_hPlacePrefab = ezResourceManager::LoadResource<ezPrefabResource>(item.m_szPlacePrefab);
  }

  ClearPrevizObject();

  if (m_pLevelState && m_pLevelState->HasEntry("ActiveItem"))
  {
    m_pLevelState->SetEntryValue("ActiveItem", (ezInt32)uiItem);
  }
}

ezInt32 ezPlayerComponent::GetItemCost(ezUInt32 uiItem) const
{
  const ItemDesc& item = s_Items[uiItem];

  if (item.m_szCostKey == nullptr)
    return 0;

  if (const ezBlackboard::Entry* pEntry = m_pLevelState->GetEntry(ezTempHashedString(item.m_szCostKey)))
    return pEntry->m_Value.ConvertTo<ezInt32>();

  return item.m_iDefaultCost;
}

void ezPlayerComponent::PlaceTrap()
{
  if (m_hPrevizObject.IsInvalidated())
  {
    ShowNotice("Can't place this trap here");
    return;
  }

  const ezInt32 iCost = GetItemCost(m_uiActiveItem);
  const ezInt32 iMoney = m_pLevelState->GetEntry("Money")->m_Value.ConvertTo<ezInt32>();

  if (iMoney < iCost)
  {
    ShowNotice("Not enough gold");
    return;
  }

  m_pLevelState->SetEntryValue("Money", iMoney - iCost);

  ClearPrevizObject();

  ezResourceLock<ezPrefabResource> pPrefab(m_hPlacePrefab, ezResourceAcquireMode::BlockTillLoaded_NeverFail);
  pPrefab->InstantiatePrefab(*GetWorld(), ezTransform::Make(m_vPrevizPosition, m_qPrevizRotation), {});
}

void ezPlayerComponent::ShowNotice(ezStringView sText)
{
  if (!m_pLevelState || !m_pLevelState->HasEntry("Notice"))
    return;

  m_pLevelState->SetEntryValue("Notice", sText);
  m_NoticeEndTime = GetWorld()->GetClock().GetAccumulatedTime() + ezTime::MakeFromSeconds(2.0);
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

  if (s_Items[m_uiActiveItem].m_Placement == TrapPlacement::None)
  {
    ClearPrevizObject();
  }
  else if (!DetermineTrapPlacement(pCameraObject))
  {
    ClearPrevizObject();
  }

  if (m_NoticeEndTime.IsPositive() && GetWorld()->GetClock().GetAccumulatedTime() >= m_NoticeEndTime)
  {
    m_NoticeEndTime = ezTime::MakeZero();
    m_pLevelState->SetEntryValue("Notice", "");
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

  const TrapPlacement placement = s_Items[m_uiActiveItem].m_Placement;

  if (placement == TrapPlacement::Floor)
  {
    // only allow flat floors
    if (!result.m_vNormal.IsEqual(ezVec3(0, 0, 1), 0.05f))
      return false;
  }

  if (placement == TrapPlacement::Ceiling)
  {
    // only allow flat ceilings
    if (!result.m_vNormal.IsEqual(ezVec3(0, 0, -1), 0.05f))
      return false;
  }

  if (placement == TrapPlacement::Wall)
  {
    // only allow straight up walls
    if (!ezMath::IsZero(result.m_vNormal.z, 0.01f))
      return false;
  }

  if (placement == TrapPlacement::Floor || placement == TrapPlacement::Ceiling)
  {
    // place it at the picked height, but on a 2D grid
    m_vPrevizPosition.x = ezMath::Round(m_vPrevizPosition.x);
    m_vPrevizPosition.y = ezMath::Round(m_vPrevizPosition.y);

    m_qPrevizRotation.SetIdentity();
    // TODO: more validation
  }

  if (placement == TrapPlacement::Wall)
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
