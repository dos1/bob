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

static char* LABELS[][20] = {
	{"Bob was alone.", "...or not.", "He didn't know.", "In fact, he couldn't. He was just a square.", "Unable to think, unable to feel.", "A very squary square.", "Bob was a square.", NULL},
	{"Bob couldn't move. He was just there.", "Minding his own square business.", "Which means no business at all.", NULL},
	{"There's one thing he could do though.", "He could grow.", NULL},
	{"...and ungrow.", "In more sophisticated words: he could also shrink.", NULL},
	{"A little point appeared on Bob's face", "proving beyond any doubt, that this game isn't pointless.", NULL},
	{"Some other things around appeared as well.", "He didn't care though.", "He couldn't.", NULL},
	{"\"Where am I supposed to go?\" - thought someone.", "Someone who wasn't Bob.", "Not being a square makes thinking somewhat easier.", NULL},
	{"Bob has reached an exit.", "Not that he would care.", "But that happened. I found it worth mentioning.", NULL},
	{"Bob would wonder about this strange force that controls his actions.", "What is it?", "Is there some external entity controlling his own actions?", "Is there a goal they were following?", "Is there something special about Bob to make them interested in him?", "He would, if he could.", NULL},
	{"Bob couldn't die.", "Amazingly though, he could make others feel like they just died.", "What an extraordinary ability for a square to possess.", NULL},
	{"Bob still couldn't die.", "Pretty handy, if you ask me.", NULL},
	{"Stairs.", NULL},
	{"Lots of stairs.", NULL},
	{"There were answers on top.", "You could feel that in the air.", NULL},
	{"What's the Bob's legacy?", "Who is he?", "How did he come to exist?", "Was it God?", NULL},
	{"So many questions.", "...and nobody to ask them.", NULL},
	{"Is this it...?", NULL},
};

static float POSITIONS[][20] = {
	{0, 2, 3.2, 4.7, 8, 11, 13.5},
	{0, 2.8, 5.2},
	{0, 2.3},
	{0, 1.6},
	{0, 2.5},
	{0, 2.5, 4},
	{0, 3.5, 5},
	{0, 2.3, 4.3},
	{0, 4.5, 6, 9.5, 12, 17},
	{0, 1.5, 5.7},
	{0, 2},
	{0},
	{0},
	{0, 2},
	{0, 2, 3.4, 6},
	{0, 2},
	{0},
};

static char* FILES[20] = {
	"voice/1.flac",
	"voice/2.flac",
	"voice/3.flac",
	"voice/4.flac",
	"voice/5.flac",
	"voice/6.flac",
	"voice/7.flac",
	"voice/8.flac",
	"voice/9.flac",
	"voice/die1.flac",
	"voice/die2.flac",
	"voice/stairs1.flac",
	"voice/stairs2.flac",
	"voice/stairs3.flac",
	"voice/stairs4.flac",
	"voice/stairs5.flac",
	"voice/stairs6.flac",
};

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

	bool growlock, pivotlock, inputlock;
	bool upped, downed, pivoted, triedtomove, shown;

	struct Timeline* timeline;

	int current_voice;
	int fab_voice;

	int level;
	int die_counter;

	bool isthisit_triggered;

	struct {
		ALLEGRO_SAMPLE* sample;
		ALLEGRO_SAMPLE_INSTANCE* instance;
		int id;
	} voices[17];
};

int Gamestate_ProgressCount = 18; // number of loading steps as reported by Gamestate_Load; 0 when missing

static TM_ACTION(WaitForVoice) {
	TM_RunningOnly;
	if (data->current_voice >= 0) {
		return !al_get_sample_instance_playing(data->voices[data->current_voice].instance);
	}
	return false;
}

static TM_ACTION(WaitForTryMove) {
	TM_RunningOnly;
	return data->triedtomove;
}

static TM_ACTION(WaitForPivoted) {
	TM_RunningOnly;
	return data->pivoted;
}

static TM_ACTION(WaitForUpped) {
	TM_RunningOnly;
	return data->upped;
}

static TM_ACTION(WaitForDowned) {
	TM_RunningOnly;
	return data->downed;
}

static TM_ACTION(UnlockInput) {
	TM_RunningOnly;
	data->inputlock = false;
	return true;
}

