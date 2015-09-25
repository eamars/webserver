/*
 * Simple library for handling the config files.
 * The format of cfg file is as follow
 *
 * settings.cfg:
 * # This is a comment
 * key=value
 * key1=value1 #comment can also be here
 *
 * Version 1.1/webhttpd
 */

#ifndef CONFIG_H
#define CONFIG_H

#define MAX_KEY_LEN 512
#define MAX_VALUE_LEN 512

// abstract data type for configuration class
typedef struct configuration_ini_style Configuration;

// Allocate memory for Configuration class
Configuration *config_init(char *path);

// Free memory for Configuration class
void config_destroy(Configuration *config);

/*
 * Load config from files
 * @param cname : config file name
 * @return 		: 0 for successfully load
 * 				: negative for failed to load
 */
int config_load(Configuration *config);

/*
 * Always unload the resource when finish loading configs
 */

/*
 * Get the value of corresponding key
 * @param key 	: input key for searching
 * @param value : return value
 * @return		: 1 for found
 * 				: 0 for not found
 */
int config_get_value(Configuration *config, char *key, char *value);
const char *config_get_path(Configuration *config);

/**
 * Creates a list of strings splited by given token. The list termianted by NULL
 * @param  src 	 source string
 * @param  token the char that splits the string
 * @return       pointer to a list
 */
char **split_create(const char *src, char token);

/**
 * Destory the list created by split_create
 * @param llist input list
 */
void split_destory(char **list);

#endif
