#include <avr/io.h>
#include <stdio.h>
#include "avr-lcd1602/lcd1602.h"
#include "avr-serial/serial.h"
#include "protocol.h"

static struct lcd_t lcd;

static void initialize()
{
	// Create an interface to the display and initialize it.
	lcd = lcd_initialize(PC0, PC1, PC2, PC3, PC4, PC5);

	// Create some custom characters just for fun.
	uint8_t glyph1[] = { 0x00, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00 }; // Backslash
	uint8_t glyph2[] = { 0x00, 0x0A, 0x0A, 0x0A, 0x00, 0x11, 0x0E, 0x00 }; // Smiley face
	uint8_t glyph3[] = { 0x00, 0x0A, 0x0A, 0x0A, 0x00, 0x0E, 0x11, 0x00 }; // Sad face

	lcd_create_glyph(&lcd, 0, glyph1);
	lcd_create_glyph(&lcd, 1, glyph2);
	lcd_create_glyph(&lcd, 2, glyph3);

	// Initialize the serial connection.
	serial_initialize();

	// Print some initial text which is displayed on startup.
	lcd_print_glyph(&lcd, ' ');
	lcd_print_glyph(&lcd, 0x00);
	lcd_print_glyph(&lcd, 0x00);
	lcd_print(&lcd, "BUILDSERVER");
}

static void set_cursor(void)
{
	struct packet_set_cursor_t p;
	serial_read_data(&p, sizeof(p));

	lcd_set_cursor(&lcd, p.row, p.column);
}

static void print(void)
{
	char buffer[MAX_TEXT_LEN + 1];

	// Get the length of the message.
	uint8_t len = (uint8_t)serial_read();

	// Make sure the data fits into the buffer.
	if (len > MAX_TEXT_LEN) {
		len = MAX_TEXT_LEN;
	}

	// Read the actual message and null terminate it.
	serial_read_data(buffer, len);
	buffer[len] = 0;

	// Print it.
	lcd_print(&lcd, buffer);
}

static void print_glyph(void)
{
	struct packet_print_glyph_t p;
	serial_read_data(&p, sizeof(p));

	lcd_print_glyph(&lcd, p.index);
}

static void create_glyph(void)
{
	struct packet_create_glyph_t p;
	serial_read_data(&p, sizeof(p));

	lcd_create_glyph(&lcd, p.index, p.glyph);
}

int main(void)
{
	char errmsg[32];

	initialize();

	for (;;) {

		if (serial_is_data_available()) {

			char packet = serial_read();

			switch (packet) {

			case PACKET_CLEAR:
				lcd_clear(&lcd);
				break;

			case PACKET_SET_CURSOR:
				set_cursor();
				break;

			case PACKET_PRINT:
				print();
				break;

			case PACKET_PRINT_GLYPH:
				print_glyph();
				break;

			case PACKET_CREATE_GLYPH:
				create_glyph();
				break;

			case 0:
				break;

			default:
				sprintf(errmsg, "READ ERROR: %d", packet);

				lcd_clear(&lcd);
				lcd_print(&lcd, errmsg);

				break;
			}
		}
	}

	return 0;
}
