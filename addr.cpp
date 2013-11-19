
#include <iostream>
#include <string>
#include <vector>
#include "addr.h"

using namespace std;

#define S_IDLE 0
#define S_ACTIVE 1

addr_part::addr_part( int bits, int lsb_start, string name )
    : bits( bits ), lsb_start( lsb_start ), name( name )
{
    // size が 2 のべき乗であるかチェックする
    this->mask = ( ( 1u << bits ) - 1 ) << lsb_start;
}

unsigned int addr_part::get( unsigned int addr ) {
    return ( addr & mask ) >> lsb_start;
}

void addr_part::set( unsigned int *addr, unsigned int part ) {
    // clear bits
    *addr &= !mask;
    *addr |= ( part << lsb_start ) & mask;
}

void addr_part::shift( int delta ) {
    if( delta > 0 ) {
        lsb_start += delta;
        mask <<= delta;
    } else if ( delta < 0 ) {
        delta = -delta;
        lsb_start -= delta;
        mask >>= delta;
    }
}

void addr_part::info() {
    cout << "name=" << name;
    cout << "\t bits=" << dec << bits;
    cout << "\t mask=" << hex << mask;
    cout << "\t lsb=" << dec << lsb_start;
    cout << "\t msb=" << lsb_start+bits-1;
    cout << dec << endl;
}


//////////////////////////////////////////////////////////

int needed_bits( unsigned int max ) {
    const int bits_max = 32;
    for( int bits=1; bits<bits_max; ++bits )
    if( max <= (1u << bits) )
        return bits;
    return bits_max;
}

void test1() {
    cout << "test1 running" << endl;
    unsigned int num;
    for(;;) {
        cout << "input unsigned number :" << endl;
        cin >> num;
        if( cin.fail() ) break;
        cout << "needed_bits = " << needed_bits( num ) << endl;
        cout << endl;
    }
}

void test2() {
    int n, total;
    total = 0;
    n = needed_bits( 64 );
    addr_part offset( n, total, "offset" );
    total += n;
    n = needed_bits( 8 );
    addr_part bank( n, total, "bank" );
    total += n;
    n = needed_bits( 1024 );
    addr_part column( n, total, "column" );
    total += n;
    n = needed_bits( 8192 );
    addr_part row( n, total, "row" );
    total += n;

    offset.info();
    bank.info();
    column.info();
    row.info();
    
    unsigned int ad;
    for(;;) {
        cout << "input address:" << endl;
        cin >> ad;
        if( cin.fail() ) break;
        cout << "addr = " << hex << ad << endl;
        cout << "row = " << hex << row.get(ad) << "(" << dec << row.get(ad) << ")" << endl;
        cout << "col = " << hex << column.get(ad) << "(" << dec << column.get(ad) << ")" << endl;
        cout << "bank= " << hex << bank.get(ad) << "(" << dec << bank.get(ad) << ")" << endl;
        cout << "offs= " << hex << offset.get(ad) << "(" << dec << offset.get(ad) << ")" << endl;
    }
}

/*
int main() {
    //test1();
    test2();
    return 0;
}
*/
