
#include <cstdio>
#include <iostream>
#include "../model.h"
#include "../proc.h"
#include "../dram.h"

using namespace std;

/*
class dummy : public element {
public:
    full_duplex_port port;

    void cycle1() {
        if( port.in.data.valid ) {
            // すぐ送り返す
            port.out.data = port.in.data;
            printf( "dummy: data have come, returning it to sender\n" );
        }
    }

    void cycle2() {
    }
};
*/

int main() {
    composite config;
    pseudo_processor *proc = new pseudo_processor("addr.dat");
    //dummy *d = new dummy();
    dram_controller *dram = new dram_controller();
    config.elements.push_back( proc );
    config.elements.push_back( dram );
    config.elements.push_back( new channel( proc->mem_port, dram->port ) );
    config.elements.push_back( new channel( dram->port, proc->mem_port ) );

    printf( "press any key to step the simulation\n" );
    int cycle=0;
    for(;;) {
        char buff[1024];
        cin.getline( buff, 1024 );
        if( cin.fail() ) break;
        if( buff[0] == 'q' || buff[0] == 'Q' ) break;
        ++cycle;
        printf( "----------------------" );
        printf( "cycle #%d\n", cycle );
        config.cycle1();
        config.cycle2();
    }
    config.delete_elements();
    return 0;
}
