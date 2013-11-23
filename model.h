// -*- mode:c++ -*-

#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>

// メモリ読み書きリクエスト
class mem_req_t {
public:
    bool valid;
    int rw; // 0:read, 1:write
    unsigned int addr;
    int length;
    int count;
    int err;

    mem_req_t();
    mem_req_t( int rw, unsigned int addr, int length );
};

// 単方向入出力ポート
class port {
public:
    mem_req_t data;
};

// 全二重(双方向)入出力ポート
class full_duplex_port {
public:
    port in;
    port out;
};

// シミュレーション要素
class element {
public:
    virtual void cycle1() = 0;
    virtual void cycle2() = 0;
};

// 複合シミュレーション要素
class composite : public element {
public:
    std::vector<element*> elements;
    void delete_elements();
    void cycle1();
    void cycle2();
};

// 単方向通信ができるチャネル
class channel : public element {
public:
    port& input;
    port& output;

    channel( port& input, port& output );
    channel( full_duplex_port& input, full_duplex_port& output );
    void cycle1();
    void cycle2();
};

#endif
