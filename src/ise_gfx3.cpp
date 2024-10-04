// InfiniScroll Engine
// ise_gfx3.cpp
// 3D Graphics routines
//
// Kamal Pillai
// 6/20/2020

#include "ise.h"

static const ise_opcode_t ise_opcode_table[ISE_NUM_OPCODES] = {
	{ "NOP", 0, 0 },
	{ "MOV", 1, 1 },
	{ "ADD", 1, 2 },
	{ "SUB", 1, 2 },
	{ "MAD", 1, 3 },
	{ "MUL", 1, 2 },
	{ "RCP", 1, 1 },
	{ "RSQ", 1, 1 },
	{ "DP3", 1, 2 },
	{ "DP4", 1, 2 },
	{ "MIN", 1, 2 },
	{ "MAX", 1, 2 },
	{ "SLT", 1, 2 },
	{ "SGE", 1, 2 },
	{ "EXP", 1, 1 },
	{ "LOG", 1, 1 },
	{ "LIT", 1, 1 },
	{ "DST", 1, 2 },
	{ "LRP", 1, 3 },
	{ "FRC", 1, 1 },
	{ "M4X4", 1, 2 },
	{ "M4X3", 1, 2 },
	{ "M3X4", 1, 2 },
	{ "M3X3", 1, 2 },
	{ "M3X2", 1, 2 },
	{ "CALL", 0, 1 },
	{ "CALLNZ", 0, 2 },
	{ "LOOP", 0, 2 },
	{ "RET", 0, 0 },
	{ "ENDLOOP", 0, 0 },
	{ "LABEL", 0, 1 },
	{ "DCL", 1, 1 },
	{ "POW", 1, 2 },
	{ "CRS", 1, 2 },
	{ "SGN", 1, 3 },
	{ "ABS", 1, 1 },
	{ "NRM", 1, 1 },
	{ "SINCOS", 1, 3 },
	{ "REP", 0, 1 },
	{ "ENDREP", 0, 0 },
	{ "IF", 0, 1 },
	{ "IFC", 0, 2 },
	{ "ELSE", 0, 0 },
	{ "ENDIF", 0, 0 },
	{ "BREAK", 0, 0 },
	{ "BREAKC", 0, 2 },
	{ "MOVA", 1, 1 },
	{ "DEFB", 1, 1 },
	{ "DEFI", 1, 4 }
};


int ise_parse_shader_token(FILE* fp, uint32_t* instruction_slot)
{
	uint32_t size = 0;
	uint32_t token, token_size, opcode;
	while(!feof(fp)) {
		fread(&token, 1, sizeof(uint32_t), fp);
		printf("token: 0x%x\n", token);
		opcode = token & 0xFFFF;
		switch(opcode) {
		case 0xFFFF:  // end
			return size;
		case 0xFFFE:  // comment
			token_size = token >> 16;
			fseek(fp, token_size*sizeof(uint32_t), SEEK_CUR);
			break;
		case 0xFFFD:  // phase token
			break;
		default:
			token_size = (token >> 24) & 0xF;
			if(token_size == 0 && opcode < ISE_NUM_OPCODES) {
				token_size = ise_opcode_table[opcode].num_dst + ise_opcode_table[opcode].num_src;
			}
			if(instruction_slot) {
				*instruction_slot = token;
				fread(instruction_slot+1, token_size, sizeof(uint32_t), fp);
				instruction_slot += token_size + 1;
			} else {
				fseek(fp, token_size*sizeof(uint32_t), SEEK_CUR);
			}
			size += token_size + 1;
			break;
		}
	}
	// we should get an end of program token, but if not, return at end of file
	return size;
}

void ise_load_shader_from_file(const char* filename, ise_shader_t* shader)
{
	printf("loading shader %s\n", filename);
	if(filename == NULL || shader == NULL) {
		printf("invalid args\n");
		return;
	}

	shader->instructions = NULL;

	FILE* fp = fopen( filename, "rb" );
    if( fp == NULL ) {
		printf("shader file not found: %s\n", filename);
		return;
	}

	fread(shader, 1, sizeof(uint32_t), fp);
	if(shader->shader_type != ISE_SHTYPE_VERTEX && shader->shader_type != ISE_SHTYPE_PIXEL) {
		printf("unknown shader type: 0x%x\n", shader->shader_type);
		fclose(fp);
		return;
	}
	
	shader->num_instructions = ise_parse_shader_token(fp, NULL);
	if(shader->num_instructions) {
		shader->instructions = (uint32_t*) malloc(shader->num_instructions * sizeof(uint32_t));
		fseek(fp, 4, SEEK_SET);
		ise_parse_shader_token(fp, shader->instructions);
	} else {
		printf("no instructions in shader\n");
	}
	fclose(fp);
}

void ise_free_shader(ise_shader_t* shader)
{
	if(shader->instructions) {
		free(shader->instructions);
		shader->instructions = NULL;
	}
}
