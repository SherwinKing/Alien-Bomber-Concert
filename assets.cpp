#include "assets.hpp"

void load_sound_assets(std::vector<Sound::Sample> * sample_vector_ptr) {
    // Similar way of loading assets as game1
	std::ifstream asset_metadata_file(data_path("/sounds/sound_files_metadata.txt"));
	std::string sound_asset_info;

	while (std::getline(asset_metadata_file, sound_asset_info)) {
		std::stringstream sound_info_sstream(sound_asset_info);
		int sound_id;
		std::string sound_file_path;
		sound_info_sstream >> sound_id >> sound_file_path;

		sample_vector_ptr->emplace_back(Sound::Sample(data_path(sound_file_path)));

	}
}