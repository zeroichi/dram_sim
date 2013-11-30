
#include <new>
#include <iostream>
#include <cstdio>
#include <bitset>
#include "dram.h"

using namespace std;

const char * const state_string_table[] = {
    "NULL",
    "IDLE",
    "ACTING",
    "ACTIVE",
    "WRITE",
    "READ",
    "WRITEA",
    "READA",
    "PRE"
};

static const char * state_to_s( const int state ) {
    const int table_size = sizeof(state_string_table)/sizeof(*state_string_table);
    if( state < 0 || table_size <= state ) {
        return "(invalid)";
    }
    return state_string_table[state];
}

// =======================================================================================
// class dram_t::bank_t

dram_t::bank_t::bank_t() {}

// constructor
dram_t::bank_t::bank_t( dram_t *parent, int id ) {
    // DDR3-1333
    state = S_IDLE;
    cRCD = cCAS = cRAS = cRP = cRFC = 0;
    this->parent = parent;
    this->id = id;
    row = 0;
}

void dram_t::bank_t::do_update() {
    DEC(cRCD);
    DEC(cCAS);
    DEC(cRAS);
    DEC(cRP);
    DEC(cRFC);
    if( state == S_ACTIVATING && cRCD == 0 ) {
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
}

void dram_t::bank_t::do_command() {
    // コマンド発行可能な状態かチェック
    if( !parent->is_issuable( parent->cmd ) ) {
        cout << "dram: [ERR] not issuable - " << parent->cmd << endl;
        return;
    }
    if( parent->cmd.bank != id ) {
        // コントローラが対象バンクに対してしかdo_command()を呼び出さないので
        // ここは通らないはず
        return;
    }
    const int &cmd = parent->cmd.cmd;
    if( cmd == CMD_ACTIVATE ) {
        state = S_ACTIVATING;
        cRCD = parent->tRCD;
        cRAS = parent->tRAS;
        parent->cRRD = parent->tRRD;
        row = parent->cmd.addr;
    } else if ( cmd == CMD_PRE ) {
        state = S_PRE;
        cRP = parent->tRP;
    } else if ( cmd == CMD_READ ) {
        state = S_READ;
        parent->cCCD = parent->tCCD;
        cCAS = parent->tCAS;
    } else if ( cmd == CMD_WRITE ) {
        state = S_WRITE;
        parent->cCCD = parent->tCCD;
        cCAS = parent->tCAS;
    }
}


// =======================================================================================
// class dram_t

// constructor
dram_t::dram_t()
{
    width  = 8;
    ncols  = 1024;
    nrows  = 8192;
    nbanks = 8;
    burst  = 8;
    cout << "dram: [info] bus_width="<<width<<"b, ncols="<<ncols<<", nrows="<<nrows<<", nbanks="<<nbanks<<", burst_len="<<burst<<endl;
    cout << "dram: [info] page_size="<<width*ncols/8<<" B, total="<<width*ncols*nrows*nbanks/8<<" B"<<endl;

    // アドレス計算フィルタを初期化
    const int burst_bits = needed_bits(burst);
    int lsb=burst_bits;
    bank_addr = addr_part( needed_bits(nbanks), lsb, "bank" );
    lsb += needed_bits(nbanks);
    // バースト長の分(3bits)を引く
    col_addr  = addr_part( needed_bits(ncols)-burst_bits , lsb, "col" );
    lsb += needed_bits(ncols) - burst_bits;
    row_addr  = addr_part( needed_bits(nrows) , lsb, "row" );

    tCAS = 9;
    tRCD = 9;
    tRAS = 24;
    tRP  = 9;
    tRC  = tRAS + tRP;
    tRFC = 74;
    tCCD = 4;
    tRRD = 4;
    
    cRRD = cCCD = 0;
    bank = new bank_t[nbanks];
    for( int i=0; i<nbanks; ++i ) {
        // placement new
        new(&bank[i]) bank_t( this, i );
    }
}

dram_t::~dram_t() {
    delete bank;
}

// コマンド発行条件を満たしているかをチェックする
bool dram_t::is_issuable( const cmd_t& command ) const {
    const int cmd = command.cmd;

    // グローバルなコマンド
    if( cmd == CMD_NOP ) {
        // 無条件でok
        return true;
    }

    // バンクローカルなコマンド
    const int bankid = command.bank;
    const int state = bank[bankid].state;
    if( cmd == CMD_ACTIVATE ) {
        if( state == S_IDLE && cRRD==0 ) {
            return true;
        } else {
            return false;
        }
    } else if ( cmd == CMD_PRE ) {
        if( ( state == S_ACTIVE || state == S_READ || state == S_WRITE )
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

    // invalid command number
    printf( "dram_t::is_issuable(): invalid command number = %d\n", cmd );
    return false;
}

void dram_t::do_update() {
    DEC(cRRD);
    DEC(cCCD);
    for( int bankid=0; bankid<nbanks; ++bankid ) {
        bank[bankid].do_update();
    }
}

void dram_t::do_command() {
    // debug print
    //printf( "dram: cRRD=%d, cCCD=%d\n", cRRD, cCCD );
    if( cmd.cmd != CMD_NOP )
        cout << "dram: cmd=" << cmd << endl;

    // グローバルなコマンド

    // バンクローカルなコマンド
    if( cmd.cmd!=CMD_NOP && is_issuable( cmd ) ) {
        bank[cmd.bank].do_command();
    }

    // コマンドを無効化
    cmd.cmd = CMD_NOP;
}


// =======================================================================================
// class dram_req_t

//dram_req_t::dram_req_t( int rw, int bankid, int row, int col, mem_req_t* from )
//    : rw(rw), bank(bankid), row(row), col(col), from(from)
//{
//}
dram_req_t::dram_req_t( int rw, int bankid, int row, int col )
    : rw(rw), bank(bankid), row(row), col(col)
{
}

// =======================================================================================
// class schedule_t

schedule_t::schedule_t( int type, int count, dram_req_t& req )
    : type(type), count(count), req(req)
{
}

// =======================================================================================
// class dram_controller

// constructor
dram_controller::dram_controller()
    : waitq( 64 )
{
    align_mask = ~(dram.width * dram.burst / 8 - 1);
    cout << "dramc: [debug] align_mask=" << bitset<32>(align_mask) << endl;
    bankq = new vector<bank_req_t>[dram.nbanks];
    cout << "dramc: [debug] nbanks=" << dram.nbanks << ", bankq=" << (void*)bankq << endl;
}

dram_controller::~dram_controller() {
    delete[] bankq;
}

void dram_controller::cycle1() {
    // dram の内部更新処理
    dram.do_update();

    // ポートにリクエストが来ているか確認
    if( port.in.data.valid ) {
        printf( "dramc: [debug] request came\n" );
        printf( "       rw=%d addr=0x%x length=%d\n", port.in.data.rw, port.in.data.addr, port.in.data.length );
        // '対コントローラ'リクエストキューに入れる
        request_t newreq;
        newreq.original  = port.in.data;
        newreq.addr      = newreq.original.addr & align_mask;
        newreq.required  = 1;
        newreq.requested = 0;
        newreq.count     = 0;
        // 全部で何回のDRAMコマンド発行が必要になるかを数える
        while( newreq.addr + newreq.required * ( dram.width * dram.burst / 8 ) < newreq.original.addr + newreq.original.length ) {
            newreq.required++;
        }
        reqq.push_back( newreq );
        cout << "dramc: [debug] request queued - addr=" << (void*)newreq.addr << ", required=" << newreq.required << endl;
    }

    // '対コントローラ'リクエストを処理
    for( deque<request_t>::iterator it=reqq.begin(); it!=reqq.end(); ) {
        cout << dec << "dramc: [debug] required=" << (*it).required << ", requested=" << (*it).requested << ", count=" << (*it).count << endl;
        if( (*it).required == (*it).count ) {
            // 完了済み => 出力
            if( !port.out.data.valid ) {
                port.out.data = it->original;
                port.out.data.err = 0;
                // todo: キューから削除
                it=reqq.erase(it);
            } else {
                ++it;
            }
        } else {
            // バンクに対する要求
            for( int i=(*it).requested; i<(*it).required; ++i ) {
                // i は何回目のコマンド発行かを表す
                unsigned int addr = (*it).addr + i * dram.width * dram.burst / 8;
                int bankid = dram.bank_addr(addr);
                if( bankq[bankid].size() != 0 ) break; // バンクキューが空でない
                //bankq[bankid].push_back( dram_req_t( (*it).original.rw, bankid, dram.row_addr(addr), dram.col_addr(addr), 
                bankq[bankid].push_back( bank_req_t( dram.row_addr(addr), dram.col_addr(addr), (*it).original.rw, &(*it).count ) );
                (*it).requested++;
            }
            ++it;
        }
    }

    // '対バンク'リクエストを処理
    for( int i=0; i<dram.nbanks; ++i ) {
        if( bankq[i].size() >= 1 ) {
            dram_t::bank_t &b = dram.bank[i];
            bank_req_t &req = bankq[i][0];
            cmd_t cmd;
            // アクティブか？
            if( b.state == S_IDLE ) {
                cmd.cmd = CMD_ACTIVATE;
                cmd.bank = i;
                cmd.addr = req.row;
            } else if( b.state == S_ACTIVE ) {
                if( b.row == req.row ) {
                    cmd.cmd = CMD_READ;
                    cmd.bank = i;
                    cmd.addr = req.col;
                } else {
                    cmd.cmd = CMD_PRE;
                    cmd.bank = i;
                }
            }
            // コマンドを発行できるかチェック
            if( cmd.cmd!=CMD_NOP && dram.is_issuable( cmd ) ) {
                //cout << "dramc: issue - " << cmd << endl;
                dram.cmd = cmd;
                if( cmd.cmd == CMD_READ ) {
                    // tCAS + バースト長/2 サイクル後に読み込み完了
                    int needed_cycles = dram.tCAS + dram.burst / 2;
                    //waitq.push_back( wait_t( 1, needed_cycles, req.counter ) );
                    waitq[needed_cycles].type = 1;
                    waitq[needed_cycles].counter = req.counter;
                    bankq[i].pop_back();
                    cout << "bank[" << i << "]: scheduled, cycles=" << needed_cycles << " cmd=" << cmd << endl;
                    //schedules.push_back( schedule_t( 0, dram.tCAS+dram.burst/2, req ) );
                    //waits.push_back( wait_t( dram.tCAS+dram.burst/2, (int*)0/* todo */ ) );
                    //cout << "bank " << i << ": " << cmd << endl;
                    //cout << "  data transfer will be completed in " << dec << dram.tCAS+dram.burst/2 << " cycles" << endl;
                    //bankq[i].pop_front();
                }
                break;
            }
        }
    }

    // 処理待ちリングバッファの処理
    if( waitq[0].type != 0 ) {
        cout << "dramc: [debug] waitq[0].type=" << waitq[0].type << endl;
        (*waitq[0].counter)++;
    }
    waitq.pop_front();
    waitq.push_back( wait_t() );
    //cout << "dramc: [debug] waitq.size=" << waitq.size() << endl;

    // dram のコマンド処理
    dram.do_command();

    // debug print
    printf( "dram: cRRD=%d, cCCD=%d\n", dram.cRRD, dram.cCCD );
    for( int bankid=0; bankid<dram.nbanks; ++bankid ) {
        dram_t::bank_t& b = dram.bank[bankid];
        printf( "bank[%d]: state=%-6s cRCD=%d cCAS=%d cRAS=%2d cRP=%d row=%d\n", bankid, state_to_s(b.state), b.cRCD, b.cCAS, b.cRAS, b.cRP, b.row );
    }
}

void dram_controller::cycle2() {
}

dram_controller::bank_req_t::bank_req_t( int row, int col, int rw, int *counter )
    : row(row), col(col), rw(rw), counter(counter)
{
}

dram_controller::wait_t::wait_t()
    : type(0), counter(NULL)
{
}

dram_controller::wait_t::wait_t( int type, int *counter )
    : type(type), counter(counter)
{
}

// =======================================================================================
// class cmd_t

cmd_t::cmd_t()
    : cmd(CMD_NOP), bank(0), addr(0)
{
}

cmd_t::cmd_t( int cmd, int bank, unsigned int addr )
    : cmd(cmd), bank(bank), addr(addr)
{
}

static const char * const cmd_string_table[] = {
    "NOP",
    "ACTIVATE",
    "READ",
    "READA",
    "WRITE",
    "WRITEA",
    "PRE"
};

void cmd_t::print() const {
    printf( "[cmd=%s, bank=%d, addr=0x%x]", cmd_string_table[cmd], bank, addr );
}

std::ostream& operator <<( std::ostream& os, const cmd_t& value ) {
    value.print();
    return os;
}

// =======================================================================================
// class gather_controller

gather_controller::gather_controller()
    : ncmds(8), maxsize(1024)
{
}

void gather_controller::cycle() {
    for( int i=0; i<queue.size(); ++i ) {
        if( queue[i].status == GS_INDEX ) {
            for( int j=queue[i].done_count; j<queue[i].length; ++j ) {
                unsigned int addr = queue[i].index_addr + queue[i].data_size * j; // 要求するアドレス
                int bankid = dram->bank_addr.get(addr); // 上のアドレスに対応するバンク番号
                if( 0 < dramc->bankq[bankid].size() ) {
                    // the bank is busy, give up
                    break;
                }
                // read コマンド発行準備
                //dram_req_t req( 0, bankid, dram->row_addr.get(addr), dram->col_addr.get(addr), 
                //              NULL /* TODO: mem_req_t ? */ );
            }
        }
    }
}

