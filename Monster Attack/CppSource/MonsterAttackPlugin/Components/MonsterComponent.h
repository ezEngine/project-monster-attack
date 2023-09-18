#include <AiPlugin/Navigation/Navigation.h>
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
  ezHashedString m_sNavmeshConfig;
  ezHashedString m_sPathSearchConfig;
  float m_fWalkSpeed = 4.0f;

private:
  void Update();

  void OnMsgDamage(ezMsgDamage& msg);

  ezInt32 m_iHealthPoints = 100;

  ezAiNavigation m_Navigation;

  ezGameObjectHandle m_hMoveToTarget;
};
