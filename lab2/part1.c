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

int arr[1000];
void wait();

/**
 * @brief 得到Collatz猜想的结果序列
 * 
 * @param parm 待分析的数字N
 * @return int* 结果序列
 */
int *getSeq(int parm)
{
    int num = parm, i = 0;
    while (num != 1)
    {
        arr[i] = num;
        if (num % 2)
            num = 3 * num + 1;
        else
            num = num / 2;
        i++;
    }
    arr[i] = num;    // 记得将1加入序列
    arr[i + 1] = -1; // -1指示序列结尾
    return arr;
}

int main(int argc, char const *argv[])
{
    int n;
    scanf("%d", &n);
    pid_t pid;

    printf("fork() was called\n");
    pid = fork();

    if (pid == 0)
    {
        /* child process */
        printf("Child Process start\n");
        int *seq = getSeq(n);

        printf("Result is:\n");
        for (size_t i = 0; seq[i] != -1; i++)
            printf("%d ", seq[i]);
        printf("\n");

        printf("Child Process done\n");
    }
    else if (pid > 0)
    {
        /* parent process */
        printf("Parent Process waiting\n");
        wait(NULL);
        printf("Parent Process resume\n");

        printf("Parent Process done\n");
    }
    else
    {
        perror("fork error\n");
        return -1;
    }
    return 0;
}
