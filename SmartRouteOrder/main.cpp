#include <iostream>
#include<stdio.h>
#include<string.h>
#include "Order.h"
#include "OrderBook.h"

void fix_orders(Order* o, int const level, int const qty, int const curQty)
{
    if(curQty>=qty)
    {
        return;
    }
    double beta = double(qty)/curQty;
    int cum = 0;
    int beta_cum = 0;
    for(int i = 0;i<level;++i)
    {
        int s = (o+i)->qty;
        cum +=s;
        (o+i)->qty = int(cum*beta)-beta_cum;
        beta_cum+=(o+i)->qty;
    }
}

void ShowSmartOrder(Order* o, int const level)
{
    printf("%-6s%-10s%-10s%-10s%-10s\n","SIDE","TICKER","QUANTITY","PRICE","EXCHANGE");
    for(int i = 0;i<level;++i)
    {
        if((o+i)->exch=="NYSE")
        {
            printf("%-6c%-10s%-10d%-10.2f%-10s\n",(o+i)->side,(o+i)->ticker.c_str(),(o+i)->qty,double((o+i)->priceInt)/1000, (o+i)->exch.c_str());
        }
    }
    for(int i = 0;i<level;++i)
    {
        if((o+i)->exch=="NASDAQ")
        {
            printf("%-6c%-10s%-10d%-10.2f%-10s\n",(o+i)->side,(o+i)->ticker.c_str(),(o+i)->qty,double((o+i)->priceInt)/1000, (o+i)->exch.c_str());
        }
    }
    for(int i = 0;i<level;++i)
    {
        if((o+i)->exch=="IEX")
        {
            printf("%-6c%-10s%-10d%-10.2f%-10s\n",(o+i)->side,(o+i)->ticker.c_str(),(o+i)->qty,double((o+i)->priceInt)/1000, (o+i)->exch.c_str());
        }
    }
}

Order* find_orders(OrderBook const& ob, Order const& order, int& level)
{
    char side = order.side;
    int px = order.priceInt;
    int size = order.qty;
    int cumQty = 0;
    Order* ret;

    if(side=='B'){
        int bid_size = ob.bidlevel();
        ret = new Order[bid_size];
        for(int i = 0;i<bid_size;++i)
        {
            if(cumQty>=size || px<ob.bid[i].priceInt)
                break;
            if(cumQty + ob.bid[i].qty>size)
            {
                ret[i] = Order('b', size-cumQty,ob.bid[i].priceInt, ob.bid[i].ticker,ob.bid[i].exch);
                cumQty = size;
                level ++;
            }
            else if (cumQty + ob.bid[i].qty<=size)
            {
                ret[i] = Order('b', ob.bid[i].qty,ob.bid[i].priceInt, ob.bid[i].ticker,ob.bid[i].exch);
                cumQty += ob.bid[i].qty;
                level ++;
            }
        }
    }
    else{
        int ask_size = ob.asklevel();
        ret = new Order[ask_size];
        for(int i = ask_size-1;i>=0;--i)
        {
            if(cumQty>=size || px>ob.ask[i].priceInt)
                break;
            if(cumQty + ob.ask[i].qty>size)
            {
                ret[ask_size-1-i] = Order('s', size-cumQty,ob.ask[i].priceInt, ob.ask[i].ticker,ob.ask[i].exch);
                cumQty = size;
                level ++;
            }
            else if (cumQty + ob.ask[i].qty<=size)
            {
                ret[ask_size-1-i] = Order('s', ob.ask[i].qty,ob.ask[i].priceInt, ob.ask[i].ticker,ob.ask[i].exch);
                cumQty += ob.ask[i].qty;
                level ++;
            }
        }
    }
    fix_orders(ret, level, size, cumQty);
    return ret;
}

int main() {
    std::string command;
    std::string AllExch[3];
    AllExch[0] = "NYSE";
    AllExch[1] ="NASDAQ";
    AllExch[2] = "IEX";

    while(true)
    {
        char side;
        int qty;
        double price;
        std::string ticker;
        std::cin>>side;
        std::cin>>ticker;
        std::cin>>qty;
        std::cin>>price;
        std::string exch = "NONE";
        Order* o = new Order(side, qty, int(1000*price), ticker, exch);
        OrderBook* nyse = get_orderbook_by_exch(ticker, AllExch[0]);
        OrderBook* nq = get_orderbook_by_exch(ticker, AllExch[1]);
        OrderBook* iex = get_orderbook_by_exch(ticker, AllExch[2]);
        OrderBook* orderbook = intergrate_orderbooks(*nyse, *nq, *iex);

        nq->showall();

        int level;
        Order* result = find_orders(*orderbook, *o, level);
        ShowSmartOrder(result, level);

    }
    return 0;
}
