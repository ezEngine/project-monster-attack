#include <Core/World/Component.h>
#include <Core/World/ComponentManager.h>
#include <GameEngine/Gameplay/InputComponent.h>

struct ezMsgDamage;

using ezMonsterComponentManager = ezComponentManagerSimple<class ezMonsterComponent, ezComponentUpdateType::WhenSimulating>;

class ezMonsterComponent : public ezComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezMonsterComponent, ezComponent, ezMonsterComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // ezComponent

public:
  virtual void SerializeComponent(ezWorldWriter& stream) const override;
  virtual void DeserializeComponent(ezWorldReader& stream) override;

protected:
  virtual void OnSimulationStarted() override;

  //////////////////////////////////////////////////////////////////////////
  // ezMonsterComponent

public:
private:
  void Update();

  void OnMsgDamage(ezMsgDamage& msg);

  ezInt32 m_iHealthPoints = 100;
};
