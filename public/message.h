//
// Created by ChiuPhonic on 2017/11/14.
//

#ifndef FSC_MESSAGE_H
#define FSC_MESSAGE_H

typedef struct clientMessage{
    int operation;
    char clientName[1024] ;
    char stockName[1024] ;
    int price;
    int num;
} cmsgType;

typedef struct serverMessage{
    int operation;
    char stockName[1024] ;
    int  num;
} smsgType;


#endif //FSC_MESSAGE_H
