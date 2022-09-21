#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include "Load.hpp"
#include "Sound.hpp"
#include "data_path.hpp"


//----- music assets -----
extern std::vector<Load<Sound::Sample>> sound_assets;

void load_sound_assets();