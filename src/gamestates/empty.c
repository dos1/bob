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
	struct Entity *square1, *square2, *square3;

	struct Entity* squares[4];

	bool up, down;
};

int Gamestate_ProgressCount = 1; // number of loading steps as reported by Gamestate_Load; 0 when missing

void Gamestate_Logic(struct Game* game, struct GamestateResources* data, double delta) {
	// Here you should do all your game logic as if <delta> seconds have passed.
	//vrWorldStep(data->world);
}
void Gamestate_Tick(struct Game* game, struct GamestateResources* data) {
	// Here you should do all your game logic as if <delta> seconds have passed.
	if (data->down) {
		ChangeEntitySize(game, data->square1, 0.98);
	}
	if (data->up) {
		ChangeEntitySize(game, data->square1, 1.02);
	}
	if (!data->down && !data->up) {
		vrWorldStep(data->world);
	}
}

void Gamestate_Draw(struct Game* game, struct GamestateResources* data) {
	// Draw everything to the screen here.

	/*	vrPolygonShape* pshape = ((vrShape*)data->body->shape->data[0])->shape;
	vrVec2 topleft = pshape->vertices[0];
	PrintConsole(game, "%f %f", topleft.x, topleft.y);

	ALLEGRO_VERTEX v[] = {
		{.x = pshape->vertices[0].x, .y = pshape->vertices[0].y, .color = al_map_rgb(255, 255, 255)},
		{.x = pshape->vertices[1].x, .y = pshape->vertices[1].y, .color = al_map_rgb(255, 255, 255)},
		{.x = pshape->vertices[2].x, .y = pshape->vertices[2].y, .color = al_map_rgb(255, 255, 255)},

		{.x = pshape->vertices[0].x, .y = pshape->vertices[0].y, .color = al_map_rgb(255, 255, 255)},
		{.x = pshape->vertices[2].x, .y = pshape->vertices[2].y, .color = al_map_rgb(255, 255, 255)},
		{.x = pshape->vertices[3].x, .y = pshape->vertices[3].y, .color = al_map_rgb(255, 255, 255)}};
	//	al_draw_prim(v, NULL, NULL, 0, 6, ALLEGRO_PRIM_TRIANGLE_LIST);
*/
	DrawEntity(game, data->square1);
	DrawEntity(game, data->square2);
	DrawEntity(game, data->square3);

	for (int i = 0; i < 3; i++) {
		//DrawEntity(game, data->squares[i]);
	}
	/*
	//	al_draw_filled_rectangle(topleft.x, topleft.y, topleft.x + 20, topleft.y + 20, al_map_rgb(255, 255, 255));
	pshape = ((vrShape*)data->body2->shape->data[0])->shape;
	ALLEGRO_VERTEX v2[] = {
		{.x = pshape->vertices[0].x, .y = pshape->vertices[0].y, .color = al_map_rgb(255, 255, 255)},
		{.x = pshape->vertices[1].x, .y = pshape->vertices[1].y, .color = al_map_rgb(255, 255, 255)},
		{.x = pshape->vertices[2].x, .y = pshape->vertices[2].y, .color = al_map_rgb(255, 255, 255)},

		{.x = pshape->vertices[0].x, .y = pshape->vertices[0].y, .color = al_map_rgb(255, 255, 255)},
		{.x = pshape->vertices[2].x, .y = pshape->vertices[2].y, .color = al_map_rgb(255, 255, 255)},
		{.x = pshape->vertices[3].x, .y = pshape->vertices[3].y, .color = al_map_rgb(255, 255, 255)}};
	al_draw_prim(v2, NULL, NULL, 0, 6, ALLEGRO_PRIM_TRIANGLE_LIST);

	pshape = ((vrShape*)data->body3->shape->data[0])->shape;
	ALLEGRO_VERTEX v3[] = {
		{.x = pshape->vertices[0].x, .y = pshape->vertices[0].y, .color = al_map_rgb(255, 0, 255)},
		{.x = pshape->vertices[1].x, .y = pshape->vertices[1].y, .color = al_map_rgb(255, 0, 255)},
		{.x = pshape->vertices[2].x, .y = pshape->vertices[2].y, .color = al_map_rgb(0, 0, 255)},

		{.x = pshape->vertices[0].x, .y = pshape->vertices[0].y, .color = al_map_rgb(255, 0, 255)},
		{.x = pshape->vertices[2].x, .y = pshape->vertices[2].y, .color = al_map_rgb(255, 0, 0)},
		{.x = pshape->vertices[3].x, .y = pshape->vertices[3].y, .color = al_map_rgb(255, 0, 255)}};
	al_draw_prim(v3, NULL, NULL, 0, 6, ALLEGRO_PRIM_TRIANGLE_LIST);
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

	data->square1 = CreateEntity(game, data->world, 60, 10, 20, 20, 0.1, 0.0, 0.0, true);
	data->square2 = CreateEntity(game, data->world, 50, 60, 20, 20, -1, 9999, 0, false);
	data->square3 = CreateEntity(game, data->world, 90, 130, 20, 20, -1, 9999, 1.5, false);

	data->squares[0] = CreateEntity(game, data->world, -20, -100, 20, 280, -1, 2, 0.1, false);
	data->squares[1] = CreateEntity(game, data->world, 320, -100, 20, 280, -1, 2, 0.1, false);
	data->squares[2] = CreateEntity(game, data->world, 0, 180, 320, 20, -1, 2, 0.1, false);
	data->squares[3] = CreateEntity(game, data->world, 0, -120, 320, 20, -1, 2, 0.1, false);
	/*
	data->body = vrBodyInit(vrBodyAlloc());
	data->body->bodyMaterial.mass = 0.1;
	data->body->bodyMaterial.invMass = 1.0 / data->body->bodyMaterial.mass;
	//data->body->bodyMaterial.momentInertia = vrMomentForBox(20, 20, data->body->bodyMaterial.mass);
	//data->body->bodyMaterial.invMomentInertia = 1.0 / data->body->bodyMaterial.momentInertia;
	data->body->bodyMaterial.friction = 0.0;
	data->body->bodyMaterial.restitution = 0.1;
	//	data->body->gravity = true;
	//	data->body->position = (vrVec2){.x = 20, .y = 20};
	vrWorldAddBody(data->world, data->body);

	data->shape = vrShapeInit(vrShapeAlloc());
	data->shape = vrShapePolyInit(data->shape);
	data->shape->shape = vrPolyBoxInit(data->shape->shape, 30, 10, 20, 20);
	vrArrayPush(data->body->shape, data->shape);

	data->body2 = vrBodyInit(vrBodyAlloc());
	//data->body2->bodyMaterial.mass = 999999;
	data->body2->bodyMaterial.invMass = 0;
	//data->body2->bodyMaterial.momentInertia = vrMomentForBox(20, 20, data->body2->bodyMaterial.mass);
	data->body2->bodyMaterial.invMomentInertia = 0; //1.0 / data->body2->bodyMaterial.momentInertia;
	data->body2->bodyMaterial.friction = 9999;
	data->body2->bodyMaterial.restitution = 0;
	data->body2->gravity = false;
	//	data->body->position = (vrVec2){.x = 20, .y = 20};
	vrWorldAddBody(data->world, data->body2);

	data->shape2 = vrShapeInit(vrShapeAlloc());
	data->shape2 = vrShapePolyInit(data->shape2);
	data->shape2->shape = vrPolyBoxInit(data->shape2->shape, 20, 60, 20, 20);
	vrArrayPush(data->body2->shape, data->shape2);

	data->body3 = vrBodyInit(vrBodyAlloc());
	//data->body2->bodyMaterial.mass = 999999;
	data->body3->bodyMaterial.invMass = 0;
	//data->body2->bodyMaterial.momentInertia = vrMomentForBox(20, 20, data->body2->bodyMaterial.mass);
	data->body3->bodyMaterial.invMomentInertia = 0; //1.0 / data->body2->bodyMaterial.momentInertia;
	data->body3->bodyMaterial.friction = 9999;
	data->body3->bodyMaterial.restitution = 2;
	data->body3->gravity = false;
	//	data->body->position = (vrVec2){.x = 20, .y = 20};
	vrWorldAddBody(data->world, data->body3);

	data->shape3 = vrShapeInit(vrShapeAlloc());
	data->shape3 = vrShapePolyInit(data->shape3);
	data->shape3->shape = vrPolyBoxInit(data->shape3->shape, 70, 150, 20, 20);
	vrArrayPush(data->body3->shape, data->shape3);
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
