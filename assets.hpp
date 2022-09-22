#pragma once
#include <vector>
#include <array>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>

#include "Load.hpp"
#include "Sound.hpp"
#include "data_path.hpp"


//----- music assets -----
extern std::array<Load<Sound::Sample>, 128> sound_assets;
extern Load< Sound::Sample > dusty_floor_sample;

void load_sound_assets(std::vector<Sound::Sample> * sample_vector_ptr);