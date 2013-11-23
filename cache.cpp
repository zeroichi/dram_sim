
#include <cstdio>
#include "dram.h"
#include "cache.h"

using namespace std;

// ================================================================================================
// class cacheline

// constuctor
cacheline::cacheline() : valid(false), dirty(false), tag(0), lasttime(0) {
}


// ================================================================================================
// class cache

// constructor
cache::cache( int nrows, int nsets, int linesize ) : nrows(nrows), nsets(nsets), linesize(linesize) {
    lines = new cacheline[nrows*nsets];
    int lsb = 0;
    addr_offset = addr_part( needed_bits( linesize ), lsb, "offset" );
    lsb += needed_bits( linesize );
    addr_row = addr_part( needed_bits(nrows), lsb, "row" );
    lsb += needed_bits( nrows );
    addr_tag = addr_part( sizeof(unsigned int)*8 - lsb, lsb, "tag" );
    cout << "cache: [info] nrows=" << nrows << ", nsets=" << nsets << ", linesize=" << linesize << endl;
}

cache::~cache() {
    delete lines;
}

void cache::cycle1() {
    // lasttime の更新
    for( int i=0; i<nrows*nsets; ++i ) {
        if( lines[i].valid )
            ++lines[i].lasttime;
    }

    // プロセッサからリクエストが来ているかどうかをチェックする
    if( proc_port.in.data.valid ) {
        cout << "cache: request - " << proc_port.in.data << endl;
        const unsigned int &addr = proc_port.in.data.addr;
        // アラインメントチェック: 行をまたぐリクエストは無効
        if( proc_port.in.data.length > linesize
            || addr_row.get(addr) != addr_row.get(addr + proc_port.in.data.length - 1) ) {
            printf( "cache: a request failed to pass alignment check\n" );
            proc_port.out.data = proc_port.in.data;
            proc_port.out.data.err = 1;
        }
        // ヒットしたかどうかをチェック
        int i = nsets * addr_row.get(addr);
        int j=0;
        for( ; j<nsets; ++j ) {
            if( lines[i+j].valid && lines[i+j].tag == addr_tag.get(addr) ) {
                // hit
                cout << "cache: hit" << endl;
                proc_port.out.data = proc_port.in.data;
                lines[i+j].lasttime = 0;
                break;
            }
        }
        if( j==nsets ) {
            // miss
            cout << "cache: miss" << endl;
            queue.push_back( proc_port.in.data );
            // dram に要求
            dram_port.out.data = mem_req_t( proc_port.in.data.rw, proc_port.in.data.addr & ~addr_offset.get_mask(), linesize );
        }
    }

    // dram から結果を受けとる
    if( dram_port.in.data.valid ) {
        int i = nsets * addr_row.get( dram_port.in.data.addr );
        cacheline *dest = NULL;
        // 既にあるか調べる
        for( int j=0; j<nsets; ++j ) {
            if( lines[i+j].valid && lines[i+j].tag == addr_tag.get( dram_port.in.data.addr ) ) {
                dest = &lines[i+j];
                break;
            }
        }
        if( dest == NULL ) {
            // 空きスロットを探す
            for( int j=0; j<nsets; ++j ) {
                if( ! lines[i+j].valid ) {
                    dest = &lines[i+j];
                    break;
                }
            }
            if( dest == NULL ) {
                // 空きなし -> 入れ替えるスロットを決める (LRU)
                dest = &lines[i];
                for( int j=1; j<nsets; ++j ) {
                    if( dest->lasttime < lines[i+j].lasttime ) {
                        dest = &lines[i+j];
                    }
                }
            }
            dest->valid = true;
            dest->dirty = false;
            dest->tag = addr_tag.get( dram_port.in.data.addr );
            dest->lasttime = 0;
        }
    }

    // リクエストされたデータがキャッシュに取り込まれたか確認する
    for( int i=0; i<queue.size(); ++i ) {
        int j = nsets * addr_row.get(queue[i].addr);
        for( int k=0; k<nsets; ++k ) {
            if( lines[j+k].valid && lines[j+k].tag == addr_tag.get(queue[i].addr) ) {
                proc_port.out.data = queue[i];
                // 1サイクルに出力できるのは1つまでなので、1つ出力したらループを抜ける
                cout << "cache: output to proc: " << proc_port.out.data << endl;
                // キューから要素を削除
                queue.erase( queue.begin()+i );
                goto quit1;
            }
        }
    }
quit1:;
}

void cache::cycle2() {

}
