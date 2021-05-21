#include<stdio.h>
#include<stdlib.h>
#include <SDL2/SDL.h>


#include "include/chip8.h"

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

const int WIDTH = 64 * 5;
const int HEIGHT = 32 * 5;

SDL_Window *window = NULL;
SDL_Surface *surface = NULL;
SDL_Renderer *renderer = NULL;
unsigned char tempwindow[64*5][32*5];
bool draw = 0;

bool init_graphics(){
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Failed to init SDL with %s\n", SDL_GetError());
        return false;
    } else {
        int res = SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, SDL_WINDOW_SHOWN, &window, &renderer);
        if(res) {
            printf("Failed to create SDL window with %s\n", SDL_GetError());
            return false;
        } else {
            surface = SDL_GetWindowSurface(window);
            SDL_SetWindowTitle(window, "chip8-c");
        }
    }
    return true;
}

void destroy_graphics() {
    SDL_FreeSurface(surface);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char **argv)
{
    if(!init_graphics()){
        exit(EXIT_FAILURE);
    }
    FILE *rom;
    rom = fopen(*(++argv), "rb");
    long size;
    if(rom) {
        fseek(rom, 0, SEEK_END); // seek to end of file
        size = ftell(rom); // get current file pointer
        printf("Size is %ld\n", size);
        rewind(rom);
    } else {
        printf("Failed to open file\n");
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
    bool quit = false;
    while(!quit) {
        SDL_Event e;
        while(SDL_PollEvent(&e) != 0) {
            if(e.type == SDL_QUIT){
                quit = true;
            }
        }
        cycle();
        if(draw) {
            // draw
            for(int i = 0; i < WIDTH; i++) {
                for(int j = 0; j < HEIGHT; j++) {
                    if(tempwindow[i][j] == 0) {
                        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    } else {
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    }
                    SDL_RenderDrawPoint(renderer, i, j);
                }
            }
            SDL_RenderPresent(renderer);
            printf("Drew something\n");
            draw = 0;            
        }
    }
    fclose(rom);
    free(buffer);
    destroy_graphics();
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
    if(opcode == 0x0) {
        exit(EXIT_SUCCESS);
    }
    switch (first)
    {
    case 0x0:
        if(opcode == 0x00e0) {
            printf("Clear the screen\n");
            for(int i = 0; i < 64; i++){
                for(int j = 0; j < 32; j++) {
                    tempwindow[i][j] = 0;
                }
            }
            draw = 1;
            break;
        }
        break;
    case 0x1:
        // jump
        ;
        printf("Jump to %x\n", addr);
        pc =  addr;
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
        unsigned char x = v[second] % 64;
        unsigned char oldx = x;
        unsigned char y = v[third] % 32;
        v[0xF] = 0;
        printf("I is %x\n", ir);
        printf("X is %x, y is %x\n", x, y);
        for(int i = 0; i < fourth; i++) {
            unsigned short data = memory[ir + i];
            printf("\tData is %x, loop %x\n", data, i);
            printf("\tX is %x, y is %x\n", x, y);
            for(int j = 0; j < 8; j++) {
                if((data & (0x80 >> j)) != 0) {   
                    if(tempwindow[x][y] == 0) {
                        tempwindow[x*5][y*5] = 1;
                    } else {
                        tempwindow[x*5][y*5] = 0;
                        v[0xF] = 1;
                    }
                }
                x++;
            }
            y++;
            x = oldx;
            //debug_print_window();
        }
        draw = true;
        break;
    default:
        printf("TBI\n");
        while(1){}
        break;
    }
}

void debug_print_window(){
    for(int j = 0; j < 32; j++) {
        for(int i = 0; i < 64; i++) {
            printf("%d", tempwindow[i][j]);
        }
        printf("\n");
    }
}