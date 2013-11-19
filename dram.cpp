
#include <new>
#include <iostream>
#include "dram.h"

dram_t::bank_t::bank_t() {}

// constructor
dram_t::bank_t::bank_t( dram_t *parent, int id ) {
    // DDR3-1333
    state = S_IDLE;
    cRCD = cCAS = cRAS = cRP = cRFC = 0;
    this->parent = parent;
    this->id = id;
}

void dram_t::bank_t::exec_p() {
    DEC(cRCD);
    DEC(cCAS);
    DEC(cRAS);
    DEC(cRP);
    DEC(cRFC);

    if( state == S_ACTIVATING && cRCD != 0 ) {
        state = S_ACTIVE;
    }
    if( state == S_READ && parent->cCCD == 0 ) {
        state = S_ACTIVE;
    }
    if( state == S_WRITE && parent->cCCD == 0 ) {
        state = S_ACTIVE;
    }
    if( state == S_PRE && cRP == 0 ) {
        state = S_IDLE;
    }

    if( parent->cmd.bank != id ) return;
    int cmd = parent->cmd.cmd;
    if( cmd == CMD_ACTIVATE ) {
        if( state == S_IDLE ) {
            state = S_ACTIVATING;
            cRCD = parent->tRCD;
            cRAS = parent->tRAS;
        } else {
            throw "err: cmd==ACTIVATE but state!=IDLE";
        }
    } else if ( cmd == CMD_PRE ) {
        if( state == S_ACTIVE || state == S_READ || state == S_WRITE
            && cRAS == 0 ) {
            state = S_PRE;
            cRP = parent->tRP;
        } else {
            throw "err: cmd==PRE but state!=ACTIVE,READ,WRITE";
        }
    } else if ( cmd == CMD_READ ) {
        if( state == S_ACTIVE && parent->cCCD == 0 ) {
            state = S_READ;
            parent->cCCD = parent->tCCD;
            cCAS = parent->tCAS;
        } else {
            throw "err: cmd==READ, conditions not met";
        }
    } else if ( cmd == CMD_WRITE ) {
        if( state==S_ACTIVE && parent->cCCD==0 ) {
            state = S_WRITE;
            parent->cCCD = parent->tCCD;
            cCAS = parent->tCAS;
        } else {
            throw "err: cmd==WRITE, conditions not met";
        }
    }
}

void dram_t::bank_t::exec_n() {

}


// constructor
dram_t::dram_t() {
    tCAS = 9;
    tRCD = 9;
    tRAS = 24;
    tRP = 9;
    tRC = tRAS + tRP;
    tRFC = 74;
    tCCD = 4;
    tRRD = 4;
    
    cRRD = cCCD = 0;
    bank = new bank_t[nbank];
    for( int i=0; i<nbank; ++i ) {
        // placement new
        new(&bank[i]) bank_t( this, i );
    }
}

dram_t::~dram_t() {
    delete bank;
}

void dram_t::exec_p() {
    DEC(cRRD);
    DEC(cCCD);

    if( cmd.cmd == CMD_ACTIVATE ) {
        if( cRRD != 0 ) {
            throw "err: cmd=ACTIVATE but cRRD>0";
        }
        cRRD = tRRD;
    }

    for( int i=0; i<nbank; ++i ) {
        bank[i].exec_p();
    }
}

void dram_t::exec_n() {
    for( int i=0; i<nbank; ++i ) {
        bank[i].exec_n();
    }
}

bool dram_t::is_issuable( int cmd, int bankid ) {
    const int state = bank[bankid].state;
    if( cmd == CMD_ACTIVATE ) {
        if( state == S_IDLE ) {
            return true;
        } else {
            return false;
        }
    } else if ( cmd == CMD_PRE ) {
        if( state == S_ACTIVE || state == S_READ || state == S_WRITE
            && bank[bankid].cRAS == 0 ) {
            return true;
        } else {
            return false;
        }
    } else if ( cmd == CMD_READ ) {
        if( state == S_ACTIVE && cCCD == 0 ) {
            return true;
        } else {
            return false;
        }
    } else if ( cmd == CMD_WRITE ) {
        if( state==S_ACTIVE && cCCD==0 ) {
            return true;
        } else {
            return false;
        }
    }
}

// =======================================================================================
// class dram_controller

