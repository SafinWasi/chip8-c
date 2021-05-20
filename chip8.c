#include<stdio.h>
#include<stdlib.h>

#include "include/chip8.h"
#include <SDL2/SDL.h>

unsigned char memory[4096] = {0};

unsigned short opcode = 0;

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

int main(int argc, char **argv)
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
        unsigned char c = buffer[i];
        memory[i + 0x200] = c;
        printf("0x%x ", memory[i + 0x200]);
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
    opcode = memory[pc] << 8 | memory[pc + 1];
    pc += 2;
    // decode
    decode(opcode);
    // execute
}

void decode(unsigned short opcode) {
    printf("\nOpcode: %x\n", opcode);

    int mask = 0xF000;
    int mask2 = 0x0F00;
    int mask3 = 0x00F0;
    int mask4 = 0x0000F;
    unsigned short first = (opcode & mask) >> 12;
    unsigned short second = (opcode & mask2) >> 8;
    unsigned short third = (opcode & mask3) >> 4;
    unsigned short fourth = opcode & mask4;
    printf("Sections are %x %x %x %x\n", first, second, third, fourth);
    unsigned short addr = (second << 8) | (third << 4) | fourth;
    unsigned short value = (third << 4) | fourth;

    switch (first)
    {
    case 0x0:
        if(opcode == 0x00e0) {
            printf("Clear the screen\n");
            break;
        }
        break;
    case 0x1:
        // jump
        ;
        printf("Jump to %x\n", addr);
        pc = 0x200 + addr;
        break;
    case 0x6:
        // set register
        printf("Set register %x to %x\n", second, value);
        v[second] = value;
        break;
    case 0x7:
        // add to register
        printf("Add to register %x: %x\n", second, value);
        v[second] += value;
        break;
    case 0xA:
        // set index register
        printf("Set index register to %x\n", addr);
        ir = addr;
        break;
    case 0xD:
        // Draw on screen
        printf("Draw %x %x %x\n", second, third, fourth);
        break;
    default:
        printf("TBI\n");
        break;
    }
}