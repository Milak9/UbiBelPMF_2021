#include "gameplay/roboship/roboship_main.h"
#include "gameplay/roboship/selectTileController.h"
#include "gameplay/roboship/inventorySetup.h"
#include "gameplay/roboship/fightEnemy.h"
#include "gameplay/roboship/roboship_player_move.h";
#include "gameplay/roboship/roboship_camera_focus.h"
#include "gameplay/roboship/roboship_createbackdrop.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <string>

#include "core/core.h"
#include "core/engine.h"
#include "core/audio.h"
#include "core/input/inputs.h"
#include "core/graphics/sprite.h"
#include "core/graphics/animation.h"
#include "core/graphics/shaders.h"
#include "core/graphics/window.h"
#include "core/game/transforms.h"
#include "core/graphics/sprite_render.h"
#include "core/graphics/textures.h"
#include "core/graphics/animations.h"
#include "core/graphics/gui.h"
#include "core/graphics/text.h"
#include "tools/diagnostics.h"
#include <chrono>
#include "tools/diagnostics.h"

using namespace dagger;
using namespace roboship;
using namespace robo_game;
using namespace fightEnemy;

const int TERRAIN_HEIGHT = 30;
static int enemy_number = 0;
static int n_enemies = 0;

void Roboship::GameplaySystemsSetup()
{
    auto& engine = Engine::Instance();

    engine.AddSystem<SelectedTileInputSystem>();


    engine.AddSystem<RoboshipPlayerInputSystem>();
    engine.AddSystem<RCameraFollowSystem>();
}

REnemy* REnemy::Get(Entity entity)
{
    auto& reg = Engine::Registry();
    auto& sprite = reg.get_or_emplace<Sprite>(entity);
    auto& anim = reg.get_or_emplace<Animator>(entity);
    reg.get_or_emplace<EnemyMarker>(entity);

    return new REnemy{ entity, sprite, anim };
}

REnemy* REnemy::Create(
    ColorRGB color_ = { 1, 1, 1 },
    Vector2 position_ = { 0, 0 })
{

    auto& reg = Engine::Registry();
    auto entity = reg.create();

    REnemy *chr = REnemy::Get(entity);
    chr->sprite.scale = { 0.15f, 0.15f };
    chr->sprite.position = { position_, 0.0f };
    //chr.sprite.color = { color_, 1.0f };

    AssignSprite(chr->sprite, "robot:ENEMIES:Robot1:01_Idle:idle_000");
    AnimatorPlay(chr->animator, "robot:ENEMIES:Robot1:01_Idle");

    return chr;
}

int REnemy::getNumberOfTurns() {
    return this->numberOfTurns;
}

std::vector<int> REnemy::getSequence() {
    return this->sequence;
}

void REnemy::fillSequence() {
    int lenOfSeq = rand() % 3 + 2;

    for (int i = 0; i < lenOfSeq; i++) {
        this->sequence.push_back(rand()%5 + 1);
    }
}

void REnemy::setNumberOFTurns() {
    this->numberOfTurns = rand() % 4 + 3;
}

void REnemy::change_direction(RChangeDirection& ev)
{
    this->sprite.scale.x *= -1.0f;
}

RSpaceship* RSpaceship::Get(Entity entity)
{
    auto& reg = Engine::Registry();
    auto& sprite = reg.get_or_emplace<Sprite>(entity);
    auto& anim = reg.get_or_emplace<Animator>(entity);

    return new RSpaceship{ entity, sprite, anim};
}

RSpaceship* RSpaceship::Create(
    ColorRGB color_ = { 1, 1, 1 },
    Vector2 position_ = { 0, 0 })
{

    auto& reg = Engine::Registry();
    auto entity = reg.create();

    RSpaceship* chr = RSpaceship::Get(entity);
    chr->sprite.scale = { 1.6f, 1.6f };
    chr->sprite.position = { position_, 0.0f };
    //chr.sprite.color = { color_, 1.0f };

    AssignSprite(chr->sprite, "robot:SPACESHIP:IDLE:vehicle1");
    AnimatorPlay(chr->animator, "robot:SPACESHIP:IDLE");

    return chr;
}

