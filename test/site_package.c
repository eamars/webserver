#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "config.h"

#define MAX_PATH_SZ 256

static const char *usage = "webhttpd.out site_package_folder";


int main(int argc, char **argv)
{
    int rc;

    // test if the user enter the correct arguments
    if (argc != 2)
    {
        fprintf(stderr, "Invalid arguments\nUsage: %s\n", usage);
        return -1;
    }

    // try to read config file
    char config_path[MAX_PATH_SZ];
    FILE *fp;

    sprintf(config_path, "%s/site-config", argv[1]);
    printf("config_path: %s\n", config_path);

    // create config object
    Configuration *config;
    config = config_init();
    if (config == NULL)
    {
        fprintf(stderr, "Unable to allocate memory\n");
        return -1;
    }

    // load config from FILE
    rc = config_load(config, config_path);
    if (rc < 0)
    {
        fprintf(stderr, "Unable to read configuration file: %s\n", config_path);
        return -1;
    }

    // get value from config
    char value[MAX_VALUE_LEN];

    memset(value, 0, MAX_VALUE_LEN);
    rc = config_get_value(config, "server_name", value);
    if (rc)
    {
        printf("server_name: %s\n", value);
    }

    memset(value, 0, MAX_VALUE_LEN);
    rc = config_get_value(config, "server_port", value);
    if (rc)
    {
        printf("port: %d\n", atoi(value));
    }

    memset(value, 0, MAX_VALUE_LEN);
    rc = config_get_value(config, "default_index_page", value);
    if (rc)
    {
        printf("default_index_page: %s%s\n", argv[1], value);
    }

    memset(value, 0, MAX_VALUE_LEN);
    rc = config_get_value(config, "default_404_page", value);
    if (rc)
    {
        printf("default_404_page: %s%s\n", argv[1], value);
    }




    config_destroy(config);
    return 0;
}
