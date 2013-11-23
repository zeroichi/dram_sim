// -*- mode:c++ -*-

#ifndef __CACHE_H__
#define __CACHE_H__

#include <vector>
#include "addr.h"
#include "model.h"

// キャッシュメモリに格納されるキャッシュライン
class cacheline {
public:
    bool valid;
    bool dirty;
    unsigned int tag;
    unsigned int lasttime;

    cacheline();
};

// キャッシュメモリ
class cache : public element {
public:
    int nrows;
    int nsets;
    int linesize;
    cacheline* lines;
    full_duplex_port proc_port;
    full_duplex_port dram_port;
    addr_part addr_tag;
    addr_part addr_row;
    addr_part addr_offset;
    std::vector<mem_req_t> queue;

    cache( int nrows, int nsets, int linesize );
    ~cache();
    void cycle1();
    void cycle2();
};

#endif
