#include "chip8.hpp"
#define NIBBLE(instr) (instr & 0xf000)
#define X(instr) (instr & 0x0f00) >>8
#define Y(instr) (instr & 0x00f0) >> 4
#define N(instr) (instr & 0x000f) 
#define NN(instr) (instr & 0x0ff0) 
#define NNN(instr) (instr & 0x0fff) 
#define KK(instr) (instr & 0x00ff) 
#include "SDL2/SDL.h"
#include <algorithm>

#include <fstream>
using namespace std;

void Chip8::start(){
        //1.Fetch

        //Read the instruction that PC is currently pointing at from memory. An instruction is two bytes,
        // so you will need to read two successive bytes from memory and combine them into one 16-bit instruction.
        //cout<<"pc before "<<pc<<endl;
        unsigned short instruction = (memory[pc] << 8) | memory[pc + 1];
        //cout<<"opcode: "<<hex <<"mem at pc "<< memory[pc]<<" mem at +1 "<<memory[pc+1]<<" opcode "<<instruction <<endl;
        //cout<<"memory 0x228 should be "<<memory[552]<<endl; 
        pc += 2;
     
        
        //cout<<"pc after "<<pc<<endl;
        //2.Decode

        /*All instructions are 2 bytes long and are stored most-significant-byte first. In memory,
         the first byte of each instruction should be located at an even addresses. If a program
          includes sprite data, it should be padded so any instructions following it will be properly 
          situated in RAM.*/

        //Masking with macros!
        unsigned short  nibble = NIBBLE(instruction);
        unsigned short  x = X(instruction);
        unsigned short  y = Y(instruction);
        unsigned short  n = N(instruction);
        unsigned short  nn = NN(instruction);
        unsigned short  nnn = NNN(instruction);
        unsigned short  kk = KK(instruction);

        unsigned char xcoordinate;
        unsigned char ycoordinate;
        unsigned short ivalue;

        unsigned short  nthByte;

        int temp;
        int color = 0;

        switch (nibble)
        {
        case 0x0000:

            switch(kk){
            //CLS -> Clear the display
            case 0xE0:
                //display
                //SDL_RenderClear(renderer);
               for (int i = 0; i < 64; i++) {
                    for (int j = 0; j < 32; j++) {
                        display[i][j] = 0;
                    }
                }
                cout<<"CLS"<<endl;
                break;
            case 0xEE:
                //The interpreter sets the program counter to the address at the top of the stack,
                //then subtracts 1 from the stack pointer.
                pc = mystack.top();
                mystack.pop();
                cout<<"RET"<<endl;
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
            pc = nnn;
            cout<<" JP addr"<<endl;
            break;
        case 0x2000:
            //CALL addr -> Call subroutine at nnn The interpreter increments the stack pointer,
            //then puts the current PC on the top of the stack. The PC is then set to nnn.
            mystack.push(pc);
            pc = nnn;
            std::cout<<"CALL addr"<<std::endl;
            break;
        case 0x3000:
            if(V[x] == kk){
                pc += 2;
            }
            cout<<"SE Vx, byte"<<endl;
            break;
        case 0x4000:
            if(V[x] != kk){
                pc += 2;
            }
            cout<<"SE Vx, byte"<<endl;
            break;
        case 0x5000:
            if(V[x] == V[y]){
                pc += 2;
            }
            cout<<"SE Vx, byte"<<endl;
            break;
        case 0x6000:
            V[x] = kk;
            cout<<"LD Vx, byte "<< instruction << endl;
            break;
        case 0x7000:
            V[x] = V[x] + kk;
            cout<<"ADD Vx, byte"<<endl;
            break;
        case 0x8000:
            switch(n){
                case 0x0:
                    V[x] = V[y];
                    cout<<"LD Vx, Vy"<<endl;
                break;
                case 0x1:
                    V[x] = V[x] | V[y];
                    cout<<"OR Vx, Vy"<<endl;
                break;
                case 0x2:
                    V[x] = V[x] & V[y];
                    cout<<"AND Vx, Vy"<<endl;
                break;
                case 0x3:
                    V[x] = V[x] ^ V[y];
                    cout<<"XOR Vx, Vy"<<endl;
                break;
                case 0x4:
                    V[x] = V[x] + V[y];
                    V[0xF] = (V[x] > 255) ? 1:0;
                    cout<<"ADD Vx, Vy"<<endl;
                break;
                case 0x5:
                    V[0xF] = (V[x] > V[y]) ? 1:0;
                    V[x] = V[x] - V[y];
                    cout<<"SUB Vx, Vy"<<endl;
                break;
                case 0x6:                  
                    if((0x1 & V[x]) == 0x1){
                        V[0xF] = 1;
                    }else{
                        V[0xF] = 0;
                    }
                    V[x] = V[x] >> 1;
                    cout<<"SHR Vx {, Vy}"<<endl;
                break;
                case 0x7:
                    V[0xF] = (V[y] > V[x]) ? 1:0;
                    V[x] = V[y] - V[x];
                    cout<<"SUBN Vx, Vy"<<endl;
                break;
                case 0xE:
                    if((char)V[x] < 0 == 0x1){
                        V[0xF] = 1;
                    }else{
                        V[0xF] = 0;
                    }
                    V[x] = V[x] << 1;
                    cout<<"SHL Vx {, Vy}"<<endl;

                break;
                default:
                cerr << "Invalid Instruction: "<<hex<<instruction<<endl;
                exit(1);
                break;
            }
            break;
        case 0x9000:
            if(V[x] != V[y]){
                pc += 2;
            } 
            cout<<"SNE Vx, Vy"<<endl;
            break;
        case 0xA000:
            I = nnn;
            cout<<"LD I, addr"<<endl;
            break;
        case 0xB000:
            pc = nnn + V[0];
            cout<<"JP V0, addr"<<endl;
            break;
        case 0xC000: 
            V[x] = (rand() % 256) & kk; // 0-255
            cout<<"RND Vx, byte"<<endl;
            break; 
        case 0xD000:
            //Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = 0.
            V[0xF] = 0;
            //Read n bytes from memory
            //for each byte
            for(int nth = 0; nth < n;++nth){
                ycoordinate = (V[y] % SCREENHEIGHT) + nth;
                nthByte = memory[I + nth]; 
                for(int pixel = 0; pixel < 8;++pixel){
                    xcoordinate = (V[x] % SCREENWIDTH) + pixel;
                    /*If the current pixel in the sprite row is on and the pixel at coordinates X,Y 
                    on the screen is also on, turn off the pixel and set VF to 1*/
                    bool currentPixel = (nthByte >> (7-pixel)) & 1;
                    if(currentPixel && display[xcoordinate][ycoordinate]){
                        display[xcoordinate][ycoordinate] = 0;
                        SDL_SetRenderDrawColor( Chiprenderer, 0, 0, 0, 255);
                        SDL_RenderDrawPoint(Chiprenderer,xcoordinate,ycoordinate);
                        V[15] = 1;
                    }else if(currentPixel && display[xcoordinate][ycoordinate] == 0){
                        //Current bit is on and display not drawn (display[xcoordinate][ycoordinate] == 0)
                        display[xcoordinate][ycoordinate] = 1;
                        SDL_SetRenderDrawColor( Chiprenderer, 255, 255, 255, 255 ); 
                        SDL_RenderDrawPoint(Chiprenderer,xcoordinate,ycoordinate);                  
                    }

                    SDL_RenderPresent(Chiprenderer);
                    if(xcoordinate >= SCREENWIDTH){
                        break;
                    }
                        
                }
                        //If you reach the right edge of the screen, stop drawing this row
                if(ycoordinate >= SCREENHEIGHT + 1){
                    break;
                }
                    
            }
           
            cout<<"display/draw"<<endl;
            break;
        case 0xE000:
            switch(kk){
                case 0x9E:
                    currentKeyStates = SDL_GetKeyboardState( NULL );
                    if(currentKeyStates[SDL_GetScancodeFromKey(V[x])]){//if key in Vx is pressed skip next inst
                        pc+=2;
                    }
                    cout<<"SKP Vx"<<endl;
                    break;
                case 0xA1:
                    currentKeyStates = SDL_GetKeyboardState( NULL );
                    if(currentKeyStates[SDL_GetScancodeFromKey(V[x])] == 0){//if Vx not pressed skip
                        pc+=2;
                    }  
                    cout<<"SKNP Vx"<<endl;
                    break;
            }
            break;
        case 0xF000:
            switch(kk){
                case 0x07:
                    V[x] = delay;
                    cout<<"LD Vx, DT"<<endl;
                    break;
                case 0x0A:
                while(flag){
                    while( SDL_PollEvent(&event) )
                    {  
                    switch(event.type)
                        {
                        case SDL_KEYDOWN:
                            auto c = find(begin(keys),end(keys),SDL_GetScancodeFromKey(event.key.keysym.sym));
                            if(c != end(keys)){
                                V[x] = SDL_GetScancodeFromKey(event.key.keysym.sym);
                                flag = false;
                                break;
                            }
                        }
                    }
                }
                    flag = true;
                    cout<<"LD Vx, K"<<endl;
                    break;
                case 0x15:
                    delay = V[x];
                    cout<<"LD DT, Vx"<<endl;
                    break;
                case 0x18:
                    sound = V[x];
                    cout<<"LD ST, Vx"<<endl;
                    break;
                case 0x1E:
                    I = I + V[x];
                    cout<<"ADD I, Vx"<<endl;
                    break;
                case 0x29:
                    I = FONTSTART + (V[x] * 5);
                    cout<<"LD F, Vx"<<endl;
                    break;
                case 0x33:
                    temp = V[x];
                    memory[I + 2] = temp %10;
                    temp = temp/10;
                    memory [I+1] = temp %10;
                    temp = temp/10;
                    memory [I] = temp%10;
                    cout<<"LD B, Vx"<<endl;
                    break;
                case 0x55:
                    for(int index = 0; index <= x; index++){
                        memory[I + index] = V[index];
                    }
                    cout<<"LD [I], Vx"<<endl;
                    break;
                case 0x65:
                    for(int index = 0; index <= x; index++){
                        V[index] = memory[I + index];
                    }
                    cout<<"LD Vx, [I]"<<endl;
                    break;
                default:
                    cerr<<instruction<<" is Invalid"<<endl;
                    exit(1);
                    break;
            }
            break;
        
        default:
            cout<<"Instruction"<<hex<< instruction <<"Not Found"<<endl;
            break;
        }

        if(delay > 0){
            --delay;
        }
        if(sound > 0){
            if(sound == 1){
                //Beep
                cout<<"Beep"<<endl;
                --sound;
            }
        }

}

void Chip8::loadRom(string romFile){

    //Attemping to open the rom
    fstream rom;
    rom.open(romFile, ios::in | ios::out | ios::binary);
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
            cerr<< "Error occured while reading rom: " << romFile <<endl;
        }

    }else{
        cerr<< "Failed to open file: " <<romFile<<endl;
        exit(1);
    }
  
    //Load the rom into memory
    for(int i = 0; i < size; i++){
        memory[STARTINDEX + i] = buffer[i];
    }

    delete [] buffer;
}

