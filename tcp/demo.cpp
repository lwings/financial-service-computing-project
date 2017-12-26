//
// Created by ChiuPhonic on 2017/12/26.
//

#include <iostream>
#include "../public/Message.h"


int main()
{

    char buf[] = "S2MS.US   ";
    auto s0 = subMsgType::parse(buf);
    std::cout << s0.ticker << " -> " << strlen(s0.ticker) << std::endl;
    std::cout << s0.operation << " => " << strlen(buf) << std::endl;
    auto c0 = s0.str();
    std::cout << c0 << " <= " << strlen(c0) << std::endl;

    std::cout << std::endl;

    auto m0 = mktDataType();
    sprintf(m0.ticker, "BABA.US");
    sprintf(m0.exchange, "NYSE");
    m0.bid_px[0] = 3;
    m0.bid_sz[0] = 3;
    for (int i=0; i<3; ++i) {
        m0.bid_px[i+1] = i*500 + 50;
        m0.bid_sz[i+1] = i*70 + 15;
        std::cout << " >> bid: " << m0.bid_px[i+1] << " @ " << m0.bid_sz[i+1] << std::endl;
    }
    m0.ask_px[0] = 5;
    m0.ask_sz[0] = 5;
    for (int i=0; i<5; ++i) {
        m0.ask_px[i+1] = i*350+ 20;
        m0.ask_sz[i+1] = i*45 + 33;
        std::cout << " >> ask: " << m0.ask_px[i+1] << " @ " << m0.ask_sz[i+1] << std::endl;
    }
    auto mc0 = m0.str();
    std::cout << mc0 << " <= " << strlen(mc0) << std::endl;

    auto mdt = mktDataType::parse(mc0);
    for (int i=0; i<mdt.ask_px[0]; ++i) {
        std::cout << " >> ask: " << mdt.ask_px[i+1] << " @ " << mdt.ask_sz[i+1] << std::endl;
    }
    for (int i=0; i<mdt.bid_px[0]; ++i) {
        std::cout << " >> bid: " << mdt.bid_px[i+1] << " @ " << mdt.bid_sz[i+1] << std::endl;
    }
    std::cout << mdt.ticker << " -> " << strlen(mdt.ticker) << std::endl;
    std::cout << mdt.exchange << " -> " << strlen(mdt.exchange) << std::endl;

}