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

#define BLUR_DIVIDER 4.0

#include "common.h"
#include <libsuperderpy.h>

struct Entity* CreateEntity(struct Game* game, vrWorld* world, float x, float y, float w, float h, float mass, float friction, float restitution, bool gravity, int kind) {
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
	entity->world = world;
	entity->pivotX = 0.5;
	entity->pivotY = 1.0;
	entity->kind = kind;
	return entity;
}

static vrVec2 rescale(vrVec2 v, vrVec2 center, float scale) {
	return vrVect(center.x + (v.x - center.x) * scale, center.y + (v.y - center.y) * scale);
}

bool IsInside(vrPolygonShape* shape, vrVec2 v) {
	float testx = v.x;
	float testy = v.y;
	int i, j;
	bool c = 0;
	for (i = 0, j = shape->num_vertices - 1; i < shape->num_vertices; j = i++) {
		if (((shape->vertices[i].y + 1 > testy) != (shape->vertices[j].y + 1 > testy)) &&
			(testx < 1 + (shape->vertices[j].x - shape->vertices[i].x) * (testy - shape->vertices[i].y) / (shape->vertices[j].y - shape->vertices[i].y) + shape->vertices[i].x))
			c = !c;
	}
	return c;
}

static vrVec2 minus(vrVec2 minusee, vrVec2 minuser) {
	return vrVect(minusee.x - minuser.x, minusee.y - minuser.y);
}

static vrVec2 plus(vrVec2 plusee, vrVec2 pluser) {
	return vrVect(plusee.x + pluser.x, plusee.y + pluser.y);
}

static vrVec2 multiply(vrVec2 vec, float factor) {
	return vrVect(vec.x * factor, vec.y * factor);
}

static vrVec2 veclerp(vrVec2 from, vrVec2 to, float factor) {
	return plus(from, multiply(minus(to, from), factor));
}

vrVec2 GetPivot(struct Entity* entity) {
	vrPolygonShape* pshape = ((vrShape*)entity->body->shape->data[0])->shape;
	vrVec2 v1 = pshape->vertices[0];
	vrVec2 v2 = pshape->vertices[1];
	//vrVec2 v3 = pshape->vertices[2];
	vrVec2 v4 = pshape->vertices[3];

	vrVec2 centerx = veclerp(v1, v2, entity->pivotX);
	vrVec2 centery = veclerp(v1, v4, entity->pivotY);
	//vrVec2 center = vrVect(centerx.x, centery.y);
	return plus(minus(centerx, v1), centery);
}

void ChangeEntitySize(struct Game* game, struct Entity* entity, float scale) {
	vrPolygonShape* pshape = ((vrShape*)entity->body->shape->data[0])->shape;
	vrVec2 v1 = pshape->vertices[0];
	vrVec2 v2 = pshape->vertices[1];
	vrVec2 v3 = pshape->vertices[2];
	vrVec2 v4 = pshape->vertices[3];

	vrVec2 center = GetPivot(entity);

	v1 = rescale(v1, center, scale);
	v2 = rescale(v2, center, scale);
	v3 = rescale(v3, center, scale);
	v4 = rescale(v4, center, scale);

	if (vrDist(v1, v2) < 75) {
		return;
	}
	if (vrDist(v1, v2) > 300) {
		return;
	}

	if (scale > 1.0) {
		for (int i = 0; i < entity->world->bodies->sizeof_active; i++) {
			vrRigidBody* body = entity->world->bodies->data[i];
			vrPolygonShape* shape = ((vrShape*)body->shape->data[0])->shape;
			if (body->collisionData.categoryMask == 0) {
				continue;
			}
			if (body != entity->body && (IsInside(shape, v1) || IsInside(shape, v2) || IsInside(shape, v3) || IsInside(shape, v4))) {
				return;
			}
			vrVec2 c = shape->center;
			if (body != entity->body && (IsInside(pshape, rescale(shape->vertices[0], c, 0.9)) || IsInside(pshape, rescale(shape->vertices[1], c, 0.9)) || IsInside(pshape, rescale(shape->vertices[2], c, 0.9)) || IsInside(pshape, rescale(shape->vertices[3], c, 0.9)))) {
				return;
			}
			//shape->vertices[i];
		}
	}

	vrArrayPop(entity->body->shape);
	//vrShapeDestroy(entity->shape);
	vrShape* shape = vrShapeInit(vrShapeAlloc());

	shape = vrShapePolyInit(shape);
	vrAddVertexToPolyShape(shape->shape, v1);
	vrAddVertexToPolyShape(shape->shape, v2);
	vrAddVertexToPolyShape(shape->shape, v3);
	vrAddVertexToPolyShape(shape->shape, v4);

	vrArrayPush(entity->body->shape, shape);
	entity->width *= scale;
	entity->height *= scale;
}