void RoboshipSetCamera()
{
    auto* camera = Engine::GetDefaultResource<Camera>();
    camera->mode = ECameraMode::FixedResolution;
    camera->size = { 800, 600 };

    camera->zoom = 1;
//    camera->zoom = 1;
    camera->position = { 0, 0, 0 };
    camera->Update();
}


void Roboship::WorldSetup()
{
    RoboshipSetCamera();
    RBackdrop::RoboshipCreateBackdrop(0, 0);
    bool found = findCombination({ 1, 1 });

    auto& engine = Engine::Instance();
    auto& reg = engine.Registry();

    float zPos = 0.0f;

    // player

    {
        auto entity = reg.create();
        auto& sprite = reg.emplace<Sprite>(entity);
        auto& animator = reg.get_or_emplace<Animator>(entity);

        AssignSprite(sprite, "robot:IDLE:Idle (1)");
        AnimatorPlay(animator, "robot:IDLE");

        sprite.scale = { 0.2f, 0.2f };
        sprite.position = { 0.0f, -223.0f, 0.0f };

        auto& roboshipPlayer = reg.emplace<RoboshipPlayer>(entity);
        roboshipPlayer.speed = 200;

        reg.emplace<ControllerMapping>(entity);
        Engine::Registry().emplace<RCameraFollowFocus>(entity);
    }

    Engine::Dispatcher().sink<RPrepareFightModeOn>().connect<&Roboship::ShowTextPrepareFightMode>(this);
    //Engine::Dispatcher().sink<RPrepareFightModeOff>().connect<&Roboship::ShowTextPrepareFightMode>(this);


    n_enemies = 5;
    int i;
    for (i = 0; i < n_enemies; i++)
    {
        REnemy* enemyChar = REnemy::Create({ 0, 1, 0 }, { (i+1) * 800, -200 });
        enemyChar->sprite.scale = { -0.15f, 0.15f };
         
        enemyChar->setNumberOFTurns();
        enemyChar->fillSequence();

        Engine::Dispatcher().sink<RFightModeOn>().connect<&Roboship::RobotDie>(this);

        Engine::Dispatcher().sink<RChangeDirection>().connect<&Roboship::TurnRobots>(this);
        
        std::vector<int> sequence = enemyChar->getSequence();
        

        auto entity = reg.create();
        auto& text = reg.emplace<Text>(entity);
        text.spacing = 0.6f;

        std::string str = "Turns:" + std::to_string(enemyChar->getNumberOfTurns()) + "; ";
        std::string str2 = "Sequence: ";
        for (auto x : enemyChar->sequence) {
            str2 += std::to_string(x);
        }

        text.Set("pixel-font", str+str2, { (i + 1) * 800, -50 , 0 }, false);
    }

    RSpaceship* spaceship = RSpaceship::Create({ 0, 1, 0 }, { (i + 1) * 800, -210 });
}

void Roboship::ShowTextPrepareFightMode(){

    enemy_number++;

    /*Engine& engine = Engine::Instance();
    auto& reg = engine.Registry();

    if (brojac % 2 == 0) {
        auto ui = reg.create();

        auto& text = reg.emplace<Text>(ui);
        text.spacing = 0.6f;
        text.Set("pixel-font", "Fight(F) / Go on(G)");
    }*/
}

void Roboship::ClearTextPrepareFightMode(){

}

void Roboship::RobotDie()
{
    Engine::Registry().view<Sprite, Animator, EnemyMarker>().each([](Sprite& sprite_, Animator& animator_, const EnemyMarker&)
        {
            if (abs(sprite_.position.x - enemy_number * 800) <= 2) {
                AnimatorPlay(animator_, "robot:ENEMIES:Robot1:02_Death");

                sprite_.position.y -= 0.6;
                if (sprite_.position.y <= -250)
                    sprite_.position.x = 1000000;
            }
        });
}

void Roboship::TurnRobots()
{
    Engine::Registry().view<Sprite, EnemyMarker>().each([](Sprite& sprite_, const EnemyMarker&)
        {
            sprite_.scale.x *= -1.0f;
        });
}

void Roboship::TurnOnSpaceship() {
    
}
