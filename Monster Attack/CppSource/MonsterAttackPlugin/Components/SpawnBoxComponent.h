#include <Core/World/Component.h>
#include <Core/World/ComponentManager.h>

struct ezSpawnBoxComponentFlags
{
  using StorageType = ezUInt16;

  enum Enum
  {
    None = 0,
    SpawnAtStart = EZ_BIT(0), ///< The component will schedule a spawn once at creation time

    Default = None
  };

  struct Bits
  {
    StorageType SpawnAtStart : 1;
  };
};

EZ_DECLARE_FLAGS_OPERATORS(ezSpawnBoxComponentFlags);

using ezSpawnBoxComponentManager = ezComponentManager<class ezSpawnBoxComponent, ezBlockStorageType::Compact>;

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
  void StartSpawning(); // [ scriptable ]

  void SetHalfExtents(const ezVec3& value);                       // [ property ]
  const ezVec3& GetHalfExtents() const { return m_vHalfExtents; } // [ property ]

  void SetPrefabFile(const char* szFile); // [ property ]
  const char* GetPrefabFile() const;      // [ property ]

  bool GetSpawnAtStart() const; // [ property ]
  void SetSpawnAtStart(bool b); // [ property ]

  ezTime m_SpawnDuration;           // [ property ]
  ezUInt16 m_uiMinSpawnCount = 5;   // [ property ]
  ezUInt16 m_uiSpawnCountRange = 5; // [ property ]
  ezPrefabResourceHandle m_hPrefab; // [ property ]

  /// The spawned object's forward direction may deviate this amount from the spawn box's forward rotation. This is accomplished by rotating around the Z axis.
  ezAngle m_MaxRotationZ; // [ property ]

  /// The spawned object's Z (up) axis may deviate by this amount from the spawn box's Z axis.
  ezAngle m_MaxTiltZ; // [ property ]


private:
  void OnTriggered(ezMsgComponentInternalTrigger& msg);
  void Spawn(ezUInt32 uiCount);

  ezUInt16 m_uiSpawned = 0;
  ezUInt16 m_uiTotalToSpawn = 0;
  ezTime m_StartTime;
  ezBitflags<ezSpawnBoxComponentFlags> m_Flags;
  ezVec3 m_vHalfExtents = ezVec3(0.5f);
};
