//
// Created by ChiuPhonic on 2017/11/15.
//
#include "ExchImpl.h"


int main()
{
    const string client_name = "test_client";
    const string ip = "127.0.0.1";
    const int port = 20003;
    const int listen_port = 6666;

    ExchImpl exch = ExchImpl(client_name, ip, port, listen_port);
    exch.run();
}
