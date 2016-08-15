#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "protocol.h"

static HANDLE serial;

static void print_error(const char *format, ...)
{
	va_list arg;

	va_start(arg, format);
	vfprintf(stderr, format, arg);
	va_end(arg);
}

static void print_help(void)
{
	printf("\nUsage:\n");
	printf("  --port - Specify the COM port used by the device\n");
	printf("  --clear - Clears the display\n");
	printf("  --cursor <row> <column> - Moves the cursor to a position (0...1, 0...15)\n");
	printf("  --message <message> - Print a message (up to 16 characters)\n");
	printf("  --glyph <index> - Print a special glyph (0...7)\n");
	printf("  --createglyph <index> <data> - Replace a glyph (5x8 bitmap as 8 hex bytes)\n\n");
}

static HANDLE serial_open(const char *port)
{
	// If the connection is already open, return it.
	if (serial != NULL) {
		return serial;
	}

	if (port == NULL) {
		print_error("Device port has not been specified\n", port);
		return NULL;
	}

	// Open a handle for the serial port.
	serial = CreateFile(port, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);

	if (serial == NULL) {
		print_error("Could not open serial port %s for writing\n", port);
		return NULL;
	}

	// Adjust the serial communication parameters for the device. The device uses a 9600 baud rate.
	DCB params = { 0 };
	params.DCBlength = sizeof(params);

	GetCommState(serial, &params);

	params.BaudRate = CBR_9600;
	params.ByteSize = 8;
	params.StopBits = ONESTOPBIT;
	params.Parity = NOPARITY;

	SetCommState(serial, &params);

	return serial;
}

static bool serial_write(HANDLE serial, const void *data, size_t length)
{
	DWORD written;

	if (!WriteFile(serial, data, (DWORD)length, &written, NULL)) {
		if (GetLastError() != ERROR_IO_PENDING) {
			return false;
		}
	}

	return true;
}

int main(int argc, char **argv)
{
	if (argc <= 1) {
		print_help();
		return 0;
	}

	for (int i = 1; i < argc; ++i) {
		
		// Initialize the serial connection.
		if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
			if (serial_open(argv[++i]) == NULL) {
				return 0;
			}
		}

		// Clear the LCD.
		else if (strcmp(argv[i], "--clear") == 0) {
			if (serial_open(NULL) == NULL) {
				return 0;
			}

			uint8_t data = PACKET_CLEAR;
			serial_write(serial, &data, 1);

			Sleep(50);
		}

		// Set cursor position.
		else if (strcmp(argv[i], "--cursor") == 0 && i + 2 < argc) {
			if (serial_open(NULL) == NULL) {
				return 0;
			}

			uint8_t data[3] = { PACKET_SET_CURSOR, (uint8_t)atoi(argv[++i]), (uint8_t)atoi(argv[++i]) };
			serial_write(serial, data, sizeof(data));

			Sleep(50);
		}

		// Print a message to the LCD.
		else if (strcmp(argv[i], "--message") == 0 && i + 1 < argc) {
			if (serial_open(NULL) == NULL) {
				return 0;
			}

			const char *message = argv[++i];
			size_t len = strlen(message);

			if (len >= MAX_TEXT_LEN) {
				len = MAX_TEXT_LEN;
			}

			uint8_t data[2] = { PACKET_PRINT, (uint8_t)len };
			serial_write(serial, data, sizeof(data));
			serial_write(serial, message, len);

			Sleep(50);
		}

		// Print a special glyph.
		else if (strcmp(argv[i], "--glyph") == 0 && i + 1 < argc) {
			if (serial_open(NULL) == NULL) {
				return 0;
			}

			uint8_t data[2] = { PACKET_PRINT_GLYPH, (uint8_t)atoi(argv[++i]) };
			serial_write(serial, data, sizeof(data));

			Sleep(50);
		}

		// Replace an existing special glyph.
		else if (strcmp(argv[i], "--createglyph") == 0 && i + 2 < argc) {
			if (serial_open(NULL) == NULL) {
				return 0;
			}

			uint8_t glyph = (uint8_t)atoi(argv[++i]);

			// Only glyps 0...7 can be modified.
			if (glyph > 7) {
				glyph = 7;
			}

			uint8_t data[] = { PACKET_CREATE_GLYPH, glyph, 0, 0, 0, 0, 0, 0, 0, 0 };
			const char *s = strtok(argv[++i], " ");

			// Read the glyph bitmap data. This is a series of hex bytes.
			for (int i = 2; i < 10; ++i) {
				if (s == NULL) {
					break;
				}

				data[i] = (uint8_t)strtol(s, NULL, 16);
				s = strtok(NULL, " ");
			}

			serial_write(serial, data, sizeof(data));

			Sleep(50);
		}

		// Print the usage message for this program.
		else if (strcmp(argv[i], "--help") == 0) {
			print_help();
		}

	}

	// Close the serial connection before exiting.
	CloseHandle(serial);
	return 0;
}
