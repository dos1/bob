/*! \file empty.c
 *  \brief Empty gamestate.
 */
/*
 * Copyright (c) Sebastian Krzyszkowiak <dos@dosowisko.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../common.h"
#include <libsuperderpy.h>

#include <vrRigidBody.h>
#include <vrWorld.h>

struct GamestateResources {
	// This struct is for every resource allocated and used by your gamestate.
	// It gets created on load and then gets passed around to all other function calls.
	vrWorld* world;
	struct Entity* player;
	struct Entity* entities[9999];
	int entity_num;
	struct Entity* exit;
	bool up, down;
	bool w, a, s, d;
};

int Gamestate_ProgressCount = 1; // number of loading steps as reported by Gamestate_Load; 0 when missing

static void Start(struct Game* game, struct GamestateResources* data) {
	data->player = CreateEntity(game, data->world, 150, -150, 150, 150, 0.01, 0.0, 0.0, true, 1);
}

static void Restart(struct Game* game, struct GamestateResources* data) {
	vrShape* shape = data->player->body->shape->data[0];
	shape->move(shape->shape, vrVect(999999, 999999));
	Start(game, data);
}

void Gamestate_Logic(struct Game* game, struct GamestateResources* data, double delta) {
	// Here you should do all your game logic as if <delta> seconds have passed.
	//vrWorldStep(data->world);
}
void Gamestate_Tick(struct Game* game, struct GamestateResources* data) {
	// Here you should do all your game logic as if <delta> seconds have passed.
	if (data->down) {
		ChangeEntitySize(game, data->player, 0.975);
	}
	if (data->up) {
		//if (!data->square1->body->manifolds->sizeof_active) {
		ChangeEntitySize(game, data->player, 1.025);
		//vrWorldQueryCollisions(data->world);
		//if (data->square1->body->manifolds->sizeof_active) {
		//	ChangeEntitySize(game, data->square1, 0.98);
		//}
		//}
	}
	if (!data->down && !data->up) {
		vrWorldStep(data->world);
	} else {
		data->world->timeStep = 1.0 / 600.0;
		vrWorldStep(data->world);
		data->world->timeStep = 1.0 / 60.0;
	}

	if (data->a) {
		data->player->pivotY += 0.0333 * sin(data->player->body->orientation);
		data->player->pivotX -= 0.0333 * cos(data->player->body->orientation);
	}
	if (data->d) {
		data->player->pivotY -= 0.0333 * sin(data->player->body->orientation);
		data->player->pivotX += 0.0333 * cos(data->player->body->orientation);
	}
	if (data->w) {
		data->player->pivotX -= 0.0333 * sin(data->player->body->orientation);
		data->player->pivotY -= 0.0333 * cos(data->player->body->orientation);
	}
	if (data->s) {
		data->player->pivotX += 0.0333 * sin(data->player->body->orientation);
		data->player->pivotY += 0.0333 * cos(data->player->body->orientation);
	}

	if (data->player->pivotX > 1.0) {
		data->player->pivotX = 1.0;
	}
	if (data->player->pivotX < 0.0) {
		data->player->pivotX = 0.0;
	}
	if (data->player->pivotY > 1.0) {
		data->player->pivotY = 1.0;
	}
	if (data->player->pivotY < 0.0) {
		data->player->pivotY = 0.0;
	}

	if (data->player->body->center.y > 1600) {
		Restart(game, data);
	}

	vrPolygonShape* p = ((vrShape*)(data->player->body->shape->data[0]))->shape;
	vrPolygonShape* e = ((vrShape*)(data->exit->body->shape->data[0]))->shape;
	if (IsInside(e, p->vertices[0]) && IsInside(e, p->vertices[1]) && IsInside(e, p->vertices[2]) && IsInside(e, p->vertices[3])) {
		Restart(game, data);
	}
}

static struct Entity* PushEntity(struct Game* game, struct GamestateResources* data, struct Entity* entity) {
	data->entities[data->entity_num++] = entity;
	return entity;
}

void Gamestate_Draw(struct Game* game, struct GamestateResources* data) {
	// Draw everything to the screen here.

	ClearToColor(game, al_map_rgba(0, 0, 0, 0));

	for (int i = 0; i < data->entity_num; i++) {
		DrawEntity(game, data->entities[i]);
	}

	float c = 0.9 - sin(game->time * 4) * 0.1;
	al_draw_rectangle(1920 - 250, 1080 - 250, 1920 - 50, 1080 - 50, al_map_rgb_f(c, c * 1.1, c * 1.1), 5);
	al_draw_rectangle(1920 - 250 + 8, 1080 - 250 + 8, 1920 - 50 - 8, 1080 - 50 - 8, al_map_rgb_f(c, c * 1.1, c * 1.1), 4);
	al_draw_rectangle(1920 - 250 + 15, 1080 - 250 + 15, 1920 - 50 - 15, 1080 - 50 - 15, al_map_rgb_f(c, c * 1.1, c * 1.1), 3);
	al_draw_rectangle(1920 - 250 + 21, 1080 - 250 + 21, 1920 - 50 - 21, 1080 - 50 - 21, al_map_rgb_f(c, c * 1.1, c * 1.1), 2);
	al_draw_rectangle(1920 - 250 + 26, 1080 - 250 + 26, 1920 - 50 - 26, 1080 - 50 - 26, al_map_rgb_f(c, c * 1.1, c * 1.1), 1);

	DrawEntity(game, data->player);
	if (data->up || data->down) {
		al_draw_filled_circle(data->player->body->center.x, data->player->body->center.y, 4, al_map_rgb(10, 200, 200));

		al_draw_line(data->player->body->center.x, data->player->body->center.y,
			data->player->body->center.x + data->player->body->velocity.x / 8.0, data->player->body->center.y + data->player->body->velocity.y / 8.0,
			al_map_rgb(10, 200, 200), 2);
	}
	if (data->up || data->down || data->w || data->a || data->s || data->d) {
		vrVec2 pivot = GetPivot(data->player);
		al_draw_filled_circle(pivot.x, pivot.y, 4, al_map_rgb(200, 200, 40));
	}

	//al_draw_filled_rectangle(pivot.x - 1, pivot.y - 1, pivot.x + 1, pivot.y + 1, al_map_rgb(255, 0, 0));

	/*	DrawEntity(game, data->walls[0]);
	DrawEntity(game, data->walls[1]);
	DrawEntity(game, data->walls[2]);
	DrawEntity(game, data->walls[3]);
	*/
}

