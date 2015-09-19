#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "parser.h"
#include <http_parser.h>

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
    ((http_header_t *) p->data)->method = p->method;

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

int parse(http_header_t *header, char *buf, int size)
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
    parser->data = header;

    http_parser_init(parser, HTTP_REQUEST);
    parsed = http_parser_execute(parser, &settings, buf, size);


    free(parser);
    return parsed;
}

const char *HTTP_404_CODE = "Not Found";
const char *HTTP_200_CODE = "OK";
const char *HTTP_501_CODE = "Method Not Implemented";
const char *HTTP_500_CODE = "Internal Server Error";
const char *HTTP_UNKNOWN_CODE = "Known";

const char *get_http_code_description(int code)
{
    switch (code)
    {
        case 200:
            return HTTP_200_CODE;
        case 404:
            return HTTP_404_CODE;
        case 500:
            return HTTP_500_CODE;
        case 501:
            return HTTP_501_CODE;
        default:
            return HTTP_UNKNOWN_CODE;
    }
}
