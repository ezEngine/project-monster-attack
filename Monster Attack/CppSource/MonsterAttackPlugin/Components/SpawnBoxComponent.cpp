#include <MonsterAttackPlugin/MonsterAttackPluginPCH.h>

#include <Core/Messages/CommonMessages.h>
#include <MonsterAttackPlugin/Components/SpawnBoxComponent.h>

// clang-format off
EZ_BEGIN_COMPONENT_TYPE(ezSpawnBoxComponent, 1, ezComponentMode::Dynamic)
{
  EZ_BEGIN_PROPERTIES
  {
    EZ_ACCESSOR_PROPERTY("HalfExtents", GetHalfExtents, SetHalfExtents)->AddAttributes(new ezDefaultValueAttribute(ezVec3(0.5f)), new ezClampValueAttribute(ezVec3(0), ezVariant())),
  }
  EZ_END_PROPERTIES;
  //EZ_BEGIN_MESSAGEHANDLERS
  //{
  //  EZ_MESSAGE_HANDLER(ezMsgInputActionTriggered, OnMsgInputActionTriggered),
  //}
  //EZ_END_MESSAGEHANDLERS;
  EZ_BEGIN_ATTRIBUTES
  {
    new ezCategoryAttribute("MonsterAttack"),
    new ezBoxManipulatorAttribute("HalfExtents", 2.0f, true),
    new ezBoxVisualizerAttribute("HalfExtents", 2.0f),
  }
  EZ_END_ATTRIBUTES;
}
EZ_END_COMPONENT_TYPE;
// clang-format on

void ezSpawnBoxComponent::SetHalfExtents(const ezVec3& value)
{
  m_vHalfExtents = value.CompMax(ezVec3::MakeZero());

  if (IsActiveAndInitialized())
  {
    GetOwner()->UpdateLocalBounds();
  }
}

void ezSpawnBoxComponent::SerializeComponent(ezWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();
  s << m_vHalfExtents;
}

void ezSpawnBoxComponent::DeserializeComponent(ezWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);

  auto& s = stream.GetStream();
  s >> m_vHalfExtents;
}

void ezSpawnBoxComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();
}

void ezSpawnBoxComponent::Update()
{
}
