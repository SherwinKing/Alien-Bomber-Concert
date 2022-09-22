#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"
#include "Load.hpp"
#include "assets.hpp"

#include <glm/glm.hpp>

#include <list>
#include <vector>
#include <deque>
#include <random>
#include<cmath>
#include <fstream>
#include <sstream>


const int32_t init_hp = 30000;

enum BombState : size_t {
	Inactive = 0,
	Active = 1
};

typedef struct Bomb {
	BombState state = Inactive;
	Scene::Transform transform;
	uint32_t sound_id = 0;
	Bomb(Bomb const &) = delete;
	Bomb() = default;
} Bomb;

typedef struct NoteInfo {
	uint32_t pitch_id;
	float start_time;
	uint32_t asset_id;
} NoteInfo;

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//other methods
	void restart_game();
	void reset_bomb_position(Scene::Transform &transform);
	void activate_bomb_position(Scene::Transform &transform);
	void bomb_explode(Bomb &bomb, float bomb_distance);


	//----- game state -----
		// game status
	enum GameStatus : uint32_t{
		STOPPED,
		ACTIVE
	};
	GameStatus game_status = STOPPED;

	std::shared_ptr< Sound::PlayingSample > bgm_playing_sample_ptr = nullptr;

	// total time elapsed
	double total_time_elapsed = 0;

	// init wait time
	float init_wait_time = 2.0f;

	// HP
	int32_t hp = init_hp;
	// score
	uint32_t score = 0;
	// bomb speed
	float bomb_speed = 5.0f;

	//random generator
	std::mt19937 mt; 


	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, key_r, left_click;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//note start time sequence list
	std::list<NoteInfo> note_info_list;

	//bomb_transforms
	Scene::Transform* bomb_init_transform;
	std::list<Bomb> bombs;

	std::list<Bomb*> inactive_bomb_ptrs;

	glm::vec3 get_leg_tip_position();

	//music coming from the tip of the leg (as a demonstration):
	std::shared_ptr< Sound::PlayingSample > leg_tip_loop;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
