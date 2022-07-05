#include <iostream>
#include "SDL2/SDL.h"
#include <stdio.h>
#include "chip8.hpp"
#include <chrono>
#include <thread>
#include<curses.h>


using namespace std::chrono;




int main(int argc, char* args[]){


    Chip8 myChip;

    std::string romFile;
    std::cout << "Provide the ROM you would like to play"<<std::endl;
    std::cin >> romFile;

    //RENDERING
    SDL_Window *window;
    SDL_Renderer* renderer;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_CreateWindowAndRenderer(SCREENWIDTH * 4 , SCREENHEIGHT * 4, 0, &window, &renderer);
    SDL_RenderSetScale(renderer, 4, 4);
    //black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

  /*  SDL_SetRenderDrawColor( renderer, 225, 225, 225, 225 );
    SDL_RenderDrawPoint(renderer,SCREENWIDTH/2,SCREENHEIGHT/2);
    SDL_RenderPresent(renderer);*/




    



    /*    
        void draw();
        void setKeys();  */

    myChip.setUp( renderer);
  
    //romFile = "final.ch8";
    myChip.loadRom(romFile);
    myChip.setKeys();
    
   





    
    //auto now = steady_clock::now();
    auto prevFrame = steady_clock::now();
    //MAIN LOOP
    int count = 0;
    SDL_Event evt;
    bool programrunning = true;
    bool t = false;

    while(programrunning){
        while( SDL_PollEvent(&evt) )
        {  
            switch(evt.type)
            {
            case SDL_QUIT:  programrunning = false; break;
            case SDL_KEYDOWN: std::cout<<"Ahah!!!!"; t = true; break; 
            }
            if(t)
                break;
    }
        std::cout<<count<<": ";

        auto now = steady_clock::now();
        duration<double,std::milli> delta = now - prevFrame;
        
        if(delta.count() > .05){
        prevFrame = now;
        myChip.start();
        //std::chrono::duration<double, std::milli> delta_ms(1200.0 - delta.count());
        //auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
        //std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
     }
        count++;
    }
    SDL_Delay(1000);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

