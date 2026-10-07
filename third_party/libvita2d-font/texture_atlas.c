#include <stdlib.h>
#include <string.h>
#include "texture_atlas.h"

texture_atlas *texture_atlas_create(int width, int height, SceGxmTextureFormat format)
{
	texture_atlas *atlas = malloc(sizeof(*atlas));
	if (!atlas)
		return NULL;

	bp2d_rectangle rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = width;
	rect.h = height;

	atlas->texture = vita2d_create_empty_texture_format(width,
							    height,
							    format);
	if (!atlas->texture) {
		free(atlas);
		return NULL;
	}

	atlas->texture->gxm_rtgt = NULL;
	atlas->texture->depth_UID = 0;
	memset(vita2d_texture_get_datap(atlas->texture), 0,
	       vita2d_texture_get_stride(atlas->texture) * height);
	atlas->bp_root = bp2d_create(&rect);
	atlas->htab = int_htab_create(256);

	vita2d_texture_set_filters(atlas->texture,
				   SCE_GXM_TEXTURE_FILTER_POINT,
				   SCE_GXM_TEXTURE_FILTER_LINEAR);

	return atlas;
}

void texture_atlas_free(texture_atlas *atlas)
{
	vita2d_free_texture(atlas->texture);
	bp2d_free(atlas->bp_root);
	int_htab_free(atlas->htab);
	free(atlas);
}

int texture_atlas_insert(texture_atlas *atlas, unsigned int character,
			 const bp2d_size *size,
			 const texture_atlas_entry_data *data,
			 bp2d_position *inserted_pos)
{
	atlas_htab_entry *entry;
	bp2d_node *new_node;

	bp2d_size padded = { size->w + 4, size->h + 4 };
	if (!bp2d_insert(atlas->bp_root, &padded, inserted_pos, &new_node))
		return 0;

	inserted_pos->x += 2;
	inserted_pos->y += 2;
	entry = malloc(sizeof(*entry));

	entry->rect.x = inserted_pos->x;
	entry->rect.y = inserted_pos->y;
	entry->rect.w = size->w;
	entry->rect.h = size->h;
	entry->data = *data;

	if (!int_htab_insert(atlas->htab, character, entry)) {
		bp2d_delete(atlas->bp_root, new_node);
		return 0;
	}

	return 1;
}

int texture_atlas_exists(texture_atlas *atlas, unsigned int character)
{
	return int_htab_find(atlas->htab, character) != NULL;
}

int texture_atlas_get(texture_atlas *atlas, unsigned int character,
		      bp2d_rectangle *rect, texture_atlas_entry_data *data)
{
	atlas_htab_entry *entry = int_htab_find(atlas->htab, character);
	if (!entry)
		return 0;

	*rect = entry->rect;
	*data = entry->data;

	return 1;
}
