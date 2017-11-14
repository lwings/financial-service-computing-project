//
// Created by ChiuPhonic on 2017/11/14.
//

#ifndef FSC_MESSAGE_H
#define FSC_MESSAGE_H

typedef struct MarketData{
    int     operation;
    char    clientName[1024] ;
    char    stockName[1024] ;
    int     price[20];
    int     num[20];
} mktDataType;

typedef struct ClientMessage{
    int     operation;
    char    stockName[1024] ;
    int     num;
} clMsgType;


#endif //FSC_MESSAGE_H

