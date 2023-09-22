#include <Core/World/Component.h>
#include <Core/World/ComponentManager.h>

using ezSpawnBoxComponentManager = ezComponentManagerSimple<class ezSpawnBoxComponent, ezComponentUpdateType::WhenSimulating>;

class ezSpawnBoxComponent : public ezComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezSpawnBoxComponent, ezComponent, ezSpawnBoxComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // ezComponent

public:
  virtual void SerializeComponent(ezWorldWriter& stream) const override;
  virtual void DeserializeComponent(ezWorldReader& stream) override;

protected:
  virtual void OnSimulationStarted() override;

  //////////////////////////////////////////////////////////////////////////
  // ezSpawnBoxComponent

public:
  void SetHalfExtents(const ezVec3& value);                       // [ property ]
  const ezVec3& GetHalfExtents() const { return m_vHalfExtents; } // [ property ]

private:
  void Update();

  ezVec3 m_vHalfExtents = ezVec3(0.5f);
};
