
#include <iostream>
#include "model.h"

using namespace std;

// =======================================================================================
// class mem_req_t

mem_req_t::mem_req_t()
    : valid(false), rw(0), addr(0), length(0), count(0), err(0)
{
}

mem_req_t::mem_req_t( int rw, unsigned int addr, int length )
    : valid(true), rw(rw), addr(addr), length(length), count(0), err(0)
{
}

std::ostream& operator <<( std::ostream& os, const mem_req_t& obj ) {
    os << "[mem_req_t rw=" << dec << obj.rw << ", addr=0x" << hex << obj.addr
       << ", len=" << dec << hex << ", err=" << obj.err << "]";
    return os;
}


// =======================================================================================
// class composite

void composite::cycle1() {
    for( std::vector<element*>::iterator it=elements.begin(); it!=elements.end(); ++it ) {
        (*it)->cycle1();
    }
}

void composite::cycle2() {
    for( std::vector<element*>::iterator it=elements.begin(); it!=elements.end(); ++it ) {
        (*it)->cycle2();
    }
}

void composite::delete_elements() {
    for( std::vector<element*>::iterator it=elements.begin(); it!=elements.end(); ++it ) {
        delete *it;
    }
    elements.clear();
}

// =======================================================================================
// class channel

channel::channel( port& input, port& output )
    : input(input), output(output) {
}

channel::channel( full_duplex_port& input, full_duplex_port& output )
    : input(input.in), output(output.out) {
}

void channel::cycle1() {
    // do nothing
}

void channel::cycle2() {
    //std::cout << "channel: cycle2()" << std::endl;
    input.data = output.data;
    output.data.valid = false;
}