static TM_ACTION(UnlockGrow) {
	TM_RunningOnly;
	data->growlock = false;
	return true;
}

static TM_ACTION(ShowStuff) {
	TM_RunningOnly;
	data->shown = true;
	return true;
}

static TM_ACTION(UnlockPivot) {
	TM_RunningOnly;
	data->pivotlock = false;
	return true;
}

static TM_ACTION(PlayNextVoice) {
	TM_RunningOnly;
	data->fab_voice++;
	if (data->fab_voice == 9) {
		data->fab_voice = 11;
	}
	data->current_voice = data->fab_voice;
	if (data->current_voice < 17) {
		al_play_sample_instance(data->voices[data->current_voice].instance);
	}
	return true;
}

static TM_ACTION(JustDied1) {
	TM_RunningOnly;
	data->current_voice = 9;
	al_play_sample_instance(data->voices[data->current_voice].instance);
	return true;
}

static TM_ACTION(JustDied2) {
	TM_RunningOnly;
	data->current_voice = 10;
	al_play_sample_instance(data->voices[data->current_voice].instance);
	return true;
}

static void DestroyPhysics(struct Game* game, struct GamestateResources* data) {
	for (int i = 0; i < data->entity_num; i++) {
		vrWorldRemoveBody(data->world, data->entities[i]->body);
		free(data->entities[i]);
	}
	if (data->world) {
		vrWorldDestroy(data->world);
		data->world = NULL;
	}
	data->entity_num = 0;
	data->exit = NULL;
	data->player = NULL;
}

static void Start(struct Game* game, struct GamestateResources* data) {
	data->player = CreateEntity(game, data->world, 150, -150, 150, 150, 0.01, 0.0, 0.0, true, 1);
	data->player->body->center = vrVect(150 + 75, -75);
	game->data->chime = 4.0;
}

static struct Entity* PushEntity(struct Game* game, struct GamestateResources* data, struct Entity* entity) {
	data->entities[data->entity_num++] = entity;
	return entity;
}

static struct Entity* Rotate(float angle, struct Entity* entity) {
	vrShape* shape = entity->body->shape->data[0];
	shape->rotate(shape->shape, angle, shape->getCenter(shape->shape));
	return entity;
}

static void CreateExit(struct Game* game, struct GamestateResources* data, float x, float y) {
	data->exit = PushEntity(game, data, CreateEntity(game, data->world, x, y, 200, 200, -1, 0, 0, false, 2));
	data->exit->body->collisionData.categoryMask = 0;
	data->exit->body->collisionData.maskBit = 0;
	data->exit->body->center = vrVect(x + 100, y + 100);
}

