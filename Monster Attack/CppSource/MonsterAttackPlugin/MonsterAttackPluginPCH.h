#pragma once

#include <Foundation/Basics.h>

#include <MonsterAttackPlugin/MonsterAttackPluginDLL.h>

#include <Core/Input/InputManager.h>
#include <Core/Interfaces/PhysicsWorldModule.h>
#include <Core/Prefabs/PrefabResource.h>
#include <Core/ResourceManager/ResourceManager.h>
#include <Core/Utils/Blackboard.h>
#include <Core/World/Component.h>
#include <Core/World/ComponentManager.h>
#include <Core/World/World.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <Foundation/Containers/Deque.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Types/SharedPtr.h>
#include <GameEngine/GameApplication/GameApplication.h>
#include <GameEngine/GameState/GameState.h>
#include <RendererCore/Components/BlackboardComponent.h>
