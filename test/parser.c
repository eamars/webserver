#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include <http_parser.h>

const char *test_request_header =
"GET / HTTP/1.1\r\n"
"Host: www.zhihu.com\r\n"
"Connection: keep-alive\r\n"
"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
"Upgrade-Insecure-Requests: 1\r\n"
"User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/45.0.2454.85 Safari/537.36\r\n"
"Accept-Encoding: gzip, deflate, sdch\r\n"
"Accept-Language: en,zh-CN;q=0.8,zh;q=0.6\r\n"
"\r\n";

#define FIELD_SZ 1024
#define MAX_NUM_FIELD 10


typedef struct
{
    char url[FIELD_SZ];
    char body[FIELD_SZ];
    char field[MAX_NUM_FIELD][FIELD_SZ];
    char value[MAX_NUM_FIELD][FIELD_SZ];
    int num_fields;
    int num_values;
} http_header_t;


int http_url_cb(http_parser* p, const char *at, size_t length)
{
    char *url = ((http_header_t *) p->data)->url;
    memset(url, 0, FIELD_SZ);
    strncpy(url, at, length);
    return 0;
}

int http_body_cb(http_parser* p, const char *at, size_t length)
{
    char *body = ((http_header_t *) p->data)->body;
    memset(body, 0, FIELD_SZ);
    strncpy(body, at, length);
    return 0;
}

int http_header_field_cb(http_parser* p, const char *at, size_t length)
{
    static int i = 0;
    char *field = ((http_header_t *) p->data)->field[i];
    ((http_header_t *) p->data)->num_fields = i;
    memset(field, 0, FIELD_SZ);
    strncpy(field, at, length);
    i++;
    return 0;
}

int http_header_value_cb(http_parser* p, const char *at, size_t length)
{
    static int i = 0;
    char *value = ((http_header_t *) p->data)->value[i];
    ((http_header_t *) p->data)->num_values = i;
    memset(value, 0, FIELD_SZ);
    strncpy(value, at, length);
    i++;
    return 0;
}

int http_headers_complete_cb(http_parser* p)
{
    assert(((http_header_t *) p->data)->num_values == ((http_header_t *) p->data)->num_fields);
    return 0;
}

int http_message_begin_cb(http_parser* p)
{
    return 0;
}

int http_message_complete_cb(http_parser* p)
{
    return 0;
}

int main()
{
    http_parser *parser;
    size_t parsed;

    static http_parser_settings settings = {
        .on_url = http_url_cb,
        .on_body = http_body_cb,
        .on_header_field = http_header_field_cb,
        .on_header_value = http_header_value_cb,
        .on_headers_complete = http_headers_complete_cb,
        .on_message_begin = http_message_begin_cb,
        .on_message_complete = http_message_complete_cb
    };

    parser = (http_parser *) malloc (sizeof(struct http_parser));
    parser->data = (http_header_t *) malloc (sizeof(http_header_t));

    http_parser_init(parser, HTTP_REQUEST);
    parsed = http_parser_execute(parser, &settings, test_request_header, strlen(test_request_header));

    printf("%s\n", ((http_header_t *) parser->data)->url);
    printf("%s\n", ((http_header_t *) parser->data)->body);
    printf("%s\n", ((http_header_t *) parser->data)->field[1]);

    free(parser->data);
    free(parser);
    return 0;
}
