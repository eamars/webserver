/*
 * Simple library for handling the config files.
 * The format of cfg file is as follow
 *
 * settings.cfg:
 * # This is a comment
 * key=value
 * key1=value1 #comment can also be here
 *
 */

#ifndef CONFIG_H
#define CONFIG_H

/*
 * Load config from files
 * @param cname : config file name
 * @return 		: 0 for successfully load
 * 				: negative for failed to load
 */
int config_open(char *cname);

/*
 * Always unload the resource when finish loading configs
 */
void config_close();

/*
 * Get the value of corresponding key
 * @param key 	: input key for searching
 * @param value : return value
 * @return		: 1 for found
 * 				: 0 for not found
 */
int config_get_value(char *key, char *value);


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
