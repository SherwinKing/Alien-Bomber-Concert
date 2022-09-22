#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"

#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint hexapod_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("hexapod.pnct"));
	hexapod_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

void add_drawable(Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
	Mesh const &mesh = hexapod_meshes->lookup(mesh_name);

	scene.drawables.emplace_back(transform);
	Scene::Drawable &drawable = scene.drawables.back();

	drawable.pipeline = lit_color_texture_program_pipeline;

	drawable.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
	drawable.pipeline.type = mesh.type;
	drawable.pipeline.start = mesh.start;
	drawable.pipeline.count = mesh.count;
}

Load< Scene > hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("hexapod.scene"), add_drawable);
});

// Load the sound assets into one vector
Load< std::vector<Sound::Sample> > sample_vector_ptr_load(LoadTagDefault, []() -> std::vector<Sound::Sample> const * {
	std::vector<Sound::Sample> * sample_vector_ptr = new std::vector<Sound::Sample>();
	load_sound_assets(sample_vector_ptr);
	return sample_vector_ptr;
});
});

PlayMode::PlayMode() : scene(*hexapod_scene) {
	for (auto &transform : scene.transforms) {
		if (transform.name == "Bomb") {
			bomb_init_transform = &transform;
		}
	}

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	//add bombs
	for (int i = 0; i < 10; i++) {
		bombs.emplace_back();
		Bomb &bomb = bombs.back();
		bomb.sound_id = i;
		bomb.transform.name = bomb_init_transform->name + std::to_string(i);
		reset_bomb_position(bomb.transform);
		add_drawable(scene, &bomb.transform, "Bomb");
	}
}
	
PlayMode::~PlayMode() {
}

void PlayMode::restart_game() {
	hp = init_hp;
	score = 0;
	for (auto &bomb: bombs) {
		reset_bomb_position(bomb.transform);
	}
	game_status = ACTIVE;
}

void PlayMode::reset_bomb_position(Scene::Transform &bomb_transform) {
	bomb_transform.position = bomb_init_transform->position;
	bomb_transform.position[0] = (float) -40;
	bomb_transform.position[1] = 100;
	bomb_transform.position[2] = (float)16 - 40 + 60 * ((double) mt()/(double)UINT32_MAX);
	bomb_transform.rotation = bomb_init_transform->rotation;
	bomb_transform.scale = bomb_init_transform->scale;
	bomb_transform.parent = bomb_init_transform->parent;
}

void PlayMode::bomb_explode(Bomb &bomb, float bomb_distance) {
	hp -= (int32_t) std::max((double) 0, (1000 / std::pow(0.75+bomb_distance/4, 3)));
	// hp -= (int32_t) std::max((double) 0, std::pow(40 - bomb_distance, 3) / 16);
	hp = std::max(0, hp);
	reset_bomb_position(bomb.transform);

	Sound::play_3D((*sample_vector_ptr_load)[bomb.sound_id], 1.0f, get_leg_tip_position(), 10.0f);
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_r) {
			key_r.downs += 1;
			key_r.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_r) {
			key_r.pressed = false;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
		}
		left_click.pressed = true;
		return true;
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			camera->transform->rotation = glm::normalize(
				camera->transform->rotation
				* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
			);
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
	// if game is in STOPPED status
	if (game_status == STOPPED) {
		if (key_r.downs > 0) {
			restart_game();
		} else {
			return;
		}
	}

	for (Bomb & bomb: bombs) {
		const static glm::vec4 camera_space_origin = glm::vec4(0, 0, 0, 1);
		glm::mat4x3 camera_to_bomb = (bomb.transform.make_world_to_local() 
			* glm::mat4(camera->transform->make_local_to_world()));
		glm::vec3 camera_position_in_bomb_space = camera_to_bomb * camera_space_origin;
		bomb.transform.position += bomb_speed * glm::normalize(camera_position_in_bomb_space);

		// distance between player (at camera) and bomb
		float camera_to_bomb_distance = glm::length(camera_position_in_bomb_space);

		// if player shoot bomb
		if (left_click.pressed) {
			glm::vec3 fire_line_dir = camera_to_bomb * glm::vec4(0, 0, 1, 0);
			float intersection_indicator = std::pow(glm::dot(camera_position_in_bomb_space, fire_line_dir), 2) - std::pow(glm::length(camera_position_in_bomb_space), 2) + 1;
			if (intersection_indicator >= 0) {
				bomb_explode(bomb, camera_to_bomb_distance);
				score += camera_to_bomb_distance;
				// as you get higher score, the game gets harder
				bomb_speed = 0.1f + 0.0001f * score;
			}
		}

		// if bomb hit player
		if (glm::length(camera_position_in_bomb_space) < 0.5) {
			bomb_explode(bomb, camera_to_bomb_distance);
		}

		// if player hp is 0
		if (hp == 0) {
			game_status = STOPPED;
		}
	}

	{ //update listener to camera position:
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		glm::vec3 frame_at = frame[3];
		Sound::listener.set_position_right(frame_at, frame_right, 1.0f / 60.0f);
	}

	//reset button press counters:
	left_click.pressed = false;
	key_r.downs = 0;
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		std::string game_info_string = "Score: " + std::to_string(score)+ ", HP: " + std::to_string(hp);
		lines.draw_text(game_info_string,
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text(game_info_string,
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));

		// start game guidance
		std::string start_game_guidance = "Press R to start game";
		if (game_status == STOPPED) {
			lines.draw_text(start_game_guidance,
				glm::vec3(-aspect + 0.5f * H, -1.0 + 0.1f * H + 0.5f, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		}
	}
	GL_ERRORS();
}

glm::vec3 PlayMode::get_leg_tip_position() {
	//the vertex position here was read from the model in blender:
	
	return bombs.front().transform.make_local_to_world() * glm::vec4(-1.26137f, -11.861f, 0.0f, 1.0f);
}
