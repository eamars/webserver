#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define READ_SZ 10240

int main(void)
{
    int                 input_fd[2];
    int                 output_fd[2];
    int                 status;
    int                 pid;
    char                *path = "site-package/index3.py";

    // open pipe for executing cgi script
    if (pipe(input_fd) < 0 || pipe(output_fd) < 0)
    {
        perror("pipe");
        return 501;
    }

    pid = fork();
    if (pid < 0)
    {
        perror("fork");
        return 501;
    }
    else if (pid == 0)
    {
        // redirect pipe
        dup2(output_fd[1], STDOUT_FILENO);
        dup2(input_fd[0], STDIN_FILENO);
        close(output_fd[0]);
        close(input_fd[1]);


        execl(path, path, NULL);
        perror("execl");
        exit(errno);
    }
    else
    {
        // parent code
        char strbuf[READ_SZ];
        ssize_t sz;

        // close pipe
        close(output_fd[1]);
        close(input_fd[0]);


        memset(strbuf, 0, READ_SZ);
        while ((sz = read(output_fd[0], strbuf, READ_SZ)) != 0)
        {
            if (sz < 0)
            {
                perror("read");
                return 501;
            }

            // write to client
            printf("OUTPUT: %s\n", strbuf);
            memset(strbuf, 0, READ_SZ);
        }

        // wait for child to complete
        waitpid(pid, &status, 0);

        // child has error occured
        if (WIFEXITED(status) && WEXITSTATUS(status)) {
            printf("STATUS: %d\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        }


        close(output_fd[0]);
        close(input_fd[1]);
        return 0;
    }
    return 0;
}
