/*! \file example.c
 *  \brief Example gamestate.
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

struct GamestateResources {
	ALLEGRO_FONT* font;
	ALLEGRO_AUDIO_STREAM* stream;
	float counter;
};

int Gamestate_ProgressCount = 1;

void Gamestate_Logic(struct Game* game, struct GamestateResources* data, double delta) {
	data->counter += delta;
}

void Gamestate_Draw(struct Game* game, struct GamestateResources* data) {
	ClearToColor(game, al_map_rgb(255, 255, 255));
	if (data->counter > 2.0) {
		al_draw_text(game->data->font, al_map_rgb(0, 0, 0), 1920 / 2, 1080 / 2, ALLEGRO_ALIGN_CENTER, "Press a button to play again.");
	}
}

void Gamestate_ProcessEvent(struct Game* game, struct GamestateResources* data, ALLEGRO_EVENT* ev) {
	if (ev->type == ALLEGRO_EVENT_KEY_DOWN && data->counter > 2.0) {
		SwitchCurrentGamestate(game, "empty");
	}
}

void* Gamestate_Load(struct Game* game, void (*progress)(struct Game*)) {
	struct GamestateResources* data = calloc(1, sizeof(struct GamestateResources));
	data->stream = al_load_audio_stream(GetDataFilePath(game, "heaven.wav"), 4, 2048);
	al_set_audio_stream_playing(data->stream, false);
	al_set_audio_stream_playmode(data->stream, ALLEGRO_PLAYMODE_ONCE);
	al_attach_audio_stream_to_mixer(data->stream, game->audio.fx);

	int flags = al_get_new_bitmap_flags();
	al_set_new_bitmap_flags(flags & ~ALLEGRO_MAG_LINEAR); // disable linear scaling for pixelarty appearance
	data->font = al_create_builtin_font();
	progress(game); // report that we progressed with the loading, so the engine can move a progress bar
	al_set_new_bitmap_flags(flags);

	return data;
}

void Gamestate_Unload(struct Game* game, struct GamestateResources* data) {
	al_destroy_audio_stream(data->stream);
	free(data);
}

void Gamestate_Start(struct Game* game, struct GamestateResources* data) {
	al_set_audio_stream_playing(data->stream, true);
}

void Gamestate_Stop(struct Game* game, struct GamestateResources* data) {}