void Gamestate_ProcessEvent(struct Game* game, struct GamestateResources* data, ALLEGRO_EVENT* ev) {
	// Called for each event in Allegro event queue.
	// Here you can handle user input, expiring timers etc.
	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
		UnloadCurrentGamestate(game); // mark this gamestate to be stopped and unloaded
		// When there are no active gamestates, the engine will quit.
	}

	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_DOWN)) {
		data->down = true;
	}
	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_UP)) {
		data->up = true;
	}
	if ((ev->type == ALLEGRO_EVENT_KEY_UP) && (ev->keyboard.keycode == ALLEGRO_KEY_DOWN)) {
		data->down = false;
	}
	if ((ev->type == ALLEGRO_EVENT_KEY_UP) && (ev->keyboard.keycode == ALLEGRO_KEY_UP)) {
		data->up = false;
	}

	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_W)) {
		data->w = true;
	}
	if ((ev->type == ALLEGRO_EVENT_KEY_UP) && (ev->keyboard.keycode == ALLEGRO_KEY_W)) {
		data->w = false;
	}

	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_A)) {
		data->a = true;
	}
	if ((ev->type == ALLEGRO_EVENT_KEY_UP) && (ev->keyboard.keycode == ALLEGRO_KEY_A)) {
		data->a = false;
	}

	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_S)) {
		data->s = true;
	}
	if ((ev->type == ALLEGRO_EVENT_KEY_UP) && (ev->keyboard.keycode == ALLEGRO_KEY_S)) {
		data->s = false;
	}

	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_D)) {
		data->d = true;
	}
	if ((ev->type == ALLEGRO_EVENT_KEY_UP) && (ev->keyboard.keycode == ALLEGRO_KEY_D)) {
		data->d = false;
	}
}

