#include <iostream>
#include <stack>
#include <stdio.h>
#include <stdlib.h>
#include "SDL2/SDL.h"
#include <bitset>
#include <unordered_map>

#ifndef _chip_hpp_
#define _chip_hpp_
#define SCREENWIDTH 64
#define SCREENHEIGHT 32
#define STARTINDEX 512
#define FONTSTART 80


#define KILOBYTE 1024
#define MEMORYSIZE KILOBYTE*4




class Chip8{
     public:
        void start();
        void loadRom(std::string rom);
        void setUp(SDL_Renderer*& render);
        void draw(int x,int y);
        void setKeys();  


        SDL_Event event;
        const Uint8* currentKeyStates;
    private:
        /*  Direct Access to up to 4kilobytes of RAM
        0x000 (0) -> 0x1FF (511) Reserved For Interpreter (Not to be used by programs)
        Chip-8 programs start at 0x200 (512) -> 0xFFF (4095)
        Some begin at 0x600 (1536), those starting in the latter are for ETI 660 computers
        */
        unsigned char memory [MEMORYSIZE]; 

        std::unordered_map<SDL_Scancode,int> keyPad;

        bool display [64][32] {};//pixels total of 2048 or 64 * 32
        //std::bitset<SCREENWIDTH*SCREENHEIGHT>;
        bool keyPressed;
        int scanCode;

        SDL_Renderer* Chiprenderer;
        
        unsigned short pc;//Program Counter 16-bit pointer

        bool flag;

        std::stack<unsigned short> mystack;//Stack 16-bit

        
        unsigned short I; //Index Register 16-bit

        
        unsigned char V[16];//16 General Purpose Variable Registers V[0] - V[15] 8-bit

        
        unsigned char delay;//Sound Registers 8-bit

         
        unsigned char sound;//The sound timer is active whenever the sound timer register (ST) is non-zero.

        
        unsigned char keys[16];//Current state of keypad
};

#endif