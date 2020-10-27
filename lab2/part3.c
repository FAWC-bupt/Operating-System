#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define STR_SIZE 1000

void wait();

int main(int argc, char const *argv[])
{
    pid_t pid;
    int pipe_1[2], pipe_2[2], ret;
    char msg[STR_SIZE], *newline, msg_from_pipe_child[STR_SIZE], msg_from_pipe_parent[STR_SIZE],
        msg_to_pipe_child[STR_SIZE], msg_to_pipe_parent[STR_SIZE];

    printf("Please type the sentence:\n");
    fgets(msg_to_pipe_parent, STR_SIZE, stdin);

    // 去除输入的换行符
    newline = strchr(msg_to_pipe_parent, '\n');
    if (newline)
        *newline = '\0';

    // pipe_1为父进程发，子进程收的管道
    ret = pipe(pipe_1);
    if (ret < 0)
    {
        perror("pipe error\n");
        return -1;
    }

    // pipe_2为子进程发，父进程收的管道
    ret = pipe(pipe_2);
    if (ret < 0)
    {
        perror("pipe error\n");
        return -1;
    }

    printf("fork() was called\n");
    pid = fork();

    if (pid == 0)
    {
        /* child process */
        size_t i;
        printf("Child Process start\n");

        close(pipe_1[1]); // 关闭不需要的pipe_1写入端
        close(pipe_2[0]); // 关闭不需要的pipe_2读取端

        ret = read(pipe_1[0], msg_from_pipe_child, sizeof(msg_from_pipe_child));
        if (ret < 0)
        {
            perror("read error\n");
            return -1;
        }
        printf("In Child: Reading from pipe 1 – Message is %s\n", msg_from_pipe_child);

        for (i = 0; msg_from_pipe_child[i] != '\0'; i++)
        {
            if (msg_from_pipe_child[i] >= 'a' && msg_from_pipe_child[i] <= 'z')
                msg_to_pipe_child[i] = (char)(msg_from_pipe_child[i] - 32);
            else if (msg_from_pipe_child[i] >= 'A' && msg_from_pipe_child[i] <= 'Z')
                msg_to_pipe_child[i] = (char)(msg_from_pipe_child[i] + 32);
            else
                msg_to_pipe_child[i] = msg_from_pipe_child[i];
        }
        msg_to_pipe_child[i] = '\0';

        printf("In Child: Writing to pipe 2 – Message is %s\n", msg_to_pipe_child);
        ret = write(pipe_2[1], msg_to_pipe_child, sizeof(msg_to_pipe_child));
        if (ret < 0)
        {
            perror("write error\n");
            return -1;
        }

        printf("Child Process done\n");
    }
    else if (pid > 0)
    {
        /* parent process */
        printf("Parent Process working\n");

        close(pipe_1[0]); // 关闭不需要的pipe_1读取端
        close(pipe_2[1]); // 关闭不需要的pipe_2写入端

        ret = write(pipe_1[1], msg_to_pipe_parent, sizeof(msg_to_pipe_parent));
        if (ret < 0)
        {
            perror("write error\n");
            return -1;
        }
        printf("In Parent: Writing to pipe 1 – Message is %s\n", msg_to_pipe_parent);

        // 等待子进程完成处理
        printf("Parent Process waiting\n");
        wait(NULL);
        printf("Parent Process resume\n");

        ret = read(pipe_2[0], msg_from_pipe_parent, sizeof(msg_from_pipe_parent));
        if (ret < 0)
        {
            perror("read error\n");
            return -1;
        }
        printf("In Parent: Reading from pipe 2 – Message is %s\n", msg_from_pipe_parent);
        printf("\nResult is:\n%s\n\n", msg_from_pipe_parent);

        printf("Parent Process done\n");
    }
    else
    {
        perror("fork error\n");
        return -1;
    }

    return 0;
}
