// -*- mode:c++ -*-

#ifndef __DRAM_H__
#define __DRAM_H__

#include <deque>
#include "addr.h"
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

//extern const char *cmd_string_table const;

struct cmd_t {
    int cmd;
    int bank;
    unsigned int addr;

    cmd_t();
    cmd_t( int cmd, int bank, unsigned int addr );
    void print() const;
};

std::ostream& operator <<( std::ostream& os, const cmd_t& value );

class dram_t {
public:
    class bank_t {
    public:
        int id;
        int state; // one of enum dram_state
        unsigned int row; // active row
        int cRCD, cCAS, cRAS, cRP, cRFC;
        dram_t *parent;
        bank_t();
        bank_t( dram_t*, int );
        // ~bank_t();
        void do_update();  // 内部カウンタ更新処理
        void do_command(); // コマンド処理
    };

    // DRAM parameters
    int tRCD;   // アクティブコマンド入力 → 行アクティブになるまで
    int tCAS;   // (tCL) readコマンド入力 → データがでてくるまで
    int tRAS;   // アクティブコマンド入力 → プリチャージコマンド入力まで
    int tRP;    // プリチャージコマンド入力 → 完了するまで
    int tRFC;   // リフレッシュコマンド入力 → 完了するまで
    int tRC;    // tRAS + tRP, 1つの行の操作開始から完了するのに少なくとも要する時間
    int tCCD;   // カラムコマンド(read/write)入力 → カラムコマンド入力間隔 DDR3=4, DDR2=2, DDR=1
    int tRRD;   // アクティブコマンド入力 → 別のバンクをアクティブできるまで
    int tWTR;   // write to read ペナルティ

    cmd_t cmd;  // issued command
    int cRRD, cCCD, cWTR;
    bank_t *bank;

    // メモリ容量 = width * nrows * ncols * nbanks (bits)
    int width;  // バス幅(ワード長), 単位 bits
    int nrows;  // 行数
    int ncols;  // 列数
    int nbanks; // バンク数
    int burst;  // バースト長
    addr_part row_addr;
    addr_part col_addr;
    addr_part bank_addr;

    dram_t();
    ~dram_t();
    //bool is_issuable( int cmd, int bankid );
    bool is_issuable( const cmd_t& command ) const;
    void do_update();  // 内部カウンタ更新処理
    void do_command(); // コマンド処理
};

// dramに対する読み書きのリクエストを表す構造体
class dram_req_t {
public:
    int rw;
    int bank;
    int row;
    int col;
    mem_req_t *from; // 元になっているリクエスト

    dram_req_t( int rw, int bankid, int row, int col, mem_req_t* from );
};

// コマンドを発行し，読み込み・書き込みの完了待ちを表す
class schedule_t {
public:
    int type;
    int count;
    dram_req_t req;

    schedule_t( int type, int count, dram_req_t& req );
};

// メモリコントローラ
class dram_controller : public element {
public:
    dram_controller();
    dram_t dram;
    std::deque<mem_req_t> queue;
    std::deque<dram_req_t> bankq[8];
    full_duplex_port port;
    std::vector<schedule_t> schedules;

    // from class 'element'
    void cycle1();
    void cycle2();
};

#endif

