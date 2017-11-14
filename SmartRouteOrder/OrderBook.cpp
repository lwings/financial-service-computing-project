#include "OrderBook.h"
OrderBook * get_orderbook_by_exch(std::string &ticker, std::string &exch)
{
    int MAXLEVEL = 10;
    OrderBook *ob = new OrderBook(MAXLEVEL, MAXLEVEL, exch, ticker);
    for(int i = 0;i<MAXLEVEL;++i)
    {
        ob->bid[MAXLEVEL-1-i]=Order('b', 100*(MAXLEVEL-i),0, 19950-10*i,ticker, exch);
        ob->ask[i]=Order('s', 101*(MAXLEVEL-i), 0, 20100+10*i,ticker, exch);
    }
    return ob;
}

OrderBook* intergrate_orderbooks(OrderBook &ob1, OrderBook &ob2, OrderBook &ob3)
{
    int blevel = ob1.bidlevel()+ob2.bidlevel()+ob3.bidlevel();
    int alevel = ob1.asklevel()+ob2.asklevel()+ob3.asklevel();
    std::string exch = "ALL";
    std::string ticker = ob1.ticker;
    OrderBook* ob = new OrderBook(blevel,alevel,exch,ticker);
    int z = 0;
    for(int i = 0,j=0,k=0;i<ob1.bidlevel() || j<ob2.bidlevel() || k<ob3.bidlevel();)
    {
        int p1 = i<ob1.bidlevel()?ob1.bid[i].priceInt:10000000;
        int p2 = j<ob2.bidlevel()?ob2.bid[j].priceInt:10000000;
        int p3 = k<ob3.bidlevel()?ob3.bid[k].priceInt:10000000;
        Order o;
        if(p1<=p2 && p1<=p3)
        {
            o = ob1.bid[i];
            i++;
        }
        else if(p2<=p3 && p2<=p1){
            o = ob2.bid[j];
            j++;
        }
        else if(p3<=p1 && p3<=p2) {
            o = ob3.bid[k];
            k++;
        }
        ob->bid[z] = o;
        z++;
    }
    z = 0;
    for(int i = 0,j=0,k=0;i<ob1.asklevel() || j<ob2.asklevel() || k<ob3.asklevel();)
    {
        int p1 = i<ob1.asklevel()?ob1.ask[i].priceInt:10000000;
        int p2 = j<ob2.asklevel()?ob2.ask[j].priceInt:10000000;
        int p3 = k<ob3.asklevel()?ob3.ask[k].priceInt:10000000;
        Order o;
        if(p1<=p2 && p1<=p3)
        {
            o = ob1.ask[i];
            i++;
        }
        else if(p2<=p3 && p2<=p1){
            o = ob2.ask[j];
            j++;
        }
        else if(p3<=p1 && p3<=p2) {
            o = ob3.ask[k];
            k++;
        }
        ob->ask[z] = o;
        z++;
    }
    return ob;
}

