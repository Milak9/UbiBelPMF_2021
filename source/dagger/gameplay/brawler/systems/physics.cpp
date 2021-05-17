#include "physics.h"

#include "core/game/transforms.h"

#include "gameplay/brawler/components/movable.h"
#include "gameplay/common/simple_collisions.h"

using namespace dagger;
using namespace brawler;

bool PhysicsSystem::s_UseGravity = true;
float PhysicsSystem::s_Gravity = 500.0f;
float PhysicsSystem::s_RunSpeed = 150.0f;
float PhysicsSystem::s_JumpSpeed = 500.0f;
float PhysicsSystem::s_TerminalVelocity = 800.0f;
float PhysicsSystem::s_DragSpeed = 100.0f;
float PhysicsSystem::s_AirMobility = 0.9f;

void PhysicsSystem::Run()
{
    auto objects = Engine::Registry().view<Transform, Movable, SimpleCollision>();
    for (auto obj : objects)
    {
        auto& t = objects.get<Transform>(obj);
        auto& m = objects.get<Movable>(obj);
        auto& c = objects.get<SimpleCollision>(obj);

        // Update previous frame data
        m.prevPosition = t.position;
        m.prevSpeed = m.speed;

        // Drag
        if(EPSILON_ZERO(m.speed.x)) {
            m.speed.x = 0;
        } else {
            m.speed.x -= (m.speed.x < 0.0f? -1.0f : 1.0f) * PhysicsSystem::s_DragSpeed * Engine::DeltaTime();
            if(m.prevSpeed.x * m.speed.x < 0)
                m.speed.x = 0;
        }

        // Gravity
        if(s_UseGravity && !m.isOnGround)
            m.speed.y -= PhysicsSystem::s_Gravity * Engine::DeltaTime();

        // Clamp speed
        if(m.speed.y > s_TerminalVelocity)
            m.speed.y = s_TerminalVelocity;
        if(m.speed.y < -s_TerminalVelocity)
            m.speed.y = -s_TerminalVelocity;

        // Update position
        t.position.x += m.speed.x * Engine::DeltaTime();
        t.position.y += m.speed.y * Engine::DeltaTime();
    }
}