void Chip8::setUp(SDL_Renderer*& render){

    /* Adding the font. There’s a special instruction for setting I to a character’s address, 
    so you can choose where to put it.But 050-09F is the standard for...some reason....welp why not lol */
    const unsigned char fonts [] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
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
    0xF0, 0x80, 0xF0, 0x80, 0x80 }; //F


    //Adding fonts from 0x050-0x09F (80-159)
    int start = FONTSTART;
    for(unsigned char x: fonts){
        memory[start] = x;
        //cout<<start<<endl;
        ++start;
    }
    //set the pc to start at 0x200
    pc = 0x200;
    flag = true;

    Chiprenderer = render;//Chip8 renderer pts to the windows one
}

void Chip8::draw(int x, int y){
    SDL_SetRenderDrawColor( Chiprenderer, 225, 225, 225, 225 );
    SDL_RenderDrawPoint(Chiprenderer,x, y);
    SDL_RenderPresent( Chiprenderer );
    SDL_Delay(100);
}

void Chip8::setKeys(){
/*1   2   3   4
Q   W   E   R
A   S   D   F
Z   X   C   V*/
keys[0] = SDL_SCANCODE_1;
keys[1] = SDL_SCANCODE_2;
keys[2] = SDL_SCANCODE_3;
keys[3] = SDL_SCANCODE_4;

keys[4] = SDL_SCANCODE_Q;
keys[5] = SDL_SCANCODE_W;
keys[6] = SDL_SCANCODE_E;
keys[7] = SDL_SCANCODE_R;

keys[8] = SDL_SCANCODE_A;
keys[9] = SDL_SCANCODE_S;
keys[10] = SDL_SCANCODE_D;
keys[11] = SDL_SCANCODE_F;

keys[12] = SDL_SCANCODE_Z;
keys[13] = SDL_SCANCODE_X;
keys[14] = SDL_SCANCODE_C;
keys[15] = SDL_SCANCODE_V;


}