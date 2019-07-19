/*! \file common.c
 *  \brief Common stuff that can be used by all gamestates.
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

#include "common.h"
#include <libsuperderpy.h>

struct Entity* CreateEntity(struct Game* game, vrWorld* world, float x, float y, float w, float h, float mass, float friction, float restitution, bool gravity) {
	vrRigidBody* body = vrBodyInit(vrBodyAlloc());
	if (mass >= 0) {
		body->bodyMaterial.mass = mass;
		body->bodyMaterial.invMass = mass < 0 ? 0 : (1.0 / body->bodyMaterial.mass);
		body->bodyMaterial.momentInertia = vrMomentForBox(w, h, mass);
		body->bodyMaterial.invMomentInertia = 1.0 / body->bodyMaterial.momentInertia;
	} else {
		body->bodyMaterial.invMass = 0;
		body->bodyMaterial.invMomentInertia = 0;
	}
	body->bodyMaterial.friction = friction;
	body->bodyMaterial.restitution = restitution;
	body->gravity = gravity;
	vrWorldAddBody(world, body);

	vrShape* shape = vrShapeInit(vrShapeAlloc());
	shape = vrShapePolyInit(shape);
	shape->shape = vrPolyBoxInit(shape->shape, x, y, w, h);
	vrArrayPush(body->shape, shape);

	struct Entity* entity = calloc(1, sizeof(struct Entity));
	entity->body = body;
	entity->shape = shape;
	entity->width = w;
	entity->height = h;
	return entity;
}

vrVec2 rescale(vrVec2 v, vrVec2 center, float scale) {
	return vrVect(center.x + (v.x - center.x) * scale, center.y + (v.y - center.y) * scale);
}

void ChangeEntitySize(struct Game* game, struct Entity* entity, float scale) {
	vrPolygonShape* pshape = ((vrShape*)entity->body->shape->data[0])->shape;
	vrVec2 v1 = pshape->vertices[0];
	vrVec2 v2 = pshape->vertices[1];
	vrVec2 v3 = pshape->vertices[2];
	vrVec2 v4 = pshape->vertices[3];
	vrVec2 center = pshape->center;

	vrArrayPop(entity->body->shape);
	//vrShapeDestroy(entity->shape);

	vrShape* shape = vrShapeInit(vrShapeAlloc());

	//shape->shape = vrPolyBoxInit(shape->shape, 10, 10, width, height);

	shape = vrShapePolyInit(shape);
	vrAddVertexToPolyShape(shape->shape, rescale(v1, center, scale));
	vrAddVertexToPolyShape(shape->shape, rescale(v2, center, scale));
	vrAddVertexToPolyShape(shape->shape, rescale(v3, center, scale));
	vrAddVertexToPolyShape(shape->shape, rescale(v4, center, scale));

	vrArrayPush(entity->body->shape, shape);
	entity->width *= scale;
	entity->height *= scale;
}

void DrawEntity(struct Game* game, struct Entity* entity) {
	vrPolygonShape* pshape = ((vrShape*)entity->body->shape->data[0])->shape;
	vrVec2 topleft = pshape->vertices[0];
	PrintConsole(game, "%f %f", topleft.x, topleft.y);

	ALLEGRO_VERTEX v[] = {
		{.x = pshape->vertices[0].x, .y = pshape->vertices[0].y, .color = al_map_rgb(255, 255, 255)},
		{.x = pshape->vertices[1].x, .y = pshape->vertices[1].y, .color = al_map_rgb(255, 255, 255)},
		{.x = pshape->vertices[2].x, .y = pshape->vertices[2].y, .color = al_map_rgb(255, 255, 255)},

		{.x = pshape->vertices[0].x, .y = pshape->vertices[0].y, .color = al_map_rgb(255, 255, 255)},
		{.x = pshape->vertices[2].x, .y = pshape->vertices[2].y, .color = al_map_rgb(255, 255, 255)},
		{.x = pshape->vertices[3].x, .y = pshape->vertices[3].y, .color = al_map_rgb(255, 255, 255)}};
	al_draw_prim(v, NULL, NULL, 0, 6, ALLEGRO_PRIM_TRIANGLE_LIST);
}

bool GlobalEventHandler(struct Game* game, ALLEGRO_EVENT* ev) {
	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_M)) {
		ToggleMute(game);
	}

	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_F)) {
		ToggleFullscreen(game);
	}

	return false;
}

struct CommonResources* CreateGameData(struct Game* game) {
	struct CommonResources* data = calloc(1, sizeof(struct CommonResources));
	return data;
}

void DestroyGameData(struct Game* game) {
	free(game->data);
}
