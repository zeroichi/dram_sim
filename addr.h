// -*- mode:c++ -*-

#ifndef __ADDR_H__
#define __ADDR_H__

#include <string>

class addr_part {
public:
    addr_part( int bits, int lsb_start, std::string name );
    unsigned int get( unsigned int addr );
    void set( unsigned int *addr, unsigned int part );
    void shift( int delta );
    void info();
private:
    int bits;
    int lsb_start;
    unsigned int mask;
    std::string name;
};

#endif