static void StartLevel(struct Game* game, struct GamestateResources* data, int level) {
	data->level = level;

	DestroyPhysics(game, data);

	data->world = vrWorldInit(vrWorldAlloc());
	data->world->gravity = vrVect(0, 9.81);

	Start(game, data);

	data->entity_num = 0;

	if (level == 0) {
		PushEntity(game, data, CreateEntity(game, data->world, 0, 600, 1920, 50, -1, 1, 0, false, 0));
		CreateExit(game, data, 1920 - 200, 600 - 200);
	}

	if (level == 1) {
		PushEntity(game, data, CreateEntity(game, data->world, 0, 500, 800 - 10, 50, -1, 1, 0, false, 0));
		PushEntity(game, data, CreateEntity(game, data->world, 800, 500, 400, 50, 0.005, 10, 0, false, 3));
		PushEntity(game, data, CreateEntity(game, data->world, 1200 + 10, 500, 720 - 10, 50, -1, 1, 0, false, 0));
		CreateExit(game, data, 1920 - 200, 1080 - 250);
		PushEntity(game, data, CreateEntity(game, data->world, 1920 - 1120, 1080 - 50, 1120, 50, -1, 1, 0, false, 0));
	}

	if (level == 2) {
		PushEntity(game, data, CreateEntity(game, data->world, 0, 350, 400, 50, -1, 1, 0, false, 0));

		PushEntity(game, data, Rotate(0.25, CreateEntity(game, data->world, 550, 750, 450, 50, -1, 0.05, 0, false, 0)));
		PushEntity(game, data, CreateEntity(game, data->world, 1200, 0, 50, 700, -1, 0.5, 0, false, 0));

		PushEntity(game, data, CreateEntity(game, data->world, 1300, 1030, 620, 50, -1, 1, 0, false, 0));
		/*
	data->exit = vrShapeInit(vrShapeAlloc());
	data->exit = vrShapePolyInit(data->exit);
	data->exit->shape = vrPolyBoxInit(data->exit->shape, 1920 - 250, 1080 - 250, 200, 200);
*/
		CreateExit(game, data, 1920 - 250, 1080 - 250);

		//PushEntity(game, data, CreateEntity(game, data->world, 50, 60, 20, 20, -1, 9999, 0, false, 0));
		//PushEntity(game, data, CreateEntity(game, data->world, 90, 130, 20, 20, -1, 9999, 1.5, false, 0));

		/*
	// walls
	//PushEntity(game, data, CreateEntity(game, data->world, 0, 1080, 1920, 100, -1, 2, 0.1, false, 0));
	PushEntity(game, data, CreateEntity(game, data->world, -100, -1080, 100, 1080 * 3, -1, 2, 0.1, false, 0));
	PushEntity(game, data, CreateEntity(game, data->world, 1920, -1080, 100, 1080 * 3, -1, 2, 0.1, false, 0));
	//PushEntity(game, data, CreateEntity(game, data->world, 0, -100, 1920, 100, -1, 2, 0.1, false, 0));
  */
	}

	if (level == 3) {
		PushEntity(game, data, CreateEntity(game, data->world, 0, 700, 1000 - 10, 50, -1, 1, 0, false, 0));
		PushEntity(game, data, CreateEntity(game, data->world, 1100, 1000, 400, 50, -1, 2, 2, false, 4));

		CreateExit(game, data, 1920 - 250, 1080 - 250 - 500);
		PushEntity(game, data, CreateEntity(game, data->world, 1920 - 300, 1080 - 50 - 500, 300, 50, -1, 1, 0, false, 0));

		PushEntity(game, data, CreateEntity(game, data->world, 1920 - 50, -200, 50, 1080 - 250 - 800 + 200 + 100 + 200 + 200, -1, 1, 0, false, 0));

		PushEntity(game, data, CreateEntity(game, data->world, 0, 0, 50, 700, -1, 1, 0, false, 0));
		PushEntity(game, data, CreateEntity(game, data->world, 500, 0, 1920 - 500 - 50, 50, -1, 1, 0, false, 0));
	}

	if (level == 4) {
		// stairs
		PushEntity(game, data, CreateEntity(game, data->world, 0, 1080 - 50, 1920, 50, -1, 1, 0, false, 0));
		PushEntity(game, data, CreateEntity(game, data->world, 120, 1080 - 50 * 2, 1920 - 120, 50, -1, 1, 0, false, 0));
		PushEntity(game, data, CreateEntity(game, data->world, 120 * 2, 1080 - 50 * 3, 1920 - 120 * 2, 50, -1, 1, 0, false, 0));
		PushEntity(game, data, CreateEntity(game, data->world, 120 * 3, 1080 - 50 * 4, 1920 - 120 * 3, 50, -1, 1, 0, false, 0));
		PushEntity(game, data, CreateEntity(game, data->world, 120 * 4, 1080 - 50 * 5, 1920 - 120 * 4, 50, -1, 1, 0, false, 0));
		PushEntity(game, data, CreateEntity(game, data->world, 120 * 5, 1080 - 50 * 6, 1920 - 120 * 5, 50, -1, 1, 0, false, 0));
		PushEntity(game, data, CreateEntity(game, data->world, 120 * 6, 1080 - 50 * 7, 1920 - 120 * 6, 50, -1, 1, 0, false, 0));
		PushEntity(game, data, CreateEntity(game, data->world, 120 * 7, 1080 - 50 * 8, 1920 - 120 * 7, 50, -1, 1, 0, false, 0));
		PushEntity(game, data, CreateEntity(game, data->world, 120 * 8, 1080 - 50 * 9, 1920 - 120 * 8, 50, -1, 1, 0, false, 0));
		PushEntity(game, data, CreateEntity(game, data->world, 120 * 9, 1080 - 50 * 10, 1920 - 120 * 9, 50, -1, 1, 0, false, 0));
		PushEntity(game, data, CreateEntity(game, data->world, 120 * 10, 1080 - 50 * 11, 1920 - 120 * 10, 50, -1, 1, 0, false, 0));
		PushEntity(game, data, CreateEntity(game, data->world, 120 * 11, 1080 - 50 * 12, 1920 - 120 * 11, 50, -1, 1, 0, false, 0));
		PushEntity(game, data, CreateEntity(game, data->world, 120 * 12, 1080 - 50 * 13, 1920 - 120 * 12, 50, -1, 1, 0, false, 0));
		PushEntity(game, data, CreateEntity(game, data->world, 120 * 13, 1080 - 50 * 14, 1920 - 120 * 13, 50, -1, 1, 0, false, 0));
		PushEntity(game, data, CreateEntity(game, data->world, 120 * 14, 1080 - 50 * 15, 1920 - 120 * 14, 50, -1, 1, 0, false, 0));

		CreateExit(game, data, 1920 - 200, 1080 - 50 * 15 - 200);
	}

	vrWorldStep(data->world);
}

