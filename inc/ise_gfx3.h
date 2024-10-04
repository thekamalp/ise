// InfiniScroll Engine
// ise_gfx.h
// 3D Graphics routines
//
// Kamal Pillai
// 6/11/2020

#ifndef __ISE_GFX3_H
#define __ISE_GFX3_H

#define ISE_PRIM_POINTS                  1
#define ISE_PRIM_LINES                   2
#define ISE_PRIM_LINE_STRIP              3
#define ISE_PRIM_TRIANGLES               4
#define ISE_PRIM_TRIANGLE_STRIP          5

#define ISE_SEMANTIC_POSITION            1
#define ISE_SEMANTIC_COLOR               2
#define ISE_SEMANTIC_TEXCOORD            3

#define ISE_SHTYPE_VERTEX                0xFFFE
#define ISE_SHTYPE_PIXEL                 0xFFFF

#define ISE_MAX_PARAMS                   32

typedef struct {
	uint8_t type;
	uint8_t index;
	uint16_t offset;
} ise_semantic_t;

typedef struct {
	uint8_t num_params;
	uint8_t prim_type;
	uint16_t size;
	ise_semantic_t param[ISE_MAX_PARAMS];
} ise_vertex_format_t;

typedef struct {
	uint8_t version_minor;
	uint8_t version_major;
	uint16_t shader_type;
	uint32_t num_instructions;
	uint32_t* instructions;
} ise_shader_t;

typedef struct {
	const char* opcode_name;
	uint8_t num_dst;
	uint8_t num_src;
} ise_opcode_t;

#define ISE_NUM_OPCODES 64

void ise_load_shader_from_file(const char* filename, ise_shader_t* shader);
void ise_free_shader(ise_shader_t* shader);

#endif  //  __ISE_GFX3_H
