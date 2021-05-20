#include<stdio.h>
#include<stdlib.h>

#include "chip8.h"

unsigned char memory[4096] = {0};

unsigned short pc = 0x200;

unsigned short ir = 0x0;

unsigned char sound_timer = 0;

unsigned char delay_timer = 0;

unsigned char v[16] = {0};

unsigned char font[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

unsigned short stack[16] = {0};

unsigned char sp = 0;

int main()
{
    FILE *rom;
    rom = fopen("Logo.ch8", "rb");
    long size;
    if(rom) {
        fseek(rom, 0, SEEK_END); // seek to end of file
        size = ftell(rom); // get current file pointer
        printf("Size is %d\n", size);
        rewind(rom);
    } else {
        exit(EXIT_FAILURE);
    }
    char *buffer = malloc(size);
    if(buffer == NULL){
        exit(EXIT_FAILURE);
    }
    size_t bytes = fread(buffer, 1, size, rom);
    if(bytes != size) {
        free(buffer);
        exit(EXIT_FAILURE);
    }
    // loading start
    for(int i = 0; i < size; i++){
        memory[i + 0x200] = buffer[i];
        printf("0x%x ", buffer[i]);
    }
    printf("\n");

    for(int i = 0; i < 80; i++){
        memory[i + 0x50] = font[i];
    }

    // main loop
    while(pc < size + 0x200) {
        // run emulation cycle
        cycle();
    }

    fclose(rom);
    free(buffer);
    return 0;
}

void cycle() {
    // fetch
    short opcode = 0x0;
    opcode = memory[pc];
    opcode = opcode << 8;
    opcode = opcode | memory[pc + 1];
    printf("0x%x\n", opcode);
    pc += 2;
    // decode
    decode(opcode);
    // execute
}

void decode(short opcode) {
    int mask = 0xF000;
    int mask2 = 0x0F00;
    int mask3 = 0x00F0;
    int mask4 = 0x000F;
    short first = opcode & mask;
    short second = opcode & mask2;
    short third = opcode & mask3;
    short fourth = opcode & mask4;
    printf("First is 0x%x\n", first);
    printf("First is 0x%x\n", third);

    switch (first)
    {
    case 0x0:
        if(opcode == 0x00e0) {
            printf("Clear the screen\n");
            break;
        }
        break;
    default:
        break;
    }
}