static void Restart(struct Game* game, struct GamestateResources* data) {
	vrShape* shape = data->player->body->shape->data[0];
	shape->move(shape->shape, vrVect(999999, 999999));
	StartLevel(game, data, data->level);
}

void Gamestate_Logic(struct Game* game, struct GamestateResources* data, double delta) {
	// Here you should do all your game logic as if <delta> seconds have passed.
	//vrWorldStep(data->world);
	TM_Process(data->timeline, delta);
}

static void Win(struct Game* game, struct GamestateResources* data) {
	if (data->level + 1 == 1) {
		TM_AddAction(data->timeline, PlayNextVoice, NULL);
		TM_AddAction(data->timeline, WaitForVoice, NULL);
	}
	if (data->level + 1 == 3) {
		TM_AddAction(data->timeline, PlayNextVoice, NULL);
		TM_AddAction(data->timeline, WaitForVoice, NULL);
	}
	if (data->level + 1 == 4) {
		TM_AddAction(data->timeline, PlayNextVoice, NULL);
		TM_AddAction(data->timeline, WaitForVoice, NULL);
		TM_AddDelay(data->timeline, 2);
		TM_AddAction(data->timeline, PlayNextVoice, NULL);
		TM_AddAction(data->timeline, WaitForVoice, NULL);

		TM_AddDelay(data->timeline, 4);
		TM_AddAction(data->timeline, PlayNextVoice, NULL);
		TM_AddAction(data->timeline, WaitForVoice, NULL);

		TM_AddDelay(data->timeline, 2);
		TM_AddAction(data->timeline, PlayNextVoice, NULL);
		TM_AddAction(data->timeline, WaitForVoice, NULL);

		TM_AddDelay(data->timeline, 5);
		TM_AddAction(data->timeline, PlayNextVoice, NULL);
		TM_AddAction(data->timeline, WaitForVoice, NULL);
	}
	if (data->level + 1 == 5) {
		SwitchCurrentGamestate(game, "heaven");
	}
	StartLevel(game, data, data->level + 1);
}

void Gamestate_Tick(struct Game* game, struct GamestateResources* data) {
	// Here you should do all your game logic as if <delta> seconds have passed.

	game->data->tint = al_map_rgba_f(0.75, 0.85, 0.85, 0.85);
	if (data->up || data->down) {
		game->data->tint = al_map_rgba_f(0.92, 0.9, 0.92, 0.9);
	}

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
		if (TM_IsEmpty(data->timeline)) {
			data->die_counter++;
			if (data->die_counter == 1) {
				TM_AddAction(data->timeline, JustDied1, NULL);
				TM_AddAction(data->timeline, WaitForVoice, NULL);
			} else if (data->die_counter == 2) {
				TM_AddAction(data->timeline, JustDied2, NULL);
				TM_AddAction(data->timeline, WaitForVoice, NULL);
			}
		}
	}

	vrPolygonShape* p = ((vrShape*)(data->player->body->shape->data[0]))->shape;
	vrPolygonShape* e = ((vrShape*)(data->exit->body->shape->data[0]))->shape;
	if (IsInside(e, p->vertices[0]) && IsInside(e, p->vertices[1]) && IsInside(e, p->vertices[2]) && IsInside(e, p->vertices[3])) {
		Win(game, data);
	}

	game->data->in = (data->up || data->down);
	if (game->data->in) {
		game->data->val += 0.05;
		if (game->data->val > 1) {
			game->data->val = 1;
		}
	} else {
		game->data->val -= 0.05;
		if (game->data->val < 0) {
			game->data->val = 0;
		}
	}

	if (game->data->chime) {
		game->data->chime -= 0.05;
	}
	if (game->data->chime < 0) {
		game->data->chime = 0;
	}

	if (data->level == 4) {
		if (!data->isthisit_triggered) {
			if (data->player->body->center.x > 1920 * 0.75 && data->player->body->center.y < 1080 * 0.3) {
				data->isthisit_triggered = true;
				TM_AddAction(data->timeline, PlayNextVoice, NULL);
				TM_AddAction(data->timeline, WaitForVoice, NULL);
			}
		}
	}
}

