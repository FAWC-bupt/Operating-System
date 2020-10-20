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
    printf("PARENT Process call fork()\n");
    pid = fork();
    if (pid == 0)
    {
        printf("CHILD Process start\n");
        /* child process */
        int *seq = getSeq(n);

        printf("Result is:\n");
        for (size_t i = 0; seq[i] != -1; i++)
            printf("%d ", seq[i]);
        printf("\n");

        printf("CHILD Process done\n");
    }
    else if (pid > 0)
    {
        /* parent process */
        wait(NULL);
        printf("PARENT Process done\n");
    }
    return 0;
}
