#include <iostream>
#ifndef SMARTROUTEORDER_ORDER_H
#define SMARTROUTEORDER_ORDER_H


class Order
{
public:
    char side;
    int qty;
    int priceInt;
    std::string ticker;
    std::string exch;

    Order(char s, int q, int p, std::string &t, std::string &e){
        side = s;
        qty = q;
        priceInt = p;
        ticker = t;
        exch = e;
    }

    Order(){
        side = '-';
        qty = 0;
        priceInt = 0;
        std::string t = "NONE";
        ticker = t;
        exch = t;
    }

    void ShowOrder()
    {
        std::cout<<side<<" "<<ticker<<" "<<qty<<"@"<<double(priceInt)/1000<<"  "<<exch<<std::endl;
    }
};

Order parse_command(std::string &str, int &success);
#endif //SMARTROUTEORDER_ORDER_H
