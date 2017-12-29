//
// Created by mac on 2017/12/26.
//

#include <string>
#include <map>
#include <unordered_set>

#ifndef FSC_ADVORDERBOOK_H
#define FSC_ADVORDERBOOK_H

class OrderInfo {
public:
    char side;
    std::string exch;
    std::string ticker;
    int curQty;
    int pendingRepQty;
    int intPrice;
    int fillQty;
    bool pendingNew;
    bool pendingCancel;

    OrderInfo(std::string & e,std::string & t, char s, int ordQty, int price)
    {
        exch = e;
        ticker = t;
        side = s;
        curQty = ordQty;
        intPrice = price;
        fillQty = 0;
        pendingRepQty = 0;
        pendingNew = true;
        pendingCancel = false;
    }

    bool isPendingReplace() const { return pendingRepQty > 0; }
};

typedef std::shared_ptr<OrderInfo> OrderInfoPtr;

class AdvOrderBook
{

    std::map<std::string, OrderInfoPtr> *oidMap;

public:
    std::string ticker;
    std::map<int, std::unordered_set<std::string>, std::greater<int>> bidMap;
    std::map<int, std::unordered_set<std::string> > askMap;
    AdvOrderBook(std::string &t, std::map<std::string, OrderInfoPtr> &m)
    {
        ticker = t;
        oidMap = &m;
    }
    void addNewOrder(std::string &exch, std::string &oid, char side, int qty, int px);
    void removeOrder(const std::string &oid, int px, char side);
    void showAll();
};

typedef std::shared_ptr<AdvOrderBook> AdvOrderBookPtr;



class AdvExchange
{
public:
    std::map<std::string, AdvOrderBookPtr> books;
    std::string exchName;
    std::map<std::string, OrderInfoPtr> byOid;
    AdvExchange(std::string &exch)
    {
        exchName = exch;
    }

    void addNewOrder(std::string &ticker, std::string &oid, char side, int qty, int px)
    {
        auto o = byOid.find(oid);
        if(o != byOid.end())
        {
            printf("%s already existed\n", oid.c_str());
            return;
        }
        auto newoid = std::make_shared<OrderInfo>(exchName, ticker, side, qty, px);
        byOid[oid] = newoid;
        auto iter = books.find(ticker);
        if(iter == books.end()) {
            auto ob = std::make_shared<AdvOrderBook>(ticker, byOid);
            books[ticker] = ob;
            iter = books.find(ticker);
        }
        iter->second->addNewOrder(exchName, oid, side, qty, px);

    }

    void removeOrder(std::string &oid)
    {
        auto iter = byOid.find(oid);
        if(iter == byOid.end())
        {
            printf("OrderID %s not exit!\n", oid.c_str());
            return;
        }
        auto info = iter->second;
        auto book = books.find(info->ticker);
        if(book == books.end())
        {
            printf("No ticker %s in Order Book\n", info->ticker.c_str());
            return;
        }
        book->second->removeOrder(oid, info->intPrice, info->side);

    }

    void onNewOrder(std::string &ticker, std::string &oid, char side, int qty, int px);

    void showAll()
    {
        for(auto iter = books.begin();iter!=books.end();iter++)
        {
            printf("Ticker: %s\n", iter->first.c_str());
            iter->second->showAll();
        }
    }



};


#endif //FSC_ADVORDERBOOK_H
