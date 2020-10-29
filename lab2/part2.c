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

/*
    切记链接上 -lrt 库
*/

#define DATA_SIZE 2048

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
    int n, parent_fd, *map_get_data;
    scanf("%d", &n);
    pid_t pid;

    parent_fd = shm_open("OS", O_CREAT | O_RDWR, 0777);
    if (parent_fd < 0)
    {
        perror("shm_open failed!\n");
        return -1;
    }

    ftruncate(parent_fd, DATA_SIZE);

    printf("fork() was called\n");
    pid = fork();

    if (pid == 0)
    {
        printf("Child Process start\n");
        /* child process */
        int *seq = getSeq(n), len = 0, *map_data, child_fd;

        printf("In Child: get the sequence of Collatz Conjecture\n");

        do
        {
            len++;
        } while (seq[len] != -1);
        len++; // -1要记得传输

        child_fd = shm_open("OS", O_RDWR, 0777);
        map_data = (int *)mmap(NULL, DATA_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, child_fd, 0);
        memcpy(map_data, seq, sizeof(int) * len);

        printf("Child Process done\n");
        close(child_fd);
    }
    else if (pid > 0)
    {
        /* parent process */
        printf("Parent Process waiting\n");
        wait(NULL);
        printf("Parent Process resume\n");

        map_get_data = (int *)mmap(NULL, DATA_SIZE, PROT_READ, MAP_SHARED, parent_fd, 0);

        printf("Result is:\n");
        for (size_t i = 0; map_get_data[i] != -1; i++)
            printf("%d ", map_get_data[i]);
        printf("\n");

        close(parent_fd);
        shm_unlink("OS");
        printf("Parent Process done\n");
    }
    else
    {
        perror("fork error\n");
        return -1;
    }
    return 0;
}
