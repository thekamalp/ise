// InfiniScroll Engine
// ise_gfx.h
// Graphics routines
//
// Kamal Pillai
// 1/13/2019

#ifndef __ISE_GFX_H
#define __ISE_GFX_H

#include "ise_vga.h"
#include "ise_bmp.h"
#include "ise_gfx3.h"

#define ISE_XFORM_NONE 0x0
#define ISE_XFORM_FLIP_XY 0x1
#define ISE_XFORM_HFLIP 0x2
#define ISE_XFORM_VFLIP 0x4
#define ISE_XFORM_ROT_90CW 0x3
#define ISE_XFORM_ROT_180 0x6
#define ISE_XFORM_ROT_270CW 0x5

#define ISE_XFORM_SHADOW 0x10

#define ISE_NO_COLOR -1

#define ISE_MASK_NONE 0x0
#define ISE_MASK_SPRITE 0x1
#define ISE_MASK_EDGE 0x2

typedef struct {
    uint32_t vmem_offset;
    uint16_t width;
    uint16_t height;
    uint16_t pitch;
    uint16_t hpan;
} ise_framebuffer_t;

#define ISE_MAX_APERTURE_ROWS 8

typedef struct {
	uint32_t vmem_offset;
	uint32_t remaining;
	uint32_t width;
	uint32_t height;
} ise_aperture_row_t;

typedef struct {
    uint32_t start;
    uint32_t base;
    uint32_t size;
	uint32_t next_row;   // next row to allocate
	ise_aperture_row_t rows[ISE_MAX_APERTURE_ROWS];
} ise_aperture_t;

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
} ise_color_t;

typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t* color;
    uint32_t* mask;
} ise_sprite_t;

typedef struct {
	uint16_t width;
	uint16_t height;
	uint32_t* mask[256];
} ise_font_t;

typedef struct {
    ise_sprite_t sprite;
    uint32_t vmem_offset;
    ise_aperture_t* aperture;
	uint32_t xforms;
	uint32_t bg_color;
} ise_fast_sprite_t;

typedef struct {
	uint16_t min_x;
	uint16_t min_y;
	uint16_t width;
	uint16_t height;
} ise_bbox_t;

typedef struct {
	int32_t min_x;
	int32_t min_y;
	int32_t width;
	int32_t height;
} ise_bbox32_t;

typedef struct {
	uint16_t duration;
	uint16_t xform;
	int16_t origin_x;
	int16_t origin_y;
	ise_bbox_t hitbox;
	ise_fast_sprite_t* fsp;
} ise_anim_frame_t;

typedef struct {
    uint16_t num_frames;
	uint16_t frame_restart;
	ise_anim_frame_t* frames;
} ise_anim_sprite_t;

typedef struct {
	uint32_t num_sprites;
	uint32_t num_anim_sprites;
	ise_fast_sprite_t* fsp;
	ise_anim_sprite_t* anim_sprite;
} ise_anim_set_t;

typedef struct {
	uint16_t anim;
    uint16_t frame;
    uint16_t frame_time;
	uint16_t flags;
    uint32_t pos_x;
    uint32_t pos_y;
	uint16_t frac_pos_x;
	uint16_t frac_pos_y;
	int32_t vec_x;
	int32_t vec_y;
    ise_anim_sprite_t* anim_sprite;
} ise_anim_object_t;

#pragma pack(push, 1)
typedef struct {
    char id[2];
    uint8_t log2_tile_width;
    uint8_t log2_tile_height;
    uint16_t map_width;
    uint16_t map_height;
    uint32_t origin_x;
    uint32_t origin_y;
    uint16_t sprite_id_start;
    uint16_t num_sprites;
    uint16_t num_fast_sprites;
	uint16_t dummy;
    ise_fast_sprite_t* sprites;
	uint8_t* tile_type;
    uint8_t* tiles;
} ise_map_t;
#pragma pack(pop)

const int MAX_SPRITES = 64;

typedef struct {
    ise_framebuffer_t buffers[2];
    ise_framebuffer_t* front;
    ise_framebuffer_t* back;
    ise_aperture_t assets;
    uint8_t fg_color;
    uint8_t bg_color;
    ise_map_t map;
	bool map_drawn;
    bool need_prev_scroll;
    int prev_scroll_x;
    int prev_scroll_y;
	int num_front_sprites;
	int num_back_sprites;
	ise_bbox32_t* front_sprite_bbox;
	ise_bbox32_t* back_sprite_bbox;
	ise_bbox32_t sprite_bbox[2*MAX_SPRITES];
} ise_gfx_t;

