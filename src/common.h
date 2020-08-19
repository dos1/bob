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

#define LIBSUPERDERPY_DATA_TYPE struct CommonResources
#include <libsuperderpy.h>
#include <vrRigidBody.h>
#include <vrWorld.h>

struct Entity {
	vrRigidBody* body;
	vrShape* shape;
	vrWorld* world;
	float width, height;
	float pivotX, pivotY;
	int kind;
};

struct CommonResources {
	// Fill in with common data accessible from all gamestates.
	ALLEGRO_BITMAP *blur1, *blur2, *buffer, *target, *tmp;
	ALLEGRO_SHADER *kawese_shader, *ghost_shader, *dis_shader;
	ALLEGRO_BITMAP* displacement;
	ALLEGRO_FONT* font;
	ALLEGRO_COLOR tint;
	ALLEGRO_AUDIO_STREAM* music;
	ALLEGRO_MIXER* mixer;
	bool in;
	float val, chime;

	struct {
		bool enabled;
		bool wasd, updown;
		bool w, a, s, d;
		bool up, down;
	} hud;
};

bool IsInside(vrPolygonShape* shape, vrVec2 v);
void Compositor(struct Game* game);
vrVec2 GetPivot(struct Entity* entity);
void ChangeEntitySize(struct Game* game, struct Entity* entity, float scale);
struct Entity* CreateEntity(struct Game* game, vrWorld* world, float x, float y, float w, float h, float mass, float friction, float restitution, bool gravity, int kind);
void DrawEntity(struct Game* game, struct Entity* entity);
struct CommonResources* CreateGameData(struct Game* game);
void DestroyGameData(struct Game* game);
bool GlobalEventHandler(struct Game* game, ALLEGRO_EVENT* ev);
