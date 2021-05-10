#pragma once

#include <vector>

#include "core/core.h"
#include "core/system.h"

#include "gameplay/brawler/entities/weaponpickup.h"



using namespace dagger;

namespace brawler
{
    struct Player
	{
		std::vector<Weapon>		weapons;
		int						active_weapon_idx{ -1 };
		Entity 					currentWeapon;
	};
}