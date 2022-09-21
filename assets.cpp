#include "assets.hpp"

std::vector<Load<Sound::Sample>> sound_assets;
std::vector<std::function< Sound::Sample const *() >> sound_loading_functions;

void load_sound_assets() {
    // Similar way of loading assets as game1
	std::ifstream asset_metadata_file(data_path("/sounds/sound_files_metadata.txt"));
	std::string sound_asset_info;
	std::cout<<"load()" <<std::endl;

	// sound_assets.push_back(dusty_floor_sample);
	// sound_assets.emplace_back(LoadTagDefault, []() -> Sound::Sample const * {
	// 	return new Sound::Sample(data_path("BeepBox-Song.wav"));
	// });

	while (std::getline(asset_metadata_file, sound_asset_info)) {
		std::stringstream sound_info_sstream(sound_asset_info);
		int sound_id;
		std::string sound_file_path;
		sound_info_sstream >> sound_id >> sound_file_path;
		std::cout<<"create Load instance for soudn id" <<sound_id<<std::endl;

        sound_loading_functions.push_back([sound_file_path]() -> Sound::Sample const * {
            std::cout<<"loading sound:" << sound_file_path<<std::endl;
			return new Sound::Sample(data_path(sound_file_path));
		});
		sound_assets.emplace_back(LoadTagDefault, sound_loading_functions.back());

	}
}