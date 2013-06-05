/*
 * wwwscan.h
 */

#define VERSION "0.1"
#define USER_AGENT "Baiduspider"
#define TEMPLATE "<html>\
                    <title>POOR MAN, POOR LIFE</title>\
                    <head>\
                        <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\
                        <style type=\"text/css\">\
                            headline\
                            {\
                                font-size: 24px;\
                                font-family: Monaco,Monospace,Consolas;\
                            }\
                            body\
                            {\
                                font-size: 12px;\
                                font-family: Monaco,Monospace,Consolas;\
                                line-height:200%%;\
                            }\
                            a:link { text-decoration: none;color: blue}\
                　　         a:active { text-decoration:blink}\
                　　         a:hover { text-decoration:underline;color: red}\
                　　         a:visited { text-decoration: none;color: green}\
                        </style>\
                    </head>\
                    <headline>\
                        <center>\
                            掃描報告(Scan Report)\
                        </center>\
                    </headline>\
                    <br>\
                    <body>\
                        %s\
                    </body>\
                </html>"

char body[1024*1024*1024];

typedef struct
{
    char *root;
    char *dir;
    char *custom404;
    int timeout;
}ARGS;