void DrawEntity(struct Game* game, struct Entity* entity) {
	vrPolygonShape* pshape = ((vrShape*)entity->body->shape->data[0])->shape;
	//vrVec2 topleft = pshape->vertices[0];
	//PrintConsole(game, "%f %f", topleft.x, topleft.y);
	if (entity->kind == 2) return;
	ALLEGRO_COLOR color = entity->kind ? al_map_rgb(255, 255, 255) : al_map_rgb(200, 220, 230);

	ALLEGRO_VERTEX v[] = {
		{.x = pshape->vertices[0].x, .y = pshape->vertices[0].y, .color = color},
		{.x = pshape->vertices[1].x, .y = pshape->vertices[1].y, .color = color},
		{.x = pshape->vertices[2].x, .y = pshape->vertices[2].y, .color = color},

		{.x = pshape->vertices[0].x, .y = pshape->vertices[0].y, .color = color},
		{.x = pshape->vertices[2].x, .y = pshape->vertices[2].y, .color = color},
		{.x = pshape->vertices[3].x, .y = pshape->vertices[3].y, .color = color},
	};
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

void Compositor(struct Game* game) {
	al_set_target_bitmap(game->data->target);
	ClearToColor(game, al_map_rgba(0, 0, 0, 0));

	struct Gamestate* tmp = GetNextGamestate(game, NULL);
	while (tmp) {
		if (IsGamestateVisible(game, tmp)) {
			al_draw_bitmap(GetGamestateFramebuffer(game, tmp), game->clip_rect.x, game->clip_rect.y, 0);
		}
		tmp = GetNextGamestate(game, tmp);
	}
	if (game->loading.shown) {
		al_draw_bitmap(GetGamestateFramebuffer(game, GetGamestate(game, NULL)), game->clip_rect.x, game->clip_rect.y, 0);
	}

	al_set_target_bitmap(game->data->tmp);
	ClearToColor(game, al_map_rgba(0, 0, 0, 0));
	al_draw_tinted_bitmap(game->data->buffer, al_map_rgba_f(0.75, 0.85, 0.85, 0.85), 0, -game->clip_rect.h * 0.0015, 0);

	float size[2] = {al_get_bitmap_width(game->data->tmp), al_get_bitmap_height(game->data->tmp)};

	al_set_target_bitmap(game->data->blur1);
	al_clear_to_color(al_map_rgb(0, 0, 0));
	al_use_shader(game->data->kawese_shader);
	al_clear_to_color(al_map_rgb(0, 0, 0));
	al_set_shader_float_vector("size", 2, size, 1);
	al_set_shader_float("kernel", 0);
	al_draw_scaled_bitmap(game->data->tmp, 0, 0, size[0], size[1], 0, 0, size[0] / BLUR_DIVIDER, size[1] / BLUR_DIVIDER, 0);
	al_use_shader(NULL);

	al_set_target_bitmap(game->data->blur2);
	al_clear_to_color(al_map_rgb(0, 0, 0));
	al_use_shader(game->data->kawese_shader);
	al_set_shader_float_vector("size", 2, size, 1);
	al_set_shader_float("kernel", 1);
	al_draw_bitmap(game->data->blur1, 0, 0, 0);
	al_use_shader(NULL);

	al_set_target_bitmap(game->data->buffer);
	ClearToColor(game, al_map_rgba(0, 0, 0, 0));

	al_use_shader(game->data->ghost_shader);
	al_set_shader_bool("invert", false);
	al_set_shader_float("time", game->time);
	float vertices[4] = {0.0, 0.0, al_get_bitmap_width(game->data->blur2), al_get_bitmap_height(game->data->blur2)};
	al_set_shader_float_vector("vertices", 4, vertices, 1);

	float tex_whole_pixel_size[2] = {al_get_bitmap_width(game->data->blur2), al_get_bitmap_height(game->data->blur2)};
	al_set_shader_float_vector("tex_whole_pixel_size", 2, tex_whole_pixel_size, 1);

	float tex_boundaries[4] = {(al_get_bitmap_x(game->data->blur2) - 1) / tex_whole_pixel_size[0],
		1.0 - ((al_get_bitmap_y(game->data->blur2) + al_get_bitmap_height(game->data->blur2) + 1) / tex_whole_pixel_size[1]),
		(al_get_bitmap_x(game->data->blur2) + al_get_bitmap_width(game->data->blur2) + 1) / tex_whole_pixel_size[0],
		1.0 - ((al_get_bitmap_y(game->data->blur2) - 1) / tex_whole_pixel_size[1])};
	al_set_shader_float_vector("tex_boundaries", 4, tex_boundaries, 1);

	al_set_shader_float("zoom", 1.0);

	al_set_shader_bool("inplace", false);
	al_set_shader_bool("active", false);
	al_draw_tinted_scaled_bitmap(game->data->blur2, al_map_rgba_f(1, 1, 1, 1), 0, 0, size[0] / BLUR_DIVIDER, size[1] / BLUR_DIVIDER, 0, 0, size[0], size[1], 0);
	al_use_shader(NULL);

	al_draw_bitmap(game->data->target, 0, 0, 0);
	al_draw_tinted_scaled_bitmap(game->data->blur2, al_map_rgba_f(0.5, 0.5, 0.5, 0.5), 0, 0, size[0] / BLUR_DIVIDER, size[1] / BLUR_DIVIDER, 0, 0, size[0], size[1], 0);

	al_set_target_backbuffer(game->display);
	ClearToColor(game, al_map_rgb(0, 0, 0));
	al_draw_bitmap(game->data->buffer, 0, 0, 0);
}

struct CommonResources* CreateGameData(struct Game* game) {
	struct CommonResources* data = calloc(1, sizeof(struct CommonResources));
	data->blur1 = CreateNotPreservedBitmap(al_get_display_width(game->display) / BLUR_DIVIDER, al_get_display_height(game->display) / BLUR_DIVIDER);
	data->blur2 = CreateNotPreservedBitmap(al_get_display_width(game->display) / BLUR_DIVIDER, al_get_display_height(game->display) / BLUR_DIVIDER);
	data->buffer = CreateNotPreservedBitmap(al_get_display_width(game->display), al_get_display_height(game->display));
	data->target = CreateNotPreservedBitmap(al_get_display_width(game->display), al_get_display_height(game->display));
	data->tmp = CreateNotPreservedBitmap(al_get_display_width(game->display), al_get_display_height(game->display));
	data->kawese_shader = CreateShader(game, GetDataFilePath(game, "shaders/vertex.glsl"), GetDataFilePath(game, "shaders/kawese.glsl"));
	data->ghost_shader = CreateShader(game, GetDataFilePath(game, "shaders/vertex.glsl"), GetDataFilePath(game, "shaders/ghosttree.glsl"));
	al_set_target_bitmap(data->buffer);
	al_clear_to_color(al_map_rgba(0, 0, 0, 0));
	al_set_target_backbuffer(game->display);
	return data;
}

void DestroyGameData(struct Game* game) {
	free(game->data);
}