extern void* ise_vram_base;
extern uint32_t ise_vram_size_64k;
extern ise_vmode_t ise_screen_mode;

extern void (*ise_svga_set_rgb8_palette)();
extern int (*ise_svga_set_mode)(ise_gfx_t* gfx);
extern void (*ise_svga_setup_mode)(ise_gfx_t* gfx);
extern void (*ise_svga_update_display)(ise_gfx_t* gfx);
extern void (*ise_svga_upload_fast_sprite_xform)(ise_gfx_t* gfx, ise_fast_sprite_t* fsp);
extern void (*ise_svga_fill)(ise_framebuffer_t* fb, int x, int y, uint16_t width, uint16_t height, uint32_t color);
extern void (*ise_svga_draw_fast_sprite)(ise_fast_sprite_t* fsp, ise_framebuffer_t* fb, int x, int y);
extern void (*ise_svga_draw_sprite)(ise_fast_sprite_t* fsp, ise_framebuffer_t* fb, int x, int y, uint16_t xform);
extern void (*ise_svga_wait_idle)();
extern void (*ise_svga_uninstall)();

void ise_valloc(ise_aperture_t* ap, ise_framebuffer_t* fb, uint8_t bpp);
void ise_vfree_all(ise_aperture_t* ap);

void ise_gfx_get_mode(ise_vmode_t* vmode);
uint16_t ise_gfx_get_supported_modes(uint16_t max_modes, ise_vmode_t* vmode);
void ise_gfx_init(ise_gfx_t* gfx, ise_vmode_t* vmode, uint16_t max_vscroll, uint16_t padding_request = 0);
void ise_gfx_clear(ise_framebuffer_t* fb, uint32_t clear_color);
void ise_gfx_upload_fast_sprite(ise_gfx_t* gfx, ise_fast_sprite_t* fsp);
void ise_gfx_set_map_origin(ise_gfx_t* gfx, uint32_t x, uint32_t y);
void ise_gfx_scroll(ise_gfx_t* gfx, int x, int y);
void ise_gfx_clear_map(ise_gfx_t* gfx, ise_bbox32_t* bbox);
void ise_gfx_swap_buffers(ise_gfx_t* gfx);
void ise_gfx_update_display(ise_gfx_t* gfx);
void ise_gfx_load_map(ise_gfx_t* gfx, const char* filename);
void ise_gfx_free_map(ise_gfx_t* gfx);
void ise_draw_map(ise_map_t* map, ise_framebuffer_t* fb, int offset_x, int offset_y, uint8_t fill_color, uint16_t mask_flags);
uint8_t ise_get_tile_type(ise_map_t* map, int x, int y);
uint32_t ise_check_map_collision(ise_map_t* map, ise_anim_object_t* obj, uint16_t ledge_depth);

uint8_t ise_get_closest_rgb8_color(ise_color_t* c);
void ise_load_sprite(ise_sprite_t* sp, const char* filename);
void ise_free_sprite_color(ise_sprite_t* sp);
void ise_free_sprite_mask(ise_sprite_t* sp);
void ise_free_sprite(ise_sprite_t* sp);

void ise_set_plane(uint16_t cur_plane);
void ise_draw_text(ise_gfx_t* gfx, int x, int y, const char* str, uint16_t xform);
int ise_draw_sprite(ise_sprite_t* sp, ise_framebuffer_t* fb, int x, int y, uint16_t xform, int fill_color, uint16_t cur_plane);
void ise_fast_fill(ise_framebuffer_t* fb, int x, int y, uint16_t width, uint16_t height, uint8_t fill_color, uint16_t mask_flags);

void ise_load_anim_set(ise_gfx_t* gfx, ise_anim_set_t* anim_set, const char* filename);
void ise_free_anim_set(ise_anim_set_t* anim_set);

void ise_clear_anim_objects(ise_gfx_t* gfx, uint32_t num_objs, ise_anim_object_t** objs);
void ise_draw_anim_objects(ise_gfx_t* gfx, uint32_t num_objs, ise_anim_object_t** objs);

bool ise_animate_object(ise_anim_object_t* obj, uint16_t t);
void ise_set_animation(ise_anim_object_t* obj, uint16_t a, bool restart);
void ise_move_obj(ise_anim_object_t* obj);
uint32_t ise_check_obj_collision(ise_anim_object_t* src, ise_anim_object_t* dst);

void ise_setup_fast_sprite_draw();
void ise_draw_fast_sprite(ise_fast_sprite_t* fsp, ise_framebuffer_t* fb, int x, int y, uint16_t mask_flags);
void ise_shutdown_fast_sprite_draw();


#endif // #ifndef __ISE_GFX_H
