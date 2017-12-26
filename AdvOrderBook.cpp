//
// Created by mac on 2017/12/26.
//

#include "AdvOrderBook.h"
#include <iostream>

void AdvOrderBook::addNewOrder(std::string &exch, std::string &oid, char side, int qty, int px)
{
    auto info = std::make_shared<OrderInfo>(exch,ticker, side, qty, px);
    (*oidMap)[oid] = info;
    if (side == 'B'){
        bidMap[px].insert(oid);
    }
    else{
        askMap[px].insert(oid);
    }
    //std::cout<<"add new order price is "<<px<<std::endl;
}

void AdvOrderBook::removeOrder(const std::string &oid, int px, char side)
{
    if (side == 'B'){
        auto lv = bidMap.find(px);
        if(lv == bidMap.end())
        {
            printf("No PriceLevel at %.2f\n", px/float(1000));
            return;
        }
        auto order = lv->second.find(oid);
        if(order == lv->second.end())
        {
            printf("No OrderID %s\n", oid.c_str());
            return;
        }
        lv->second.erase(oid);
        if(lv->second.empty())
        {
            bidMap.erase(px);
        }
    }
    else{
        auto lv = askMap.find(px);
        if(lv == askMap.end())
        {
            printf("No PriceLevel at %.2f\n", px/float(1000));
            return;
        }
        auto order = lv->second.find(oid);
        if(order == lv->second.end())
        {
            printf("No OrderID %s\n", oid.c_str());
            return;
        }
        lv->second.erase(oid);
        if(lv->second.empty())
        {
            askMap.erase(px);
        }
    }
}

void AdvOrderBook::showAll()
{
    for(auto iter=bidMap.begin();iter!=bidMap.end();++iter)
    {
        auto tmp = iter->second;
        for(auto iter2=tmp.begin();iter2!=tmp.end();iter2++)
        {
            printf("BID %d at %.2f\n",(*oidMap)[*iter2]->curQty, (*oidMap)[*iter2]->intPrice/float(1000));
        }
    }
    for(auto iter=askMap.begin();iter!=askMap.end();++iter)
    {
        auto tmp = iter->second;
        for(auto iter2=tmp.begin();iter2!=tmp.end();iter2++)
        {
            printf("ASK %d at %.2f\n",(*oidMap)[*iter2]->curQty, (*oidMap)[*iter2]->intPrice/float(1000));
        }
    }
}

void AdvExchange::onNewOrder(std::string &ticker, std::string &oid, char side, int qty, int px)
{
    auto o = byOid.find(oid);
    if(o != byOid.end())
    {
        printf("OrderID %s already existed\n", oid.c_str());
        //TODO Send Reject
        return;
    }
    //TODO send new order back to client
    //auto newOrder = std::make_shared<OrderInfo>(exchName, ticker, side, qty, px);
    //byOid[oid] = newOrder;
    auto iter = books.find(ticker);
    if(iter == books.end()) {       //First Order of this Ticker
        auto ob = std::make_shared<AdvOrderBook>(ticker, byOid);
        books[ticker] = ob;
        ob->addNewOrder(exchName, oid, side, qty, px);

    }
    else {           //Order already exist
        auto ob = iter->second;
        if (side == 'B') {
            if (ob->askMap.empty())      //Ask Map empty, no trade
            {
                ob->addNewOrder(exchName, oid, side, qty, px);
                return;
            }
            if (ob->askMap.begin()->first > px)    //Ask > bid, no trade
            {
                ob->addNewOrder(exchName, oid, side, qty, px);
                return;
            }
            int leftQty = qty;
            while (leftQty > 0) {
                if (ob->askMap.empty() || ob->askMap.begin()->first > px) {
                    ob->addNewOrder(exchName, oid, side, leftQty, px);          //stop fill trades
                    return;
                }
                auto lv = ob->askMap.begin()->second;
                auto firstOid = byOid[*lv.begin()];
                if (firstOid->curQty > leftQty) {
                    //TODO fill order (leftQty)
                    //TODO partial fill order (leftQty)
                    firstOid->curQty -= leftQty;
                    leftQty = 0;
                } else {
                    if (firstOid->curQty == leftQty) {
                        //TODO fill order (leftQty)
                        //TODO fill order (leftQty)
                        ob->removeOrder(*lv.begin(), firstOid->intPrice, firstOid->side);
                        leftQty = 0;
                    } else {
                        //TODO partial fill order (firstOid->curQty)
                        //TODO fill order (leftQty)
                        ob->removeOrder(*lv.begin(), firstOid->intPrice, firstOid->side);
                        leftQty -= firstOid->curQty;
                    }
                }
            }
        } else {
            if (ob->bidMap.empty())      //Ask Map empty, no trade
            {
                ob->addNewOrder(exchName, oid, side, qty, px);
                return;
            }
            if (ob->bidMap.begin()->first < px)    //Ask > bid, no trade
            {
                ob->addNewOrder(exchName, oid, side, qty, px);
                return;
            }
            int leftQty = qty;
            while (leftQty > 0) {
                if (ob->bidMap.empty() || ob->bidMap.begin()->first < px) {
                    ob->addNewOrder(exchName, oid, side, leftQty, px);          //stop fill trades
                    return;
                }
                auto lv = ob->bidMap.begin()->second;
                auto firstOid = byOid[*lv.begin()];
                if (firstOid->curQty > leftQty) {
                    //TODO fill order (leftQty)
                    //TODO partial fill order (leftQty)
                    firstOid->curQty -= leftQty;
                    leftQty = 0;
                } else {
                    if (firstOid->curQty == leftQty) {
                        //TODO fill order (leftQty)
                        //TODO fill order (leftQty)
                        ob->removeOrder(*lv.begin(), firstOid->intPrice, firstOid->side);
                        leftQty = 0;
                    } else {
                        //TODO partial fill order (firstOid->curQty)
                        //TODO fill order (leftQty)
                        ob->removeOrder(*lv.begin(), firstOid->intPrice, firstOid->side);
                        leftQty -= firstOid->curQty;
                    }
                }
            }
        }
    }

}
