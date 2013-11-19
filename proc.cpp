
#include <iostream>
#include <cstdio>
#include "proc.h"

using namespace std;

// constructor
// filename : メモリアクセスが記録されたファイル
pseudo_processor::pseudo_processor( const char *filename )
    : counter( 0 ), ready( true )
{
    unsigned int addr;
    accesses.clear();
    FILE *fp = fopen( filename, "rb" );
    if( fp == NULL ) throw "file not found";
    while( !feof(fp) ) {
        if( 0 >= fread( &addr, sizeof(addr), 1, fp ) )
            break;
        accesses.push_back( mem_req_t( 0, addr, 4 ) );
    }
    cout << "processor: data read done. count=" << accesses.size() << endl;
    fclose( fp );
}

void pseudo_processor::cycle1() {
    if( ready && counter<accesses.size() ) {
        cout << "processor: requesting address 0x" << hex << accesses[counter].addr << endl;
        mem_port.out.data = accesses[counter];
        ready = false;
        ++counter;
    } else {
        // ポートにデータが来ているかをチェック
        if( mem_port.in.data.valid ) {
            cout << "processor: data arrived" << endl;
            ready = true;
        }
    }
}

void pseudo_processor::cycle2() {
    // do nothing
}
