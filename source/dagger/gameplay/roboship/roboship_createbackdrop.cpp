#include "gameplay/roboship/roboship_createbackdrop.h"
#include "gameplay/roboship/fightEnemy.h"

using namespace fightEnemy;

const int TERRAIN_HEIGHT = 30;

void RBackdrop::RoboshipCreateBackdrop(Float32 background_pos_x, Float32 terrain_pos_x)
{
    Float32 base_x = 800;
    auto& reg = Engine::Registry();
    
    auto* camera = Engine::GetDefaultResource<Camera>();

    // Create terrain 
    {
        auto back = reg.create();
        auto& sprite = reg.get_or_emplace<Sprite>(back);

        AssignSprite(sprite, "EmptyWhitePixel");
        sprite.color = { 0, 0, 0, 1 };
        sprite.size = { 1000, TERRAIN_HEIGHT };
        sprite.scale = { 10, 1 };
        sprite.position = { terrain_pos_x, -(camera->size.y - TERRAIN_HEIGHT) / 2, 1 };
    }


    /* Put background image */
    {
        auto entity = reg.create();
        auto& sprite = reg.get_or_emplace<Sprite>(entity);

        AssignSprite(sprite, "robot:BACKGROUND:background2_infinite");

        /*sprite.scale.x = 0.5f;
        sprite.scale.y = 0.5f;*/
        sprite.position.x = background_pos_x;
        sprite.position.y = 5;
        sprite.position.z = 10;
    }

    // Inventory matrix - has data - positions of the parts
    {
        auto entity = reg.create();
        auto& matInv = reg.emplace<InventoryMatrix>(entity);
    }





}