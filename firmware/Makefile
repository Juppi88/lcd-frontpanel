TARGET = firmware

# Chip and programmer info
MCU = atmega328p
MCU_PROGRAMMER = m328p
F_CPU = 16000000
PROGRAMMER = usbasp-clone
PROGRAMMER_PORT = usb

CFLAGS = -std=c99 -DF_CPU=$(F_CPU)L -mmcu=$(MCU) -Wall -Os

compile:
	#
	# Compile the firmware.
	avr-gcc $(CFLAGS) -c firmware.c

	#
	# Link the object files and libraries.
	avr-gcc $(CFLAGS) firmware.o -L./avr-lcd1602 -L./avr-serial -lavr-lcd1602 -lavr-serial -o $(TARGET).elf 

	#
	# Copy the linked binary into a .hex file
	avr-objcopy -j .text -j .data -O ihex $(TARGET).elf $(TARGET).hex

asm:
	avr-gcc -S $(CFLAGS) -c firmware.c

flash:
	avrdude -p $(MCU_PROGRAMMER) -P $(PROGRAMMER_PORT) -c $(PROGRAMMER) -U flash:w:$(TARGET).hex

fuse:
	# Set fuse bits for the microcontroller (set for an ATmega328P).
	# NOTE: Expects a 16MHz external crystal oscillator!
	avrdude -p $(MCU_PROGRAMMER) -P $(PROGRAMMER_PORT) -c $(PROGRAMMER) -U lfuse:w:0xff:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m

clean:
	rm -f *.o *.s $(TARGET).obj $(TARGET).elf $(TARGET).hex

all:
	compile
