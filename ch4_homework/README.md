# 说明文档

| 姓名   | 学号       | 班级       |
| ------ | ---------- | ---------- |
| 凌国瀚 | 2018213344 | 2018211314 |

---

## 目录

<!-- TOC -->

> _1._ [题目要求](#题目要求)
>
> _2._ [开发环境与运行环境](#开发环境与运行环境)
>
> _3._ [使用说明](#使用说明)
>
> - [文件说明](#文件说明)
> - [运行说明](#运行说明)
>
> _4._ [运行结果及分析](#运行结果及分析)
>
> _5._ [源程序](#源程序)
>
> - [原版](#原版)
> - [非阻塞版](#非阻塞版)
>

<!-- /TOC -->

---

## 题目要求

Write a **multi-threaded** program that calculates various statistical values for a list of numbers.

This program will be passed a series of numbers on the command line and will then create three separate worker threads.

One thread will determine the **average** of the numbers, the second will determine the **maximum** value, and the third will determine the **minimum** value.

For example, suppose your program is passed the integers:

    90 81 78 95 79 72 85

The program will report:

    The average value is 82
    The minimum value is 72
    The maximum value is 95

The variables representing the average, minimum, and maximum values will be stored **globally**.

The worker threads will set these values, and the parent thread will output the values once the workers have exited.

---

## 开发环境与运行环境

_代码编辑器_：Visual Studio Code 15.0 with Remote Process Explorer

_操作系统_：Windows 10 (version: 2004)

_运行环境_：Windows Subsystem of Linux (Ubuntu-20.04 LTS)

_编译器_：GCC-9 on linux

---

## 使用说明

### 文件说明

    2018213344-凌国瀚-314班-第四章编程作业
    ├── thread.c
    ├── thread
    ├── thread_no_block.c
    ├── thread_no_block
    ├── README.md
    └── README.pdf

文件树如上所示，其中`thread.c`为本作业源代码，`thread`为可执行文件。

在作业要求外，额外加入了非阻塞版本的代码`thread_no_block.c`及其可执行文件`thread_no_block`，以作为对照样例说明程序运行结果

`README.md`为本说明文档，`README.pdf`是本文档的 pdf 版本，以便于在没有 Markdown 浏览条件时阅读本文档。

欲运行本程序，请参照[运行说明](#运行说明)。

### 运行说明

欲编译本程序并生成可执行文件，请在**源程序所在目录**的 Linux bash 中输入如下指令：

    gcc thread.c -o thread -lpthread

即可生成可执行文件`thread`。

> **温馨提示**
> 由于本程序调用 pthread 线程库，因此必须添加参数`-lpthread` 以实现库的动态链接

随后，直接使用以下指令即可运行可执行文件：

    ./thread

非阻塞版本程序的运行方式同上。

---

## 运行结果及分析

依照[运行说明](#运行说明)所述方式运行程序后，在控制台中输入数字`35`，得到结果如下：

    kevin_ling@Spear-of-Adun:~/Operating-System/ch4_homework$ ./thread
    35
    The average value is 35
    The minimum value is 35
    The maximum value is 35
    kevin_ling@Spear-of-Adun:~/Operating-System/ch4_homework$ 

下方流程图有助于理解本程序的运行过程

![avatar](https://github.com/FAWC-bupt/Operating-System/blob/main/ch4_homework/flow_chart.jpg)

从流程图可以看出，在多线程操作中，由于采用了pthread_join()函数，主线程将被阻塞，同一时间只有可能运行一个线程。一旦子线程结束，主线程就结束阻塞状态，输出子线程的结果并调用另一个子线程。因此，本程序的执行结果和顺序执行的代码类似，结果是确定的。

但是，当结主线程不加阻塞会发生什么呢？用指令`./thread_no_block`运行非阻塞版本的程序，结果如下。

    kevin_ling@Spear-of-Adun:~/Operating-System/ch4_homework$ ./thread_no_block
    35
    The average value is 0
    The minimum value is 0
    The maximum value is 0
    kevin_ling@Spear-of-Adun:~/Operating-System/ch4_homework$

可以看到，程序均输出3个全局变量的初始值0。这是因为在非阻塞条件下，主线程还没等待子线程完成运算任务就将结果输出。此时的变量就仍然还是初始化的值了。

## 源程序

### 原版

```c
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>

/*
    切记链接上 -lpthread 库
*/

int avg_num, max_num, min_num, len = 0; // 题目要求的3个全局变量
int pthread_kill(pthread_t thread, int sig);

/**
 * @brief Get the Avg object
 * 
 * @param param 
 * @return void* 
 */
void *getAvg(void *param)
{
    int *seq = (int *)param, sum = 0;

    for (size_t i = 0; i < len; i++)
        sum += seq[i];

    avg_num = sum / len;

    pthread_exit(0);
}

/**
 * @brief Get the Max object
 * 
 * @param param 
 * @return void* 
 */
void *getMax(void *param)
{ /* the thread */

    int *seq = (int *)param, max_temp = INT_MIN;

    for (size_t i = 0; i < len; i++)
        if (seq[i] > max_temp)
            max_temp = seq[i];

    max_num = max_temp;

    pthread_exit(0);
}

/**
 * @brief Get the Min object
 * 
 * @param param 
 * @return void* 
 */
void *getMin(void *param)
{ /* the thread */

    int *seq = (int *)param, min_temp = INT_MAX;

    for (size_t i = 0; i < len; i++)
        if (seq[i] < min_temp)
            min_temp = seq[i];

    min_num = min_temp;

    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    int arr[1024], kill_rc;
    char c = '0';

    while (c != '\n')
    {
        scanf("%d", &arr[len]);
        c = getchar();
        len++;
    }

    pthread_t tid1, tid2, tid3;
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // 线程1
    pthread_create(&tid1, &attr, getAvg, arr);
    pthread_join(tid1, NULL);
    /*
        pthread_kill()函数用于判断线程是否结束。
        这么做的原因是，题目要求必须在子线程结束后输出结果。
        尽管pthread_join()作用就是以阻塞的方式等待thread指定的线程结束，
        但是使用pthread_kill()能增加程序容错性和鲁棒性。
    */
    kill_rc = pthread_kill(tid1, 0);
    if (kill_rc == ESRCH)
        printf("The average value is %d\n", avg_num);
    else if (kill_rc == EINVAL)
        printf("signal is invalid\n");
    else
        printf("the thread 1 is still alive\n");

    // 线程2
    pthread_create(&tid2, &attr, getMin, arr);
    pthread_join(tid2, NULL);
    kill_rc = pthread_kill(tid2, 0);
    if (kill_rc == ESRCH)
        printf("The minimum value is %d\n", min_num);
    else if (kill_rc == EINVAL)
        printf("signal is invalid\n");
    else
        printf("the thread 2 is still alive\n");

    // 线程3
    pthread_create(&tid3, &attr, getMax, arr);
    pthread_join(tid3, NULL);
    kill_rc = pthread_kill(tid3, 0);
    if (kill_rc == ESRCH)
        printf("The maximum value is %d\n", max_num);
    else if (kill_rc == EINVAL)
        printf("signal is invalid\n");
    else
        printf("the thread 3 is still alive\n");
}
```

### 非阻塞版

```c
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>

/*
    切记链接上 -lpthread 库
*/

int avg_num, max_num, min_num, len = 0; // 题目要求的3个全局变量
int pthread_kill(pthread_t thread, int sig);

/**
 * @brief Get the Avg object
 * 
 * @param param 
 * @return void* 
 */
void *getAvg(void *param)
{
    int *seq = (int *)param, sum = 0;

    for (size_t i = 0; i < len; i++)
        sum += seq[i];

    avg_num = sum / len;

    pthread_exit(0);
}

/**
 * @brief Get the Max object
 * 
 * @param param 
 * @return void* 
 */
void *getMax(void *param)
{ /* the thread */

    int *seq = (int *)param, max_temp = INT_MIN;

    for (size_t i = 0; i < len; i++)
        if (seq[i] > max_temp)
            max_temp = seq[i];

    max_num = max_temp;

    pthread_exit(0);
}

/**
 * @brief Get the Min object
 * 
 * @param param 
 * @return void* 
 */
void *getMin(void *param)
{ /* the thread */

    int *seq = (int *)param, min_temp = INT_MAX;

    for (size_t i = 0; i < len; i++)
        if (seq[i] < min_temp)
            min_temp = seq[i];

    min_num = min_temp;

    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    int arr[1024], kill_rc;
    char c = '0';

    while (c != '\n')
    {
        scanf("%d", &arr[len]);
        c = getchar();
        len++;
    }

    pthread_t tid1, tid2, tid3;
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // 线程1
    pthread_create(&tid1, &attr, getAvg, arr);
    printf("The average value is %d\n", avg_num);

    // 线程2
    pthread_create(&tid2, &attr, getMin, arr);
    printf("The minimum value is %d\n", min_num);

    // 线程3
    pthread_create(&tid3, &attr, getMax, arr);
    printf("The maximum value is %d\n", max_num);
}
```
