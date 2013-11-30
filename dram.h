// -*- mode:c++ -*-

#ifndef __DRAM_H__
#define __DRAM_H__

#include <vector>
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
    //mem_req_t *from; // 元になっているリクエスト

    //dram_req_t( int rw, int bankid, int row, int col, mem_req_t* from );
    dram_req_t( int rw, int bankid, int row, int col );
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
    // 内部で使用する型の宣言
    struct request_t { // 対コントローラ・リクエスト
        mem_req_t original;
        unsigned int addr; // アラインメント済みアドレス
        int required; // 必要な r/w コマンド発行回数
        int requested; // 発行済みコマンド数
        int count; // r/w 完了コマンド数 (wait_t.target でポイントされる)
    };
    struct bank_req_t { // 対バンク・リクエスト
        int row; // 行番号
        int col; // 列番号
        int rw; // Read=0 or Write=1
        int *counter; // リクエスト処理完了時にカウントアップされる変数へのポインタ

        bank_req_t( int row, int col, int rw, int *counter );
    };
    struct wait_t {
        int type; // 通知タイプ
        int *counter; // タイムアウト時にカウントアップする変数へのポインタ

        wait_t();
        wait_t( int type, int *counter );
    };

    dram_t dram; // DRAM 本体
    full_duplex_port port; // キャッシュメモリ・プロセッサと通信するためのポート
    unsigned int align_mask; // メモリアドレスアラインメント用

    std::deque<request_t> reqq; // プロセッサからのリクエストキュー
    std::vector<bank_req_t>* bankq; // 対バンク・リクエストキュー(配列)
    std::deque<wait_t> waitq; // 処理待ちキュー <= 常に要素数が一定のリングバッファ

    dram_controller();
    ~dram_controller();
    // override methods from abstract class 'element'
    void cycle1();
    void cycle2();
};

enum gather_command {
    GCMD_VL,
    GCMD_VLS,
    GCMD_VLI,
    GCMD_VLPI,
    GCMD_VS,
    GCMD_VSS,
    GCMD_VSI,
    GCMD_VSPI,
};

// Gatherコマンド
class gather_cmd_t {
public:
    // プロセッサから指定されるパラメータ群
    int cmd; // コマンド名
    unsigned int array_addr; // データ配列開始アドレス
    unsigned int index_addr; // インデックス配列開始アドレス
    int data_size; // 要素型のサイズ
    int length; // Gatherする要素数
    int buffer_id; // 読み書きするバッファ番号

    // Gatherシステム内部で使用する変数
    bool valid; // 有効要素かどうか
    int status; // 進行状態を表すステータス
    int done_count; // どこまで終わったか
};

enum gather_status {
    GS_NULL, // 無効
    GS_INDEX, // インデックスを読み込んでいる段階
    GS_GATHER, // Gather を行っている段階
    GS_DONE, // 処理完了
};

// Gatherコントローラ
class gather_controller {
public:
    // パラメータ
    int ncmds; // 同時発行可能Gatherコマンド数
    int maxsize; // Gather可能最大要素数

    dram_t *dram;
    dram_controller *dramc;
    std::deque<gather_cmd_t> queue; // コマンドキュー

    gather_controller();
    void cycle();
};

#endif