void Gamestate_Draw(struct Game* game, struct GamestateResources* data) {
	// Draw everything to the screen here.

	ClearToColor(game, al_map_rgba(0, 0, 0, 0));

	if (!data->world || !data->exit || !data->player) {
		return;
	}

	if (data->shown) {
		for (int i = 0; i < data->entity_num; i++) {
			DrawEntity(game, data->entities[i]);
		}

		float c = 0.9 - sin(game->time * 4) * 0.1;
		al_draw_rectangle(data->exit->body->center.x - 100, data->exit->body->center.y - 100, data->exit->body->center.x + 100, data->exit->body->center.y + 100, al_map_rgb_f(c, c * 1.1, c * 1.1), 5);

		al_draw_rectangle(data->exit->body->center.x - 100 + 8, data->exit->body->center.y - 100 + 8, data->exit->body->center.x + 100 - 8, data->exit->body->center.y + 100 - 8, al_map_rgb_f(c, c * 1.1, c * 1.1), 4);
		al_draw_rectangle(data->exit->body->center.x - 100 + 15, data->exit->body->center.y - 100 + 15, data->exit->body->center.x + 100 - 15, data->exit->body->center.y + 100 - 15, al_map_rgb_f(c, c * 1.1, c * 1.1), 3);
		al_draw_rectangle(data->exit->body->center.x - 100 + 21, data->exit->body->center.y - 100 + 21, data->exit->body->center.x + 100 - 21, data->exit->body->center.y + 100 - 21, al_map_rgb_f(c, c * 1.1, c * 1.1), 2);
		al_draw_rectangle(data->exit->body->center.x - 100 + 26, data->exit->body->center.y - 100 + 26, data->exit->body->center.x + 100 - 26, data->exit->body->center.y + 100 - 26, al_map_rgb_f(c, c * 1.1, c * 1.1), 1);
	}

	DrawEntity(game, data->player);
	if (data->up || data->down) {
		al_draw_filled_circle(data->player->body->center.x, data->player->body->center.y, 8, al_map_rgb(10, 200, 200));

		al_draw_line(data->player->body->center.x, data->player->body->center.y,
			data->player->body->center.x + data->player->body->velocity.x / 8.0, data->player->body->center.y + data->player->body->velocity.y / 8.0,
			al_map_rgb(10, 200, 200), 2);
	}
	if ((!data->pivotlock) && (data->up || data->down || data->w || data->a || data->s || data->d)) {
		vrVec2 pivot = GetPivot(data->player);
		al_draw_filled_circle(pivot.x, pivot.y, 8, al_map_rgb(200, 200, 40));
	}

	if (data->current_voice >= 0) {
		if (al_get_sample_instance_playing(data->voices[data->current_voice].instance)) {
			float pos = al_get_sample_instance_position(data->voices[data->current_voice].instance) / (float)al_get_sample_instance_length(data->voices[data->current_voice].instance) * al_get_sample_instance_time(data->voices[data->current_voice].instance);
			//PrintConsole(game, "%f", pos);
			int i = 0;
			char* txt = "";
			while (LABELS[data->current_voice][i]) {
				if (pos >= POSITIONS[data->current_voice][i]) {
					txt = LABELS[data->current_voice][i];
				}
				i++;
			}

			int width = al_get_text_width(game->data->font, txt);

			float x = data->player->body->center.x + data->player->width * sqrt(2) / 2;
			float y = data->player->body->center.y - 40;

			x = fmin(1910, fmax(10, x));
			y = fmin(1000, fmax(10, y));

			if ((x > 1920 / 2.0) && (x + width > 1920)) {
				x = data->player->body->center.x - data->player->width * sqrt(2) / 2;
				al_draw_multiline_text(game->data->font, al_map_rgb(255, 255, 255), x, y, x, 64, ALLEGRO_ALIGN_RIGHT, txt);
			} else {
				if (x + width > 1920) {
					al_draw_multiline_text(game->data->font, al_map_rgb(255, 255, 255), x, y, 1920 - x, 64, ALLEGRO_ALIGN_LEFT, txt);
				} else {
					DrawTextWithShadow(game->data->font, al_map_rgb(255, 255, 255), x, y, ALLEGRO_ALIGN_LEFT, txt);
				}
			}
		}
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

	int skipbtn = 6;
#ifdef __SWITCH__
	skipbtn = 11;
#endif

	if (((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_FULLSTOP)) || ((ev->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) && (ev->joystick.button == skipbtn))) {
		if (data->current_voice >= 0) {
			al_set_sample_instance_playing(data->voices[data->current_voice].instance, false);
		}
	}

	if (game->config.debug.enabled) {
		if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ENTER)) {
			Win(game, data);
		}
	}

	if (data->inputlock) {
		return;
	}

	if (((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_BACKSPACE)) || ((ev->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) && (ev->joystick.button == 1))) {
		Restart(game, data);
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

#ifdef __SWITCH__
	if ((ev->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) && (ev->joystick.button == 13)) {
		data->w = true;
	}
	if ((ev->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP) && (ev->joystick.button == 13)) {
		data->w = false;
	}

	if ((ev->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) && (ev->joystick.button == 12)) {
		data->a = true;
	}
	if ((ev->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP) && (ev->joystick.button == 12)) {
		data->a = false;
	}

	if ((ev->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) && (ev->joystick.button == 15)) {
		data->s = true;
	}
	if ((ev->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP) && (ev->joystick.button == 15)) {
		data->s = false;
	}

	if ((ev->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) && (ev->joystick.button == 14)) {
		data->d = true;
	}
	if ((ev->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP) && (ev->joystick.button == 14)) {
		data->d = false;
	}
#endif

	if (ev->type == ALLEGRO_EVENT_JOYSTICK_AXIS) {
		if (game->config.debug.verbose) {
			PrintConsole(game, "id0 %d stick: %d axis %d pos %f", al_get_joystick(0) == ev->joystick.id, ev->joystick.stick, ev->joystick.axis, ev->joystick.pos);
		}
	}

	if (ev->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) {
		if (game->config.debug.verbose) {
			PrintConsole(game, "id0 %d button down: %d", al_get_joystick(0) == ev->joystick.id, ev->joystick.button);
		}
	}

	if (ev->type == ALLEGRO_EVENT_JOYSTICK_AXIS && ev->joystick.stick == 1) {
		if (ev->joystick.axis == 1) {
			if (ev->joystick.pos < -0.25) {
				data->up = true;
			} else {
				data->up = false;
			}
			if (ev->joystick.pos > 0.25) {
				data->down = true;
			} else {
				data->down = false;
			}
		}
	}
	if (ev->type == ALLEGRO_EVENT_JOYSTICK_AXIS && (ev->joystick.stick == 0 || ev->joystick.stick == 3)) {
		if (ev->joystick.axis == 1) {
			if (ev->joystick.pos < -0.25) {
				data->w = true;
			} else {
				data->w = false;
			}
			if (ev->joystick.pos > 0.25) {
				data->s = true;
			} else {
				data->s = false;
			}
		}
		if (ev->joystick.axis == 0) {
			if (ev->joystick.pos < -0.25) {
				data->a = true;
			} else {
				data->a = false;
			}
			if (ev->joystick.pos > 0.25) {
				data->d = true;
			} else {
				data->d = false;
			}
		}
	}

	if (ev->type == ALLEGRO_EVENT_JOYSTICK_AXIS && ev->joystick.stick == 2) {
		if (ev->joystick.axis == 0) {
			if (ev->joystick.pos > -1) {
				data->down = true;
			} else {
				data->down = false;
			}
		}
		if (ev->joystick.axis == 1) {
			if (ev->joystick.pos > -1) {
				data->up = true;
			} else {
				data->up = false;
			}
		}
	}

	if (data->growlock) {
		if (data->up || data->down) {
			data->triedtomove = true;
		}
		data->up = false;
		data->down = false;
	}
	if (data->up) {
		data->upped = true;
	}
	if (data->down) {
		data->downed = true;
	}
	if (data->pivotlock) {
		if (data->w || data->a || data->s || data->d) {
			data->triedtomove = true;
		}
		data->w = false;
		data->a = false;
		data->s = false;
		data->d = false;
	}
	if (data->w || data->a || data->s || data->d) {
		data->pivoted = true;
	}
}

