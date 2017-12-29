//
// Created by mac on 2017/12/26.
//
#include <iostream>
#include "AdvOrderBook.h"
#include <string>

int main()
{
    std::cout<<"HelloWorld"<<std::endl;
    std::string ex = "NYSE";
    AdvExchange *exch = new AdvExchange(ex);

    int i = 0;
    char s;
    std::string ticker;
    int qty;
    double price;
    while(true)
    {
        i++;
        std::cin>>s>>ticker>>qty>>price;
        std::string oid = std::to_string(i)+ticker;
        int intPrice = int(1000*price);
        exch->onNewOrder(ticker, oid, s, qty, intPrice);
        exch->showAll();
    }

    return 0;
}