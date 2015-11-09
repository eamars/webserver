#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

#include "client.h"
#include "datetime.h"
#include "parser.h"
#include "config.h"
#include "response.h"
#include "http_parser.h"
#include "cgi.h"


int execute_python(char *path, Configuration *config, Client *client)
{
    int                 child_to_parent[2];
    int                 parent_to_child[2];
    int                 pid;
    int                 status;
    int                 content_length = 0;

    // get content length
    for (int i = 0; i < client->header->num_fields; i++)
    {
        if (strcasecmp(client->header->fields[i], "Content-Length") == 0)
        {
            content_length = atoi(client->header->values[i]);
            break;
        }
    }

    // open pipe for executing cgi script
    if (pipe(child_to_parent) < 0 || pipe(parent_to_child) < 0)
    {
        handle_error("pipe");
    }


    pid = fork();
    if (pid < 0)
    {
        handle_error("fork");
    }
    else if (pid == 0)
    {
        // child code
        char method_env[MAX_QUERY_SZ];
        char query_env[MAX_QUERY_SZ];
        char length_env[MAX_QUERY_SZ];
        char dir_env[MAX_QUERY_SZ];
        char default_dir[MAX_VALUE_LEN];
        char exec_path[MAX_PATH_SZ];
        char *str = NULL;
        char *query = NULL;

        // extract post query
        if ((str = strstr(path, "?")) != NULL)
        {
            // terminate old string and create executable path
            query = str + 1;
            *str = '\0';
        }

        // get current working directory
        memset(default_dir, 0, MAX_VALUE_LEN);
        config_get_value(config, "default_dir", default_dir);

        // create new executable path
        sprintf(exec_path, "%s%s.py", default_dir, path);

        sprintf(method_env, "REQUEST_METHOD=%s", http_method_str(client->header->method));
        putenv(method_env);

        sprintf(query_env, "QUERY_STRING=%s", query);
        putenv(query_env);

        sprintf(length_env, "CONTENT_LENGTH=%d", content_length);
        putenv(length_env);

        sprintf(dir_env, "WORKING_DIR=%s", default_dir);
        putenv(dir_env);

        // debug print
        printf("EXEC_PATH: [%s]\nQUERY: [%s]\n", exec_path, query);

        // redirect pipe
        dup2(child_to_parent[1], STDOUT_FILENO);
        dup2(parent_to_child[0], STDIN_FILENO);
        close(child_to_parent[0]);
        close(parent_to_child[1]);

        execl(exec_path, exec_path, NULL);
        handle_error_en(errno, "execl");
    }
    else
    {
        // parent code
        char strbuf[READ_SZ];
        ssize_t sz;

        // close pipe
        close(child_to_parent[1]);
        close(parent_to_child[0]);

        // write form data to CGI
        if (client->header->method == 3) // only post will write data to it
        {
            write(parent_to_child[1], client->header->body, strlen(client->header->body));
        }


        memset(strbuf, 0, READ_SZ);
        while ((sz = read(child_to_parent[0], strbuf, READ_SZ)) != 0)
        {

            if (sz < 0)
            {
                perror("read");
                return -1;
            }

            // write to client
            write(client->msgsock, strbuf, sz);

            memset(strbuf, 0, READ_SZ);
        }

        // wait for child to complete
        waitpid(pid, &status, 0);

        // child has error occured
        if (WIFEXITED(status) && WEXITSTATUS(status)) {
            printf("STATUS: %d\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        }


        close(child_to_parent[0]);
        close(parent_to_child[1]);
        return 0;
    }
}
