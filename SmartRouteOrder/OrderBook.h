#include "Order.h"
#include <string.h>
#include <stdio.h>
#ifndef SMARTROUTEORDER_ORDERBOOK_H
#define SMARTROUTEORDER_ORDERBOOK_H

class OrderBook
{
private:
    int BIDLEVEL;
    int ASKLEVEL;
public:
    std::string Exch;
    std::string ticker;
    Order* bid;
    Order* ask;

    OrderBook(int blevel, int alevel, std::string &exch, std::string &t)
    {
        BIDLEVEL = blevel;
        ASKLEVEL = alevel;
        Exch = exch;
        ticker = t;
        bid = new Order[BIDLEVEL];
        ask = new Order[ASKLEVEL];
    }

    int bidlevel() const
    {
        return BIDLEVEL;
    }

    int asklevel() const
    {
        return ASKLEVEL;
    }

    void showall()
    {
        printf("%-6s%-10s%-10s%-10s%-10s|  %-6s%-10s%-10s%-10s%-10s\n","SIDE","TICKER","QUANTITY","PRICE","EXCHANGE","SIDE","TICKER","QUANTITY","PRICE","EXCHANGE");
        for(int i = asklevel()-1;i>=0;--i)
        {
            printf("%-46c|  %-6c%-10s%-10d%-10.2f%-10s\n",' ',ask[i].side,ask[i].ticker.c_str(),ask[i].qty,double(ask[i].priceInt)/1000, ask[i].exch.c_str());
        }
        for(int i = bidlevel()-1;i>=0;--i)
        {
            printf("%-6c%-10s%-10d%-10.2f%-10s|\n",bid[i].side,bid[i].ticker.c_str(),bid[i].qty,double(bid[i].priceInt)/1000, bid[i].exch.c_str());
        }
        printf("\n");

    }
};

OrderBook* get_orderbook_by_exch(std::string &ticker, std::string &exch);
OrderBook* intergrate_orderbooks(OrderBook &ob1, OrderBook &ob2, OrderBook &ob3);

#endif //SMARTROUTEORDER_ORDERBOOK_H
