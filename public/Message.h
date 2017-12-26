//
// Created by ChiuPhonic on 2017/12/27.
//

#ifndef FSC_MESSAGE_H
#define FSC_MESSAGE_H

typedef struct SubscribeMsg {//#'S'
    char    operation;
    char    ticker[8];
public:
    char* str() {
        auto *buf = new char[10];
        //buf[0] = 'S';
        //sprintf(buf, "S%c%-8s", operation, ticker);
        snprintf(buf, 10+1, "S%c%-8s", operation, ticker);
        return buf;
    }
    static SubscribeMsg parse(const char* buf) {
        auto res = SubscribeMsg();
        if (buf[0] == 'S' && strlen(buf) == 10) {
            int i=1;
            res.operation = buf[i];
            char tmp_ticker[8];
            strncpy(tmp_ticker, buf+2, 8);
            for (i=7; i>0; i--) {
                if (tmp_ticker[i] == ' ' && tmp_ticker[i-1] != ' ') {
                    tmp_ticker[i] = 0;
                    break;
                }
            }
            strncpy(res.ticker, tmp_ticker, size_t(i));
        }
        return res;
    }
} subMsgType;

typedef struct MarketData {//#'M'
    char    ticker[8];
    char    exchange[8];
    int     bid_px[6];
    int     bid_sz[6];
    int     ask_px[6];
    int     ask_sz[6];
public:
    char* str() {
        int bn = bid_px[0];
        int an = ask_px[0];
        if (bn > 5 || an > 5 || bn != bid_sz[0] || an != ask_sz[0]) {
            printf("[FATAL] null str..");
            return nullptr;
        }
        auto *buf = new char[19 + 16 * (bn + an)];
        //buf[0] = 'M';
        char head[17];
        //sprintf(head, "M%-8s%-8s", ticker, exchange);
        snprintf(head, 17+1, "M%-8s%-8s", ticker, exchange);
        strncpy(buf, head, 17);
        //buf[17] = '0';

        auto *bids = new char[1 + 16 * bn];
        bids[0] = char('0' + bn);
        //printf("[+bn][%d]\n[+bn][%c]\n", bn, bids[0]);
        for (int i=0; i<bn; ++i) {

            char item[16];
            snprintf(item, 16 + 1, "%08d%08d", bid_px[i + 1], bid_sz[i + 1]);
            strncpy(bids+1+i*16, item, 16);

        }
        strncpy(buf+17, bids, size_t(1+16*bn));
        //buf[18+16*bn] = '0';
        delete [] bids;

        auto *asks = new char[1 + 16 * an];
        asks[0] = char('0' + an);
        //printf("[+an][%d]\n[+an][%c]\n", an, asks[0]);
        for (int i=0; i<an; ++i) {

            char item[16];
            snprintf(item, 16 + 1, "%08d%08d", ask_px[i + 1], ask_sz[i + 1]);
            strncpy(asks+1+i*16, item, 16);

        }
        strncpy(buf+18+16*bn, asks, size_t(1+16*an));
        //buf[19+16*(bn+an)] = 0;
        delete [] asks;

        return buf;
    }
    static MarketData parse(const char* buf) {
        auto res = MarketData();
        size_t len = strlen(buf);
        if (buf[0] == 'M' && len > 17) {
            int tt=0;
            char tmp_ticker[8];
            strncpy(tmp_ticker, buf+1, 8);
            for (tt=7; tt>0; tt--) {
                if (tmp_ticker[tt] == ' ' && tmp_ticker[tt-1] != ' ') {
                    tmp_ticker[tt] = 0;
                    break;
                }
            }
            strncpy(res.ticker, tmp_ticker, size_t(tt));

            char tmp_exchange[8];
            strncpy(tmp_exchange, buf+9, 8);
            for (tt=7; tt>0; tt--) {
                if (tmp_exchange[tt] == ' ' && tmp_exchange[tt-1] != ' ') {
                    tmp_exchange[tt] = 0;
                    break;
                }
            }
            strncpy(res.exchange, tmp_exchange, size_t(tt));

            char bc = buf[17];
            int bn = atoi(&bc);
            if (len < 18+16*bn) {
                printf("[FATAL] not enough bids ..");
                return res;
            }
            res.bid_px[0] = bn;
            res.bid_sz[0] = bn;
            for (int i=0; i<bn; ++i) {
                char bpc[9];
                char bsc[9];
                strncpy(bpc, buf+18+i*16, 8); bpc[8] = 0;
                strncpy(bsc, buf+18+i*16+8, 8); bsc[8] = 0;
                res.bid_px[i+1] = atoi(bpc);
                res.bid_sz[i+1] = atoi(bsc);
            }

            char ac = buf[18+16*bn];
            int an = atoi(&ac);
            if (len < 19+16*(bn+an)) {
                printf("[FATAL] not enough asks ..");
                return res;
            }
            res.ask_px[0] = an;
            res.ask_sz[0] = an;
            for (int i=0; i<an; ++i) {
                char apc[9];
                char asc[9];
                strncpy(apc, buf+19+16*(bn+i), 8); apc[8] = 0;
                strncpy(asc, buf+19+16*(bn+i)+8, 8); asc[8] = 0;
                res.ask_px[i+1] = atoi(apc);
                res.ask_sz[i+1] = atoi(asc);
            }
        }
        return res;
    }
} mktDataType;

