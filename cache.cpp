
#include "dram.h"

// キャッシュメモリに格納されるキャッシュライン
class cacheline {
    bool valid;
    bool dirty;
    unsigned int tag;

    cacheline() : valid(false), dirty(false), tag(0) {
    }
};

// キャッシュメモリ
class cache : element {
public:
    int rows;
    int sets;
    int linesize;
    cacheline* lines;
    full_duplex_port proc_port;
    full_duplex_port dram_port;

    cache( int rows, int sets, int linesize ) : rows(rows), sets(sets), linesize(linesize) {
        lines = new cacheline[rows*sets];
    }

    ~cache() {
        delete lines;
    }
};

class pseudo_processor : element {
public:
    vector<mem_req_t> accesses;
    int index;
    full_duplex_port port;

    pseudo_processor() : index(0) {
    }

    void cycle1() {
        if( index >= accesses.size() ) return;
        port.out.data = accesses[index];
        ++index;
    }

    void cycle2() {
        // do nothing
    }
};
