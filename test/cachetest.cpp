
#include <cstdio>
#include <iostream>
#include "../model.h"
#include "../proc.h"
#include "../dram.h"
#include "../cache.h"

using namespace std;

int main() {
    composite config;
    pseudo_processor *proc = new pseudo_processor("addr.dat");
    //dummy *d = new dummy();
    dram_controller *dram = new dram_controller();
    cache *mycache = new cache( 1024, 4, 64 );
    
    config.elements.push_back( proc );
    config.elements.push_back( dram );
    config.elements.push_back( mycache );
    config.elements.push_back( new channel( proc->mem_port, mycache->proc_port ) );
    config.elements.push_back( new channel( mycache->proc_port, proc->mem_port ) );
    config.elements.push_back( new channel( dram->port, mycache->dram_port ) );
    config.elements.push_back( new channel( mycache->dram_port, dram->port ) );

    printf( "press any key to step the simulation\n" );
    int cycle=0;
    for(;;) {
        char buff[1024];
        cin.getline( buff, 1024 );
        if( cin.fail() ) break;
        if( buff[0] == 'q' || buff[0] == 'Q' ) break;
        ++cycle;
        printf( "-[cycle #%d]----------------------------------------\n", cycle );
        config.cycle1();
        config.cycle2();
    }
    config.delete_elements();
    return 0;
}
