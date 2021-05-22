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
        printf("Failed to open");
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
    // decode and execute
    decodeAndExecute(opcode);
}

void decodeAndExecute(unsigned short opcode) {
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
    unsigned char s = 0x0;
    if(opcode == 0x0) {
        while(1){}
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
        } else if (opcode == 0x00EE) {
            printf("Return from subroutine\n");
            pc = stack[sp - 1];
            sp--;
            if(sp < 0) {
                printf("Stack bound exceeded negative\n");
                exit(EXIT_FAILURE);
            }
        }
        break;
    case 0x1:
        // jump
        printf("Jump to %x\n", addr);
        pc =  addr;
        break;
    case 0x2:
        // call subroutine
        printf("Call %x\n", addr);
        printf("PC was %x\n", pc);
        stack[sp] = pc;
        sp++;
        pc = addr;
        break;
    case 0x3:
        printf("Skip1\n");
        if(v[second] == value) {
            pc += 2;
        }
        break;
    case 0x4:
        printf("Skip2\n");
        if(v[second] != value) {
            pc += 2;
        }
        break;
    case 0x5:
        printf("Skip3\n");
        if(v[second] == v[third]) {
            pc += 2;
        }
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
    case 0x8:
        // arithmetic
        switch(fourth) {
            case 0x0:
                printf("Set\n");
                s = v[third];
                v[second] = s;
                break;
            case 0x1:
                printf("OR\n");
                s = v[second] | v[third];
                v[second] = s;
                break;
            case 0x2:
                printf("AND\n");
                s = v[second] & v[third];
                v[second] = s;
                break;
            case 0x3:
                printf("XOR\n");
                s = v[second] ^ v[third];
                v[second] = s;
                break;
            case 0x4:
                printf("ADD\n");
                s = v[second] + v[third];
                v[second] = s;
                break;
            case 0x5:
                printf("SUB\n");
                printf("%x %x\n", v[second], v[third]);
                s = v[second] - v[third];
                printf("%x\n", s);
                //v[second] = s;
                if(v[second] >= v[third]) {
                    printf("B\n");
                    v[0xF] = 1;
                } else {
                    printf("A\n");
                    v[0xF] = 0;
                }
                v[second] = s;
                //while(1){}
                break;
            case 0x7:
                printf("SUB2\n");
                s = v[third] - v[second];
                if(v[third] >= v[second]) {
                    v[0xF] = 1;
                } else {
                    v[0xF] = 0;
                }
                v[second] = s;
                break;
            case 0x6:
                printf("SHIFT\n");
                // v[second] = v[third];
                s = v[second];
                unsigned char right = s & 0x1;
                v[0xF] = right;
                s = s >> 1;
                v[second] = s;
                break;
            case 0xE:
                printf("SHIFT2\n");
                // v[second] = v[third];
                s = v[second];
                unsigned char left = s & 0x80;
                if(left != 0) {
                    left = 1;
                } 
                v[0xF] = left;
                s = s << 1;
                v[second] = s;
                break;
            default:
                break;
        }
        break;
    case 0x9:
        if(v[second] != v[third]) {
            pc += 2;
        }
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
    case 0xF:
        printf("OS stuff\n");
        switch(value) {
            // timers
            case 0x7:
                ;
                v[second] = delay_timer;
                break;
            case 0x15:
                ;
                delay_timer = v[second];
                break;
            case 0x18:
                ;
                sound_timer = v[second];
                break;
            // add to I
            case 0x1E:
                s = ir;
                if((s + v[second]) > 0x1000) {
                    v[0xF] = 1;
                }
                s += v[second];
                ir = s;
                break;
            // get key
            case 0x0A:
                break;
            // Font character
            case 0x29:
                printf("Font\n");
                unsigned char c = v[second];
                ir = 0x50 + (5 * c);
                break;
            // Binary coded decimal conversion
            case 0x33:
                ;
                unsigned char num = v[second];
                unsigned char hun = num / 100;
                unsigned char ten = (num % 100) / 10;
                unsigned char ones = num % 10;
                printf("%d %d %d %d\n", num, hun, ten, ones);
                memory[ir] = hun;
                memory[ir + 1] = ten;
                memory[ir + 2] = ones;
                break;
            // Store and load memory
            case 0x55:
                printf("Store\n");
                for(int i = 0; i <= second; i++) {
                    memory[ir + i] = v[i];
                }
                break;
            case 0x65:
                printf("Load\n");
                for(int i = 0; i <= second; i++) {
                    v[i] = memory[ir + i];
                }
                break;
        }
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