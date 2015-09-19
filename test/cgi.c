#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>

#define READ_SZ 4096

int main(void)
{
    signal(SIGCHLD, SIG_IGN);

    int input_fd[2];
    int output_fd[2];
    int pid;
    int status;


    // open pipe for executing cgi script
    if (pipe(input_fd) < 0 || pipe(output_fd) < 0)
    {
        perror("pipe");
    }

    pid = fork();
    if (pid < 0)
    {
        perror("fork");
    }
    else if (pid == 0)
    {
        // redirect pipe
        dup2(output_fd[1], STDOUT_FILENO);
        dup2(input_fd[0], STDIN_FILENO);
        close(output_fd[0]);
        close(input_fd[1]);


        execl("folder/color.cgi", "folder/color.cgi", NULL);
        perror("execl");
        exit(-1);
    }
    else
    {
        // parent code
        char strbuf[READ_SZ];
        int sz;
        char *output_buffer = NULL;
        size_t output_size = 0;

        // close pipe
        close(output_fd[1]);
        close(input_fd[0]);



        memset(strbuf, 0, READ_SZ);
        while ((sz = read(output_fd[0], strbuf, READ_SZ)) != 0)
        {
            if (sz < 0)
            {
                perror("read");
            }

            // append
    		output_buffer = realloc(output_buffer, output_size + sz);
    		memcpy(output_buffer + output_size, strbuf, sz);
    		output_size += sz;

            memset(strbuf, 0, READ_SZ);
        }
        close(output_fd[0]);
        close(input_fd[1]);

        printf("EXECL: %s\n", output_buffer);
        free(output_buffer);
    }

    return 0;
}
