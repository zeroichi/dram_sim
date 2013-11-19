// -*- mode:c++ -*-

#ifndef __DRAM_H__
#define __DRAM_H__

#include <deque>
#include "model.h"

#define DEC(x) if((x)>0){--(x);}

enum dram_state {
    S_NULL,
    S_IDLE,
    S_ACTIVATING,
    S_ACTIVE,
    S_WRITE,
    S_READ,
    S_WRITEA, // write w/ auto precharge
    S_READA,  // read w/ auto precharge
    S_PRE,
};

enum dram_command {
    CMD_NOP,
    CMD_ACTIVATE,
    CMD_READ,
    CMD_READA,
    CMD_WRITE,
    CMD_WRITEA,
    CMD_PRE,
};


struct cmd_t {
    int cmd;
    int bank;
    int addr;
};


class dram_t {
private:
    class bank_t {
    public:
        int id;
        int state; // one of enum dram_state
        int cRCD, cCAS, cRAS, cRP, cRFC;
        dram_t *parent;
        bank_t();
        bank_t( dram_t*, int );
        // ~bank_t();
        void exec_p();
        void exec_n();
    };

    // DRAM parameters
    int tRCD; // アクティブコマンド入力 → 行アクティブになるまで
    int tCAS; // (tCL) readコマンド入力 → データがでてくるまで
    int tRAS; // アクティブコマンド入力 → プリチャージコマンド入力まで
    int tRP; // プリチャージコマンド入力 → 完了するまで
    int tRFC; // リフレッシュコマンド入力 → 完了するまで
    int tRC; // tRAS + tRP, 1つの行の操作開始から完了するのに少なくとも要する時間
    int tCCD; // カラムコマンド(read/write)入力 → カラムコマンド入力間隔 DDR3=4, DDR2=2, DDR=1
    int tRRD; // アクティブコマンド入力 → 別のバンクをアクティブできるまで
    int tWTR; // write to read ペナルティ
    int nbank;

    cmd_t cmd; // issued command
    int cRRD, cCCD, cWTR;
    bank_t *bank;
public:
    dram_t();
    ~dram_t();
    void exec_p();
    void exec_n();
    bool is_issuable( int cmd, int bankid );
};

// dramに対する読み書きのリクエストを表す構造体
class dram_req_t {
public:
    int rw;
    int bank;
    int row;
    int column;
};

// メモリコントローラ
class dram_controller : public element {
public:
    dram_controller();
    dram_t dram;
    std::deque<dram_req_t> queue;
};

#endif

