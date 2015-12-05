// Hertz, simple framerate locker. based on code by /u/concavator (ref: http://goo.gl/Ry50A4)
// - rlyeh. zlib/libpng licensed
//
// features:
// [x] auto-frameskip
// [x] dynamic/variable framerate locking

#pragma once
#include <cmath>
#include <chrono>
#include <thread>

#define HERTZ_VERSION "1.0.1" /* (2015/12/05) Add unlock function
#define HERTZ_VERSION "1.0.0" // (2015/09/19) Initial commit */

namespace hertz {
    // function that locks your logic and render to desired framerate (in HZ).
    // returns number of current fps
    template<typename FNU, typename FNR>
    static inline
    double lock( signed HZ, FNU &update, FNR &render ) {
        // rw vars
        static volatile unsigned hz = 60, isGameRunning = 1, maxframeskip = 10;
        // ro vars
        static volatile unsigned fps = 0;
        // private vars
        static volatile unsigned timer_counter = 0, loop_counter = 0;
        // private threaded timer
        static struct install {
            install() {
                std::thread([&]{
                    std::chrono::microseconds acc( 0 ), third( 300000 );
                    while( isGameRunning ) {
                        // update timer
                        timer_counter++;
                        std::chrono::microseconds duration( int(1000000/hz) );
                        std::this_thread::sleep_for( duration );
                        // update fps 3 times per second
                        acc += duration;
                        if( acc >= third ) {
                            acc -= acc;
                            static int before = loop_counter;
                            fps = int( std::round( (loop_counter - before) * 3.3333333 ) );
                            before = loop_counter;
                        }
                    }
                    isGameRunning = 1;
                }).detach();
            }
        } timer;

        if( HZ < 0 ) {
            isGameRunning = 0;
            return 1;
        }

        hz = HZ > 0 ? HZ : hz;

        // we got too far ahead, cpu idle wait
        while( loop_counter > timer_counter && isGameRunning ) {
            std::this_thread::yield();
        }

        // max auto frameskip is 10, ie, even if speed is low paint at least one frame every 10
        if( timer_counter > loop_counter + 10 ) {
            timer_counter = loop_counter;
        }

        loop_counter++;

        // only draw if we are fast enough, otherwise skip the frame
        update();
        if( loop_counter >= timer_counter ) {
            render();
        }
        return fps;
    }

    static inline void unlock() {
        auto nil = []{};
        hertz::lock( -1, nil, nil );
    }
}


#ifdef HERTZ_BUILD_SAMPLE

#ifdef _WIN32
#include <windows.h>
#pragma comment(lib, "user32.lib")
#endif

#include <algorithm>
#include <iostream>
#include <time.h>
#include <stdio.h>

int main() {
    unsigned HZ = 60, updates = 0, frames = 0, fps = 0;

    auto update = [&]{ 
        updates++;
#ifdef _WIN32
        static auto &once = std::cout << "Keys: up, down, escape, space" << std::endl;
        if( GetAsyncKeyState(VK_UP) & 0x8000 )  HZ+=(std::min)((std::max)(int(HZ*0.01), 1), 5);
        if( GetAsyncKeyState(VK_DOWN) & 0x8000 ) if(HZ > (std::max)(int(HZ*0.01), 1)) HZ-=(std::max)(int(HZ*0.01), 1);
        if( GetAsyncKeyState(VK_ESCAPE) & 0x8000 ) exit(0);
        if( GetAsyncKeyState(VK_SPACE) & 0x8000 ) Sleep( rand() % 80 );
#endif
    };

    auto render = [&]{ 
        frames++; 
        char bar[] = "\\|/-";
        auto current_time = time(NULL);
        auto sec = localtime(&current_time)->tm_sec;

        printf( "(%d/%d) [%c] updates %02ds [%c] frames \r",
            fps, HZ, bar[updates%4], sec, bar[frames%4] );
    };

    for(;;) {
        fps = hertz::lock( HZ, update, render );
    }

    hertz::unlock();
}

#endif