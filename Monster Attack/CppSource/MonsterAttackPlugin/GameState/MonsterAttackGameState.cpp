#include <MonsterAttackPlugin/MonsterAttackPluginPCH.h>

#include <Core/Input/InputManager.h>
#include <Core/System/Window.h>
#include <Core/World/World.h>
#include <MonsterAttackPlugin/GameState/MonsterAttackGameState.h>
#include <Foundation/Configuration/CVar.h>
#include <Foundation/Logging/Log.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Meshes/MeshComponent.h>

ezCVarBool cvar_DebugDisplay("MonsterAttack.DebugDisplay", false, ezCVarFlags::Default, "Whether the game should display debug geometry.");

EZ_BEGIN_DYNAMIC_REFLECTED_TYPE(MonsterAttackGameState, 1, ezRTTIDefaultAllocator<MonsterAttackGameState>)
EZ_END_DYNAMIC_REFLECTED_TYPE;

MonsterAttackGameState::MonsterAttackGameState() = default;
MonsterAttackGameState::~MonsterAttackGameState() = default;

void MonsterAttackGameState::OnActivation(ezWorld* pWorld, const ezTransform* pStartPosition)
{
  EZ_LOG_BLOCK("GameState::Activate");

  SUPER::OnActivation(pWorld, pStartPosition);
}

void MonsterAttackGameState::OnDeactivation()
{
  EZ_LOG_BLOCK("GameState::Deactivate");

  SUPER::OnDeactivation();
}

void MonsterAttackGameState::AfterWorldUpdate()
{
  SUPER::AfterWorldUpdate();

  if (cvar_DebugDisplay)
  {
    ezDebugRenderer::DrawLineSphere(m_pMainWorld, ezBoundingSphere::MakeFromCenterAndRadius(ezVec3::MakeZero(), 1.0f), ezColor::Orange);
  }

  ezDebugRenderer::Draw2DText(m_pMainWorld, "Press 'O' to spawn objects", ezVec2I32(10, 10), ezColor::White);
  ezDebugRenderer::Draw2DText(m_pMainWorld, "Press 'P' to remove objects", ezVec2I32(10, 30), ezColor::White);
}

void MonsterAttackGameState::BeforeWorldUpdate()
{
  EZ_LOCK(m_pMainWorld->GetWriteMarker());
}

ezGameStatePriority MonsterAttackGameState::DeterminePriority(ezWorld* pWorld) const
{
  return ezGameStatePriority::Default;
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

  RegisterInputAction("MonsterAttackPlugin", "SpawnObject", ezInputSlot_KeyO, ezInputSlot_Controller0_ButtonA, ezInputSlot_MouseButton2);
  RegisterInputAction("MonsterAttackPlugin", "DeleteObject", ezInputSlot_KeyP, ezInputSlot_Controller0_ButtonB);
}

void MonsterAttackGameState::ProcessInput()
{
  SUPER::ProcessInput();

  ezWorld* pWorld = m_pMainWorld;

  if (ezInputManager::GetInputActionState("MonsterAttackPlugin", "SpawnObject") == ezKeyState::Pressed)
  {
    const ezVec3 pos = GetMainCamera()->GetCenterPosition() + GetMainCamera()->GetCenterDirForwards();

    // make sure we are allowed to modify the world
    EZ_LOCK(pWorld->GetWriteMarker());

    // create a game object at the desired position
    ezGameObjectDesc desc;
    desc.m_LocalPosition = pos;

    ezGameObject* pObject = nullptr;
    ezGameObjectHandle hObject = pWorld->CreateObject(desc, pObject);

    m_SpawnedObjects.PushBack(hObject);

    // attach a mesh component to the object
    ezMeshComponent* pMesh;
    pWorld->GetOrCreateComponentManager<ezMeshComponentManager>()->CreateComponent(pObject, pMesh);

    // Set the mesh to use.
    // Here we use a path relative to the project directory.
    // We have to reference the 'transformed' file, not the source file.
    // This would break if the source asset is moved or renamed.
    pMesh->SetMeshFile("AssetCache/Common/Meshes/Sphere.ezMesh");

    // here we use the asset GUID to reference the transformed asset
    // we can copy the GUID from the asset browser
    // the GUID is stable even if the source asset gets moved or renamed
    // using asset collections we could also give a nice name like 'Blue Material' to this asset
    ezMaterialResourceHandle hMaterial = ezResourceManager::LoadResource<ezMaterialResource>("{ aa1c5601-bc43-fbf8-4e07-6a3df3af51e7 }");

    // override the mesh material in the first slot with something different
    pMesh->SetMaterial(0, hMaterial);
  }

  if (ezInputManager::GetInputActionState("MonsterAttackPlugin", "DeleteObject") == ezKeyState::Pressed)
  {
    if (!m_SpawnedObjects.IsEmpty())
    {
      // make sure we are allowed to modify the world
      EZ_LOCK(pWorld->GetWriteMarker());

      ezGameObjectHandle hObject = m_SpawnedObjects.PeekBack();
      m_SpawnedObjects.PopBack();

      // this is only for demonstration purposes, removing the object will delete all attached components as well
      ezGameObject* pObject = nullptr;
      if (pWorld->TryGetObject(hObject, pObject))
      {
        ezMeshComponent* pMesh = nullptr;
        if (pObject->TryGetComponentOfBaseType(pMesh))
        {
          pMesh->DeleteComponent();
        }
      }

      // delete the object, all its children and attached components
      pWorld->DeleteObjectDelayed(hObject);
    }
  }
}

void MonsterAttackGameState::ConfigureMainCamera()
{
  SUPER::ConfigureMainCamera();

  // do custom camera setup here
}
