
#include <cstdio>
#include "proc.h"

using namespace std;

// constructor
// filename : メモリアクセスが記録されたファイル
pseudo_processor::pseudo_processor( const char *filename ) {
    unsigned int addr;
    accesses.clear();
    FILE *fp = fopen( filename, "rb" );
    if( fp == NULL ) throw "file not found";
    while( !feof(fp) ) {
        if( 0 >= fread( &addr, sizeof(addr), 1, fp ) )
            break;
        accesses.push_back( mem_req_t( 0, addr, 4 ) );
    }
    fclose( fp );
}

void pseudo_processor::cycle1() {

}

void pseudo_processor::cycle2() {

}
