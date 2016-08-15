/*
 * Defines the protocol to use when communicating with the device.
 */

#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include <stdint.h>

#ifdef _MSC_VER
#define PACKED
#pragma pack(push, 1)
#else
#define PACKED __attribute__((packed))
#endif

 // --------------------------------------------------------------------------------

// Data packet types
enum {
	PACKET_CLEAR = 1,
	PACKET_SET_CURSOR,
	PACKET_PRINT,
	PACKET_PRINT_GLYPH,
	PACKET_CREATE_GLYPH,
};

// --------------------------------------------------------------------------------

struct packet_set_cursor_t {
	uint8_t row;		// Row to move the cursor to
	uint8_t column;		// Column to move the cursor to
} PACKED;

// --------------------------------------------------------------------------------

#define MAX_TEXT_LEN 16

struct packet_print_t {
	uint8_t length;			// The length of the message
	char text[MAX_TEXT_LEN];// Up to a 16 char message without a null terminator
} PACKED;

// --------------------------------------------------------------------------------

struct packet_print_glyph_t {
	uint8_t index;		// Glyph index
} PACKED;

// --------------------------------------------------------------------------------

struct packet_create_glyph_t {
	uint8_t index;		// Glyph index
	uint8_t glyph[8];	// 5x8 pixel bitmap describing the glyph
} PACKED;

// --------------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif
