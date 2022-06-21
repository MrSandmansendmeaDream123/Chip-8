#include <iostream>
#include <stack>
#include <cstdint>
#include <iomanip>
#include "SDL2/SDL.h"
#include <stdio.h>
#include <fstream>

#define NIBBLE(instr) (instr & 0xf000)
#define X(instr) (instr & 0x0f00)
#define Y(instr) (instr & 0x00f0)
#define N(instr) (instr & 0x000f)
#define NN(instr) (instr & 0x0ff0)
#define NNN(instr) (instr & 0x0fff)
#define KK(instr) (instr & 0x00ff)

#define KILOBYTE 1024
#define MEMORYSIZE KILOBYTE*4

#define SCREENWIDTH 64
#define SCREENHEIGHT 32
#define STARTINDEX 512

using namespace std;

int main(int argc, char* args[]){




    //RENDERING
    SDL_Window *window;
    SDL_Renderer* renderer;

    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(
        "Chip-8",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREENWIDTH,
        SCREENHEIGHT,
        SDL_WINDOW_OPENGL
    );

    if(window == NULL){
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    //Rendering Blank Screen
    renderer = SDL_CreateRenderer(window,-1,0);
    SDL_SetRenderDrawColor(renderer,0,0,0,0);
    SDL_RenderClear(renderer);
    
      
    //Display
    bool display [64][32] {};

    //RAM

    /*  Direct Access to up to 4kilobytes of RAM
        0x000 (0) -> 0x1FF (511) Reserved For Interpreter (Not to be used by programs)
        Chip-8 programs start at 0x200 (512) -> 0xFFF (4095)
        Some begin at 0x600 (1536), those starting in the latter are for ETI 660 computers
    */

    //8bit
    unsigned short int memory [MEMORYSIZE]; 
    //Program Counter 16-bit pointer
    unsigned short int *pc = &memory[513]; // same as ptr = &memory[0]

    /* Adding the font. There’s a special instruction for setting I to a character’s address, 
    so you can choose where to put it.But 050-09F is the standard for...some reason....welp why not lol */
    unsigned char fonts [] = 
    {0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70,  //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  //3
    0x90, 0x90, 0xF0, 0x10, 0x10,  //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  //6
    0xF0, 0x10, 0x20, 0x40, 0x40,  //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  //9
    0xF0, 0x90, 0xF0, 0x90, 0x90,  //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  //B
    0xF0, 0x80, 0x80, 0x80, 0xF0,  //C
    0xE0, 0x90, 0x90, 0x90, 0xE0,  //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  //E
    0xF0, 0x80, 0xF0, 0x80, 0x80}; //F

    //Adding fonts from 0x050-0x09F (80-159)
    int start = 80;
    for(unsigned char x: fonts){
        memory[start] = x;
        //cout<<start<<endl;
        start++;
    }


    //REGISTERS

    //Stack 16-bit
    //uses it to call and return from subroutines (“functions”): 16-bit addresses
    stack<unsigned short int> mystack;


   
    //Index Register 16-bit
    //Used to store memory addresses, so only the lowest(rightmost) 12 bits are used
    unsigned short int I [1]; 

    //16 General Purpose Variable Registers V[0] - V[15] 8-bit
    //V[15] should not be used by any programs as it is a flag for some instructions
    unsigned char V[16];

    //Sound Registers 8-bit, will require threading...probably

    //This is independent of the speed of the fetch/decode/execute loop 

    //When non zero value,decremented at a rate of 60Hz
    //The delay timer is active whenever the delay timer register (DT) is non-zero.
    // This timer does nothing more than subtract 1 from the value of DT at a rate of 60Hz. When DT reaches 0, it deactivates.
    unsigned char delay[] = {0};
    //The sound timer is active whenever the sound timer register (ST) is non-zero. 
    //This timer also decrements at a rate of 60Hz, however, as long as ST's value is greater than zero, 
    //the Chip-8 buzzer will sound. When ST reaches zero, the sound timer deactivates.
    unsigned char sound[] = {0};


    //LOADROM
    //However, in order to manipulate binary files, you should 
    //always specify the i/o mode, including ios::binary as one of the mode flags. For read/write access to a file, use an fstream
    /*
    ifstream is input file stream which allows you to read the contents of a file.
    ofstream is output file stream which allows you to write contents to a file.
    fstream allows both reading from and writing to files by default.
    However, you can have an fstream behave like an ifstream or ofstream by passing in the ios::open_mode flag.
    flags are additive which means you can combine multiple flags using the bitwise OR | operator
    */  
    if(argc != 2){
        cerr << "<romfile>"<<endl;
        exit(1);
    }

    fstream rom;
    rom.open(args[1], ios::in | ios::out | ios::binary);
    //0x200 (512)
    int size;
    char * buffer;
    if(rom.is_open()){
        //Get the size of the file
        //move the pos to the end, can be done by default in opening flag with ios::ate
        rom.seekg(0,rom.end);
        //size is the total at the end
        size = rom.tellg();
        //move pos back to the start
        rom.seekg(0,rom.beg);
        buffer = new char [size];
        rom.read(buffer,size);
        rom.close();
        if(!rom){//same as if(!rom.read (memory,100))
            //Error occured, rom.gcount() will give the num of bytes read and calling rom.clear() will reset stream state
            //making it usable again
            cerr<< "Error occured while reading rom: " << args[1] <<endl;
        }

    }else{
        cerr<< "Failed to open file" <<endl;
        exit(1);
    }
    //cout.write(buffer,size);
    for(int i = 0; i < size; i++){
        memory[STARTINDEX + i] = buffer[i];
        //cout<<memory[STARTINDEX + i]<<endl;
    }
    // //cout.write(buffer,2);
    // for (int x = 0; x < 4096; x++){
    //     cout<<x<<" : " << memory[x]<<endl;
    // }

    delete [] buffer;
    cout<<memory[513]<<endl;

    //MAIN LOOP
    while(1){
        //1.Fetch
        //Read instucttions that PC is pointing too
        unsigned short int instruction = *pc;
        //increment the pc
        pc++;

        //2.Decode
        //Masking with macros!
        unsigned short int nibble = NIBBLE(instruction);
        unsigned short int x = X(instruction) ;
        unsigned short int y = Y(instruction) ;
        unsigned short int n = N(instruction) ;
        unsigned short int nn = NN(instruction) ;
        unsigned short int nnn = NNN(instruction) ;
        unsigned short int kk = KK(instruction) ;

    /*
        00E0 (clear screen)
        1NNN (jump)
        6XNN (set register VX)
        7XNN (add value to register VX)
        ANNN (set index register I)
        DXYN (display/draw)
    */
       /* cout<<hex << nibble<< " vs" <<"0x"<<0x1000<<endl;*/

        unsigned char xcoordinate;
        unsigned char ycoordinate;
        unsigned short int ivalue;

        unsigned short int nthByte;
        switch (nibble)
        {
        case 0x0000:

            switch(kk){
            //CLS -> Clear the display
            case 0xE0:
                //display
                SDL_RenderClear(renderer);
                cout<<"clear screen"<<endl;
                break;
            case 0xEE:
                //The interpreter sets the program counter to the address at the top of the stack,
                //then subtracts 1 from the stack pointer.
                pc = &mystack.top();
                mystack.pop();
                cout<<"Return from a subroutuine"<<endl;
                break;
            default:
                cerr << "Invalid Instruction: "<<instruction<<endl;
                exit(1);
                break;
            }
            break;
        case 0x1000:
            //JP addr -> Jump to location nnn.
            //The interpreter sets the program counter to nnn.
            *pc = memory[nnn];
            cout<<"jumped"<<endl;
            break;
        case 0x2000:
            //CALL addr -> Call subroutine at nnn The interpreter increments the stack pointer,
            //then puts the current PC on the top of the stack. The PC is then set to nnn.
            mystack.push(*pc);
            *pc = memory[nnn];
            std::cout<<"called"<<std::endl;
            break;
        case 0x3000:
            break;
        case 0x4000:
            break;
        case 0x5000:
            break;
        case 0x6000:
            V[x] = kk;
            cout<<"set register VX"<<endl;
            break;
        case 0x7000:
            //ADD Vx, byte
            V[x] = V[x] + kk;
            cout<<"add value to register VX"<<endl;
            break;
        case 0x8000:
            break;
        case 0x9000:
            break;
        case 0xA000:
            I[0] = nnn;
            cout<<"set index register I"<<endl;
            break;
        case 0xB000:
            break;
        case 0xC000: 
            break; 
        case 0xD000:
            //Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = 0.
            xcoordinate = V[x] % 64;
            ycoordinate = V[y] % 32;
            V[15] = 0;

            //Read n bytes from memory
            //for each byte
            for(int i = 0; i < n;i++){
                ivalue = I[0];
                nthByte = memory[ivalue + i]; 
                for(int j = 0; j<8;j++){
                    /*If the current pixel in the sprite row is on and the pixel at coordinates X,Y 
                    on the screen is also on, turn off the pixel and set VF to 1*/
                    if(nthByte & (1<<j)){
                        if(display[xcoordinate][ycoordinate] == 1){
                            display[xcoordinate][ycoordinate] = 0;
                            V[15] = 1;
                        }else {
                            //Current bit is on and display not drawn (display[xcoordinate][ycoordinate] == 0)
                            display[xcoordinate][ycoordinate] = 1;
                            SDL_SetRenderDrawColor( renderer, 0xFF, 0x00, 0x00, 0xFF ); 
                            SDL_RenderDrawPoint(renderer,xcoordinate,ycoordinate);          
                            SDL_RenderPresent( renderer );
                    }
    
                    }
                    //If you reach the right edge of the screen, stop drawing this row
                    if(xcoordinate >= SCREENWIDTH){
                        break;
                    }
                    //Increment X (VX is not incremented)
                    xcoordinate++;
                }
                //Increment Y (VY is not incremented)
                ycoordinate++;
                //SDL_RenderPresent( renderer );
                //Stop if you reach the bottom edge of the screen
                if(ycoordinate >= SCREENHEIGHT){
                    break;
                }
                //SDL_RenderDrawPoints(renderer,)

             
 
            }
               for (int i = 0; i < 64; i++) {
                    for (int j = 0; j < 32; j++) {
                        std::cout << display[i][j] << ' ';
                    }
                std::cout << std::endl;
                }



            
            cout<<"display/draw"<<endl;
            break;
        case 0xE000:
            break;
        case 0xF000:
            break;
        
        default:
            cout<<"Instruction Not Found"<<endl;
            break;
        }

        SDL_Delay(3000); // 8 secs
        SDL_DestroyWindow(window);
        SDL_Quit();
        break;
        
    }
    return 0;
}

