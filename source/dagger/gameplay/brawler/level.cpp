#include "level.h"

#include "core/graphics/sprite.h"
#include "core/graphics/animation.h"
#include "gameplay/brawler/systems/level_system.h"
#include "core/graphics/sprite.h"

using namespace dagger;
using namespace brawler;

unsigned Level::LEVEL_WIDTH = 0;
unsigned Level::LEVEL_HEIGHT = 0;

Tilemap Level::tiles{};

void Level::Load(String map)
{
    auto& reg = Engine::Registry();
    const auto* level = Engine::Res<LevelData>()[map];

    LEVEL_WIDTH = level->mapWidth;
    LEVEL_HEIGHT = level->mapHeight;

    tiles.clear();
    for (unsigned y = 0; y < LEVEL_HEIGHT; y++) {
        tiles.emplace_back();
        for(unsigned x = 0; x < LEVEL_WIDTH; x++) {
            tiles[y].push_back(level->tileset[level->tilemap[y][x]].type);
            if(getTile(x, y) == PlatformType::BLOCK) {
                auto tile = reg.create();
                auto& sprite = reg.get_or_emplace<Sprite>(tile);
                auto texture = level->tileset[level->tilemap[y][x]].texture;
                AssignSprite(sprite, texture.name);
                sprite.color = { 1, 1, 1, 1 };
                sprite.size = { TILE_WIDTH, TILE_HEIGHT };
                sprite.scale = { texture.scale, texture.scale };
                sprite.position = { TileToWorld(x, y), 1 };
            }
        }
    }

    float backgroundZ = 100.0f;
    for (auto& background : level->backgrounds) {
        auto bg = reg.create();
        auto& sprite = reg.get_or_emplace<Sprite>(bg);
        AssignSprite(sprite, background.name);
        sprite.position = { 200, 150, backgroundZ };
        sprite.scale = { background.scale, background.scale };
        backgroundZ--;
    }
}

std::optional<float> Level::getGround(BrawlerCharacter c) {
    Vector2 oldBottomLeft = {c.movable.prevPosition.x - c.col.size.x/2 + 1, c.movable.prevPosition.y - c.col.size.y/2 - 1};
    Vector2 newBottomLeft = {c.transform.position.x - c.col.size.x/2 + 1, c.transform.position.y - c.col.size.y/2 - 1};

    int endY = WorldToTileY(newBottomLeft.y);
    int startY = std::max<int>(WorldToTileY(oldBottomLeft.y)-1, endY);
    int distance = std::max<int>(std::abs(endY - startY), 1);

    for (int tileY = startY; tileY >= endY; tileY--)
    {
        Vector2 bottomLeft = newBottomLeft + (std::abs(static_cast<float>(endY - tileY)) / distance) * (oldBottomLeft - newBottomLeft);
        Vector2 bottomRight = bottomLeft + Vector2(c.col.size.x-2, 0);

        for (Vector2 checkedTile = bottomLeft; ; checkedTile.x += TILE_WIDTH)
        {
            checkedTile.x = std::min<float>(checkedTile.x, bottomRight.x);

            auto [x, y] = WorldToTile(checkedTile);
            if(Level::getTile(x, y)==PlatformType::BLOCK) {
                float ground = TileToWorldY(y) + TILE_HEIGHT/2 + c.col.size.y/2;
                return {ground};
            }

            if(checkedTile.x >= bottomRight.x)
                break;
        }
    }

    return {};
}

std::optional<float> Level::getCeiling(BrawlerCharacter c) {
    Vector2 oldTopLeft = {c.movable.prevPosition.x - c.col.size.x/2, c.movable.prevPosition.y + c.col.size.y/2 + 1};
    Vector2 newTopLeft = {c.transform.position.x - c.col.size.x/2, c.transform.position.y + c.col.size.y/2 + 1};

    int endY = WorldToTileY(newTopLeft.y);
    int startY = std::min<int>(WorldToTileY(oldTopLeft.y)+1, endY);
    int distance = std::max<int>(std::abs(endY - startY), 1);

    for (int tileY = startY; tileY <= endY; tileY++)
    {
        Vector2 topLeft = newTopLeft + (std::abs(static_cast<float>(endY - tileY)) / distance) * (oldTopLeft - newTopLeft);
        Vector2 topRight = topLeft + Vector2(c.col.size.x, 0);

        for (Vector2 checkedTile = topLeft; ; checkedTile.x += TILE_WIDTH)
        {
            checkedTile.x = std::min<float>(checkedTile.x, topRight.x);

            auto [x, y] = WorldToTile(checkedTile);
            if(Level::getTile(x, y)==PlatformType::BLOCK) {
                float ceiling = TileToWorldY(y) - TILE_HEIGHT/2 - c.col.size.y/2;
                return {ceiling};
            }

            if(checkedTile.x >= topRight.x)
                break;
        }
    }

    return {};
}