static struct Entity* Rotate(float angle, struct Entity* entity) {
	vrShape* shape = entity->body->shape->data[0];
	shape->rotate(shape->shape, angle, shape->getCenter(shape->shape));
	return entity;
}

void* Gamestate_Load(struct Game* game, void (*progress)(struct Game*)) {
	// Called once, when the gamestate library is being loaded.
	// Good place for allocating memory, loading bitmaps etc.
	//
	// Keep in mind that there's no OpenGL context available here. If you want to prerender something,
	// create VBOs, etc. do it in Gamestate_PostLoad.

	struct GamestateResources* data = calloc(1, sizeof(struct GamestateResources));
	progress(game); // report that we progressed with the loading, so the engine can move a progress bar

	data->world = vrWorldInit(vrWorldAlloc());
	data->world->gravity = vrVect(0, 9.81);

	data->entity_num = 0;

	Start(game, data);

	PushEntity(game, data, CreateEntity(game, data->world, 0, 350, 400, 50, -1, 1, 0, false, 0));

	PushEntity(game, data, Rotate(0.25, CreateEntity(game, data->world, 550, 750, 450, 50, -1, 0.05, 0, false, 0)));
	PushEntity(game, data, CreateEntity(game, data->world, 1200, 0, 50, 700, -1, 0.5, 0, false, 0));

	PushEntity(game, data, CreateEntity(game, data->world, 1300, 1030, 620, 50, -1, 1, 0, false, 0));
	/*
	data->exit = vrShapeInit(vrShapeAlloc());
	data->exit = vrShapePolyInit(data->exit);
	data->exit->shape = vrPolyBoxInit(data->exit->shape, 1920 - 250, 1080 - 250, 200, 200);
*/
	data->exit = PushEntity(game, data, CreateEntity(game, data->world, 1920 - 250, 1080 - 250, 200, 200, -1, 0, 0, false, 2));
	data->exit->body->collisionData.categoryMask = 0;
	data->exit->body->collisionData.maskBit = 0;

	//PushEntity(game, data, CreateEntity(game, data->world, 50, 60, 20, 20, -1, 9999, 0, false, 0));
	//PushEntity(game, data, CreateEntity(game, data->world, 90, 130, 20, 20, -1, 9999, 1.5, false, 0));

	/*
	// walls
	//PushEntity(game, data, CreateEntity(game, data->world, 0, 1080, 1920, 100, -1, 2, 0.1, false, 0));
	PushEntity(game, data, CreateEntity(game, data->world, -100, -1080, 100, 1080 * 3, -1, 2, 0.1, false, 0));
	PushEntity(game, data, CreateEntity(game, data->world, 1920, -1080, 100, 1080 * 3, -1, 2, 0.1, false, 0));
	//PushEntity(game, data, CreateEntity(game, data->world, 0, -100, 1920, 100, -1, 2, 0.1, false, 0));
  */
	return data;
}

void Gamestate_Unload(struct Game* game, struct GamestateResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	//vrShapeDestroy(data->shape);
	//vrBodyDestroy(data->body);
	vrWorldDestroy(data->world);
	free(data);
}

void Gamestate_Start(struct Game* game, struct GamestateResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
}

void Gamestate_Stop(struct Game* game, struct GamestateResources* data) {
	// Called when gamestate gets stopped. Stop timers, music etc. here.
}

// Optional endpoints:

void Gamestate_PostLoad(struct Game* game, struct GamestateResources* data) {
	// This is called in the main thread after Gamestate_Load has ended.
	// Use it to prerender bitmaps, create VBOs, etc.
}

void Gamestate_Pause(struct Game* game, struct GamestateResources* data) {
	// Called when gamestate gets paused (so only Draw is being called, no Logic nor ProcessEvent)
	// Pause your timers and/or sounds here.
}

void Gamestate_Resume(struct Game* game, struct GamestateResources* data) {
	// Called when gamestate gets resumed. Resume your timers and/or sounds here.
}

void Gamestate_Reload(struct Game* game, struct GamestateResources* data) {
	// Called when the display gets lost and not preserved bitmaps need to be recreated.
	// Unless you want to support mobile platforms, you should be able to ignore it.
}
