#include "level_system.h"

using namespace dagger;
using namespace brawler;

void LevelSystem::SpinUp()
{
	Engine::Dispatcher().sink<AssetLoadRequest<LevelData>>().connect<&LevelSystem::OnAssetLoadRequest>(this);

	LoadDefaultAssets();
};

void LevelSystem::LoadDefaultAssets()
{
	for (auto& entry : Files::recursive_directory_iterator("maps"))
	{
		auto path = entry.path().string();
		if (entry.is_regular_file() && entry.path().extension() == ".json")
		{
			Engine::Dispatcher().trigger<AssetLoadRequest<LevelData>>(AssetLoadRequest<LevelData>{ path });
		}
	}
}

void LevelSystem::OnAssetLoadRequest(AssetLoadRequest<LevelData> request_)
{
	FilePath path(request_.path);
	Logger::info("Loading '{}'", request_.path);

	if (!Files::exists(path))
	{
		Engine::Dispatcher().trigger<Error>(Error{ fmt::format("Couldn't load map from {}.", request_.path) });
		return;
	}

	FileInputStream handle;
	auto absolutePath = Files::absolute(path);
	handle.open(absolutePath);

	if (!handle.is_open())
	{
		Engine::Dispatcher().trigger<Error>(Error{ fmt::format("Couldn't open map file '{}' for reading.", absolutePath.string()) });
		return;
	}

	JSON::json json;
	handle >> json;

	LevelData* levelData = new LevelData();
	assert(json.contains("name"));
	assert(json.contains("map_width"));
	assert(json.contains("map_height"));
	assert(json.contains("tileset"));
	assert(json.contains("tilemap"));
	assert(json.contains("backgrounds"));
	assert(json.contains("player1"));
	assert(json.contains("player2"));

	levelData->name = json["name"];
	levelData->mapWidth = json["map_width"];
	levelData->mapHeight = json["map_height"];

	// Tileset
	Sequence<TileData> tileset;
	for (auto& tile : json["tileset"])
	{
		TileData tileData;
		switch(tile.value("type", 0)) {
			case 1:
				tileData.type = PlatformType::BLOCK;
				tileData.texture = LoadTexture(tile);
				break;
			case 2:
				tileData.type = PlatformType::ONEWAY;
				tileData.texture = LoadTexture(tile);
				break;
			case 3:
				tileData.type = PlatformType::SPAWNER;
				tileData.spawnInterval = tile.value("interval", 30.0f);
				break;
			default:
				tileData.type = PlatformType::EMPTY;
				break;
		}
		tileset.push_back(std::move(tileData));
	}
	levelData->tileset = std::move(tileset);

	// Tilemap
	Sequence<Sequence<int>> tiles;
	assert(json["tilemap"].size() == levelData->mapHeight);
	for (auto row = json["tilemap"].rbegin(); row != json["tilemap"].rend(); row++)
	{
		assert(row->size() == levelData->mapWidth);
		tiles.emplace_back();
		for (auto& tile : (*row)) {
			tiles.back().push_back(tile.get<int>()-1);
		}
	}
	levelData->tilemap = std::move(tiles);

	// Backgrounds
	Sequence<TextureData> backgrounds;
	for (auto& background : json["backgrounds"])
	{
		backgrounds.push_back(LoadTexture(background));
	}
	levelData->backgrounds = std::move(backgrounds);

	// Players
	levelData->player1 = LoadPlayer(json["player1"]);
	levelData->player2 = LoadPlayer(json["player2"]);

	auto& library = Engine::Res<LevelData>();
	if (library.contains(levelData->name))
	{
		delete library[levelData->name];
	}

	library[levelData->name] = levelData;
	Logger::info("Map '{}' loaded!", levelData->name);
}

TextureData LevelSystem::LoadTexture(JSON::json& input_)
{
	TextureData texture;
	texture.name = input_.value("texture", "EmptyWhitePixel");
	texture.scale = input_.value("scale", 1.0f);
	return texture;
}

PlayerData LevelSystem::LoadPlayer(JSON::json& player_)
{
	assert(player_.contains("x"));
	assert(player_.contains("y"));
	assert(player_.contains("is_left"));
	PlayerData p1;
	p1.x = player_["x"];
	p1.y = player_["y"];
	p1.isLeft = player_["is_left"];
	return p1;
}

void LevelSystem::Run()
{
}

void LevelSystem::WindDown()
{
	Engine::Dispatcher().sink<AssetLoadRequest<LevelData>>().disconnect<&LevelSystem::OnAssetLoadRequest>(this);
};