std::optional<float> Level::getLeftWall(BrawlerCharacter c) {
    Vector2 oldBottomLeft = {c.movable.prevPosition.x - c.col.size.x/2 - 1, c.movable.prevPosition.y - c.col.size.y/2};
    Vector2 newBottomLeft = {c.transform.position.x - c.col.size.x/2 - 1, c.transform.position.y - c.col.size.y/2};

    int endX = WorldToTileX(newBottomLeft.x);
    int startX = std::max<int>(WorldToTileX(oldBottomLeft.x)-1, endX);
    int distance = std::max<int>(std::abs(endX - startX), 1);

    for (int tileX = startX; tileX >= endX; tileX--)
    {
        Vector2 bottomLeft = newBottomLeft + (std::abs(static_cast<float>(endX - tileX)) / distance) * (oldBottomLeft - newBottomLeft);
        Vector2 topLeft = bottomLeft + Vector2(0, c.col.size.y);

        for (Vector2 checkedTile = bottomLeft; ; checkedTile.y += TILE_HEIGHT)
        {
            checkedTile.y = std::min<float>(checkedTile.y, topLeft.y);

            auto [x, y] = WorldToTile(checkedTile);
            if(Level::getTile(x, y)==PlatformType::BLOCK) {
                float leftWall = TileToWorldX(x) + TILE_WIDTH/2 + c.col.size.x/2;
                return {leftWall};
            }

            if(checkedTile.y >= topLeft.y)
                break;
        }
    }

    return {};
}

std::optional<float> Level::getRightWall(BrawlerCharacter c) {
    Vector2 oldBottomRight = {c.movable.prevPosition.x + c.col.size.x/2 + 1, c.movable.prevPosition.y - c.col.size.y/2};
    Vector2 newBottomRight = {c.transform.position.x + c.col.size.x/2 + 1, c.transform.position.y - c.col.size.y/2};

    int endX = WorldToTileX(newBottomRight.x);
    int startX = std::min<int>(WorldToTileX(oldBottomRight.x)+1, endX);
    int distance = std::max<int>(std::abs(endX - startX), 1);

    for (int tileX = startX; tileX <= endX; tileX++)
    {
        Vector2 bottomRight = newBottomRight + (std::abs(static_cast<float>(endX - tileX)) / distance) * (oldBottomRight - newBottomRight);
        Vector2 topRight = bottomRight + Vector2(0, c.col.size.y);

        for (Vector2 checkedTile = bottomRight; ; checkedTile.y += TILE_HEIGHT)
        {
            checkedTile.y = std::min<float>(checkedTile.y, topRight.y);

            auto [x, y] = WorldToTile(checkedTile);
            if(Level::getTile(x, y)==PlatformType::BLOCK) {
                float rightWall = TileToWorldX(x) - TILE_WIDTH/2 - c.col.size.x/2 - 1;
                return {rightWall};
            }

            if(checkedTile.y >= topRight.y)
                break;
        }
    }

    return {};
}

TileCoords Level::WorldToTile(Vector2 worldPos)
{
    return { WorldToTileX(worldPos.x), WorldToTileY(worldPos.y) };
}

int Level::WorldToTileX(float x)
{
    if(x >= 0)
        return static_cast<int>(x) / TILE_WIDTH;
    else
        return static_cast<int>(x) / TILE_WIDTH - 1;
}

int Level::WorldToTileY(float y)
{
    if(y >= 0)
        return static_cast<int>(y) / TILE_HEIGHT;
    else
        return static_cast<int>(y) / TILE_HEIGHT - 1;
}

Vector2 Level::TileToWorld(int x, int y)
{
    return {
        TileToWorldX(x),
        TileToWorldY(y)
    };
}

float Level::TileToWorldX(int x)
{
    return x * TILE_WIDTH + TILE_WIDTH/2;
}

float Level::TileToWorldY(int y)
{
    return y * TILE_HEIGHT + TILE_HEIGHT/2;
}

PlatformType Level::getTile(int x, int y)
{
    if(x<0 || x>=LEVEL_WIDTH || y<0 || y>=LEVEL_HEIGHT)
        return PlatformType::BLOCK;
    return tiles[y][x];
}