typedef struct OrderMsg {//#'O'
    char    side;
    char    type;
    int     price;
    int     size;
    char    ticker[8];
    char    exchange[8];
    char    oid[16];
public:
    char* str() {
        auto *buf = new char[51];
        snprintf(buf, 51+1, "O%-8s%-8s%c%c%08d%08d%-16s", ticker, exchange, side, type, price, size, oid);
        return buf;
    }
    static OrderMsg parse(const char* buf) {
        auto res = OrderMsg();
        if (buf[0] == 'O' && strlen(buf) == 51) {
            int tt=0;
            char tmp_ticker[8];
            strncpy(tmp_ticker, buf+1, 8);
            for (tt=7; tt>0; tt--) {
                if (tmp_ticker[tt] == ' ' && tmp_ticker[tt-1] != ' ') {
                    tmp_ticker[tt] = 0;
                    break;
                }
            }
            strncpy(res.ticker, tmp_ticker, size_t(tt));

            char tmp_exchange[8];
            strncpy(tmp_exchange, buf+9, 8);
            for (tt=7; tt>0; tt--) {
                if (tmp_exchange[tt] == ' ' && tmp_exchange[tt-1] != ' ') {
                    tmp_exchange[tt] = 0;
                    break;
                }
            }
            strncpy(res.exchange, tmp_exchange, size_t(tt));

            res.side = buf[17];
            res.type = buf[18];

            char px[9];
            char sz[9];
            strncpy(px, buf+19, 8); px[8] = 0;
            strncpy(sz, buf+27, 8); sz[8] = 0;
            res.price = atoi(px);
            res.size = atoi(sz);

            char tmp_oid[16];
            strncpy(tmp_oid, buf+35, 16);
            for (tt=15; tt>0; --tt) {
                if (tmp_oid[tt] == ' ' && tmp_oid[tt-1] != ' ') {
                    tmp_oid[tt] = 0;
                    break;
                }
            }
            strncpy(res.oid, tmp_oid, size_t(tt));
        }
        return res;
    }
} ordMsgType;

typedef struct ReportMsg {//#'R'
    char    side;
    char    status;
    int     price;
    int     size;
    char    ticker[8];
    char    exchange[8];
    char    oid[16];
public:
    char* str() {
        auto *buf = new char[51];
        snprintf(buf, 51+1, "R%-8s%-8s%c%c%08d%08d%-16s", ticker, exchange, side, status, price, size, oid);
        return buf;
    }
    static ReportMsg parse(const char* buf) {
        auto res = ReportMsg();
        if (buf[0] == 'R' && strlen(buf) == 51) {
            int tt=0;
            char tmp_ticker[8];
            strncpy(tmp_ticker, buf+1, 8);
            for (tt=7; tt>0; tt--) {
                if (tmp_ticker[tt] == ' ' && tmp_ticker[tt-1] != ' ') {
                    tmp_ticker[tt] = 0;
                    break;
                }
            }
            strncpy(res.ticker, tmp_ticker, size_t(tt));

            char tmp_exchange[8];
            strncpy(tmp_exchange, buf+9, 8);
            for (tt=7; tt>0; tt--) {
                if (tmp_exchange[tt] == ' ' && tmp_exchange[tt-1] != ' ') {
                    tmp_exchange[tt] = 0;
                    break;
                }
            }
            strncpy(res.exchange, tmp_exchange, size_t(tt));

            res.side = buf[17];
            res.status = buf[18];

            char px[9];
            char sz[9];
            strncpy(px, buf+19, 8); px[8] = 0;
            strncpy(sz, buf+27, 8); sz[8] = 0;
            res.price = atoi(px);
            res.size = atoi(sz);

            char tmp_oid[16];
            strncpy(tmp_oid, buf+35, 16);
            for (tt=15; tt>0; --tt) {
                if (tmp_oid[tt] == ' ' && tmp_oid[tt-1] != ' ') {
                    tmp_oid[tt] = 0;
                    break;
                }
            }
            strncpy(res.oid, tmp_oid, size_t(tt));
        }
        return res;
    }
} repMsgType;

#endif //FSC_MESSAGE_H

