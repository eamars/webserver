#ifndef PARSER_H_
#define PARSER_H_

#define FIELD_SZ 2048
#define MAX_NUM_FIELD 16


typedef struct
{
    char url[FIELD_SZ];
    char body[FIELD_SZ];
    char field[MAX_NUM_FIELD][FIELD_SZ];
    char value[MAX_NUM_FIELD][FIELD_SZ];
    unsigned int method;
    int num_fields;
    int num_values;
} http_header_t;

int parse(http_header_t *header, char *buf, int size);
const char *get_http_code_description(int code);

#endif
