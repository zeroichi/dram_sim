// -*- mode: c++ -*-

#ifndef __PROC_H__
#define __PROC_H__

#include <vector>
#include "model.h"

class pseudo_processor : public element {
public:
    std::vector<mem_req_t> accesses;
    pseudo_processor( const char *filename );
    void cycle1();
    void cycle2();
};

#endif