void* Gamestate_Load(struct Game* game, void (*progress)(struct Game*)) {
	// Called once, when the gamestate library is being loaded.
	// Good place for allocating memory, loading bitmaps etc.
	//
	// Keep in mind that there's no OpenGL context available here. If you want to prerender something,
	// create VBOs, etc. do it in Gamestate_PostLoad.

	struct GamestateResources* data = calloc(1, sizeof(struct GamestateResources));

	data->timeline = TM_Init(game, data, "bob");

	progress(game); // report that we progressed with the loading, so the engine can move a progress bar

	for (int i = 0; i < 17; i++) {
		data->voices[i].sample = al_load_sample(GetDataFilePath(game, FILES[i]));
		data->voices[i].instance = al_create_sample_instance(data->voices[i].sample);
		al_attach_sample_instance_to_mixer(data->voices[i].instance, game->audio.voice);
		progress(game);
	}

	return data;
}

void Gamestate_Unload(struct Game* game, struct GamestateResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	for (int i = 0; i < 17; i++) {
		al_destroy_sample_instance(data->voices[i].instance);
		al_destroy_sample(data->voices[i].sample);
	}
	TM_Destroy(data->timeline);
	free(data);
}

void Gamestate_Start(struct Game* game, struct GamestateResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	al_set_audio_stream_playing(game->data->music, true);
	data->current_voice = -1;
	data->fab_voice = -1;
	data->growlock = true;
	data->pivotlock = true;
	data->inputlock = true;
	data->shown = false;
	data->die_counter = 0;
	data->level = 0;
	data->triedtomove = false;
	data->entity_num = 0;
	data->pivoted = false;
	data->upped = false;
	data->downed = false;
	data->w = false;
	data->a = false;
	data->s = false;
	data->d = false;
	data->up = false;
	data->down = false;
	data->isthisit_triggered = false;

	TM_CleanQueue(data->timeline);
	TM_AddDelay(data->timeline, 1);
	TM_AddAction(data->timeline, PlayNextVoice, NULL);
	TM_AddAction(data->timeline, WaitForVoice, NULL);
	TM_AddAction(data->timeline, UnlockInput, NULL);
	TM_AddAction(data->timeline, WaitForTryMove, NULL);
	TM_AddAction(data->timeline, PlayNextVoice, NULL);
	TM_AddAction(data->timeline, WaitForVoice, NULL);
	TM_AddAction(data->timeline, UnlockGrow, NULL);
	TM_AddAction(data->timeline, WaitForUpped, NULL);
	TM_AddAction(data->timeline, PlayNextVoice, NULL);
	TM_AddAction(data->timeline, WaitForVoice, NULL);
	TM_AddAction(data->timeline, WaitForDowned, NULL);
	TM_AddAction(data->timeline, PlayNextVoice, NULL);
	TM_AddAction(data->timeline, WaitForVoice, NULL);
	TM_AddAction(data->timeline, UnlockPivot, NULL);
	TM_AddAction(data->timeline, WaitForPivoted, NULL);
	TM_AddAction(data->timeline, PlayNextVoice, NULL);
	TM_AddAction(data->timeline, WaitForVoice, NULL);
	TM_AddAction(data->timeline, ShowStuff, NULL);
	TM_AddAction(data->timeline, PlayNextVoice, NULL);
	TM_AddAction(data->timeline, WaitForVoice, NULL);
	TM_AddDelay(data->timeline, 3);
	TM_AddAction(data->timeline, PlayNextVoice, NULL);
	TM_AddAction(data->timeline, WaitForVoice, NULL);

	StartLevel(game, data, 0);
}

void Gamestate_Stop(struct Game* game, struct GamestateResources* data) {
	// Called when gamestate gets stopped. Stop timers, music etc. here.
	DestroyPhysics(game, data);
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
