/*
Copyright (C) 2012-2013 pi

This program is free software; you can redistribute it and / or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110 - 1301, USA.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <curl/curl.h>
#include <pthread.h>
#include "threadpool.h"
#include "wwwscan.h"


static void write_callback_func(void *buffer,
                                  size_t size,
                                  size_t nmemb,
                                  void *userp)
{
    char **response_ptr =  (char**)userp;
    *response_ptr = strndup(buffer, (size_t)(size *nmemb));
}


static long  task_request(char *url, char *custom404, long timeout)
{
    // 定义CURL类型的指针
    CURL *curl;
    // HTTP相应码
    long http_code = 0L;
    // HTTP相应体
    char *http_body = NULL;
    // 初始化一个CURL类型的指针
    curl = curl_easy_init();

    if (curl != NULL)
    {
        // 设置curl选项
        curl_easy_setopt(curl, CURLOPT_URL, url);
        // 跟踪302
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        // 设置User-Agent
        curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
        // 设置方法
        if (*custom404)
        {
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &http_body);
        }
        else
        {
             curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        }
        // 设置超时时间
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        // UNIX系设置timeout后必须设置NOSIGNAL
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        // 无输出
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_func);
        // 使支持https
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        // 调用curl_easy_perform 执行我们的设置
        curl_easy_perform(curl);
        // 获取HTTP响应码
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (*custom404 && http_body != NULL)
        {
            if (strstr(http_body, custom404))
            {
                http_code = 404L;
            }
        }
        //清除curl操作.
        curl_easy_cleanup(curl);
        return http_code;
    }
    else
    {
        // printf("init curl object error !!");
        return 0L;
    }
}


static void thread_task(void *ptr)
{
    char url[1024] = {'\0'};
    char *root = ((ARGS *)ptr)->root;
    char *dir = ((ARGS *)ptr)->dir;
    char *custom404 = ((ARGS *)ptr)->custom404;
    int timeout = ((ARGS *)ptr)->timeout;
    long status_code;

    printf("Checking    %20.20s  ...\r", dir);
    fflush(stdout);
    strncat(url, root, sizeof(url)/sizeof(url[0])-1);
    strncat(url, dir, sizeof(url)/sizeof(url[0])-1);
    status_code = task_request(url, custom404, timeout);

    if (status_code == 0L)
    {
        printf("%-80.80s    ERROR\n", url);
        fflush(stdout);
    }
    else if (status_code != 404L)
    {
        printf("%-80.80s    %ld\n", url, status_code);
        sprintf(body, "%s<a href = \"%s\"  target=_blank>%s</a> (HTTP/1.1 %ld)</br>", body, url, url, status_code);
        fflush(stdout);
    }
    else
    {
        printf("Checking    %20.20s        %3ld\r", dir, status_code);
        fflush(stdout);
    }

    free((ARGS *)ptr);
}


static ssize_t readline(char **lineptr, FILE *stream)
{
  size_t len = 0;

  ssize_t chars = getline(lineptr, &len, stream);

  while((*lineptr)[chars - 1] == '\n' || (*lineptr)[chars - 1] == '\r')
  {
    (*lineptr)[chars - 1] = '\0';
    --chars;
  }
  return chars;
}


static void signal_handler(int signo)
{
    printf("%-44s\n", "");
    _exit(0);
}


static int report_writer(char *content, char *reportfilename)
{
    FILE *fp;
    fp = fopen(reportfilename, "w");
    if (fp == NULL)
    {
        printf("Error: 打開 %s 出錯\n", reportfilename);
        return -1;
    }
    fputs(content, fp);
    fclose(fp);
    return 0;
}


static void head_request(char *url, long timeout)
{
    // 定义CURL类型的指针
    CURL *curl;
    // 初始化一个CURL类型的指针
    curl = curl_easy_init();

    if (curl != NULL)
    {
        // 设置curl选项
        curl_easy_setopt(curl, CURLOPT_URL, url);
        // 跟踪302
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        // 设置User-Agent
        curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
        // 设置方法
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
        // 设置超时时间
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        // UNIX系设置timeout后必须设置NOSIGNAL
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        // 无输出
        // curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_func);
        // 使支持https
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        // 调用curl_easy_perform 执行我们的设置
        printf("%40s\r", "");
        curl_easy_perform(curl);
        // 清除curl操作.
        curl_easy_cleanup(curl);
    }
}


static void usage(char* filename)
{
    printf("wwwscan %s\n", VERSION);
    printf("用法: %s [參數]\n", filename);
    printf("參數:\n");
    printf("   -u <url>         URL(http/https)\n");
    printf("   -w <path>        wordlist路徑\n");
    printf("   -t <timeout>     HTTP超時時間(默認:20)\n");
    printf("   -m <threadnum>   最大線程數目(默認:5)\n");
    printf("   -c <string>      自定義404(暫僅支持ASCII字符串)\n");
    printf("   -o <path>        報告文件路徑(格式HTML)\n");
    printf("\n");
}


int main(int argc, char *argv[])
{
    if(argc <= 2)
    {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    int ch;
    char *url = NULL;
    char *wordlist = NULL;
    char *outputfile = NULL;
    char *custom404 = "";
    long timeout = 20L;
    int threadnum = 5;

    while ((ch = getopt(argc, argv, "u:w:t:m:c:o:")) != -1)
    {
        switch (ch)
        {
        case 'u':
            url = optarg;
            break;

        case 'w':
            wordlist = optarg;
            break;

        case 't':
            timeout = atol(optarg);
            break;

        case 'm':
            threadnum = atoi(optarg);
            break;

        case 'c':
            custom404 = optarg;
            break;

        case 'o':
            outputfile = optarg;
            break;

        case 'h':
        case '\0':
        case '?':
            usage(argv[0]);
            return 1;
            break;
        }
    }
    if (*url == '\0')
    {
        printf("Error: 缺少 -u 參數\n");
        exit(EXIT_FAILURE);
    }
    if (*wordlist == '\0')
    {
        printf("Error: 缺少 -w 參數\n");
        exit(EXIT_FAILURE);
    }
    if (access(wordlist, 0) != 0)
    {
        printf("Error: %s 不存在\n", wordlist);
        exit(EXIT_FAILURE);
    }
    if (!(outputfile))
    {
        outputfile = "REPORT.html";
    }

    FILE *fp;
    char *line = NULL;
    ssize_t read;

    fp = fopen(wordlist, "r");
    if (fp == NULL)
    {
        printf("Error: 打開 %s 出錯\n", wordlist);
    }

    // 創建線程池，開始工作
    threadpool *pool;
    pool = create_threadpool(threadnum);
    signal(SIGINT, signal_handler);

    printf("initializing ...\r");
    fflush(stdout);
    head_request(url, timeout);

    while ((read = readline(&line, fp)) != -1)
    {
        ARGS *args = NULL;
        if (NULL == (args=(ARGS *)malloc(sizeof(ARGS))))
        {
            printf("malloc for ARGS failed");
            exit(EXIT_FAILURE);
        }
        args->custom404 = custom404;
        args->timeout = timeout;
        args->root = url;
        args->dir = line;
        dispatch_threadpool(pool, thread_task, (void *)args);
    }

    char *html = (char *)malloc(1024*1024*1024);
    if (NULL == html)
    {
        printf("malloc for HTML failed");
        exit(EXIT_FAILURE);
    }
    sprintf(html, TEMPLATE, body);
    report_writer(html, outputfile);
    free(html);

    destroy_threadpool(pool);
    printf("%-44s\n", "");
    return 0;
}



