#include <exception>
#include <fstream>
#include <iostream>
#include <queue>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include <mutex>
#include <semaphore.h>
#include <thread>

// 需要动态链接pthread库，即需要 -lpthread

using namespace std;

/**
 * @brief 根据实验要求所设置的线程数据结构
 * 
 */
struct ThreadInfo
{
    int index;        // 序号
    bool isReader;    // 是否是读者
    double startTime; // 开始时间（单位：秒）
    double duration;  // 运行持续时间（单位：秒）
};

mutex mutexForJob; // 线程任务中队列操作的互斥锁，防止队列操作或输出操作发送冲突
char curTime[100];
queue<int> randomIntQueue;

ofstream outputFile("thread.out");

/**
 * @brief 重载输出流运算符，将threadInfo作为参数
 * 
 * @param os 输出流
 * @param threadInfo 线程信息 
 * @return ostream& 更新后的输出流
 */
ostream &operator<<(ostream &os, const ThreadInfo &threadInfo)
{
    return os << "线程 " << threadInfo.index << (threadInfo.isReader ? " (Reader)： " : " (Writer)： ");
}

/**
 * @brief 获取当前时间
 * 
 */
void getTime()
{
    time_t timer;
    struct tm *tblock;

    time(&timer);
    tblock = gmtime(&timer);
    sprintf(curTime, " %d-%d-%d %2d:%2d:%2d ", tblock->tm_year + 1900, tblock->tm_mon + 1, tblock->tm_mday,
            tblock->tm_hour + 8, tblock->tm_min, tblock->tm_sec);
}

/**
 * @brief 该函数用于每个线程开始时（进入区），先休眠至其开始的时间，以模拟题目要求的多线程场景
 * 
 * @param threadInfo 线程的四个属性信息
 */
void job_request(const ThreadInfo &threadInfo)
{
    int timeToStart = (int)(threadInfo.startTime * 1000);
    this_thread::sleep_for(chrono::milliseconds(timeToStart));

    {
        lock_guard<mutex> mutex_lg(mutexForJob); // C++11引入的自动互斥锁
        getTime();
        cout << threadInfo << "请求运行 - " << curTime << endl;
        outputFile << threadInfo << "请求运行 - " << curTime << endl;
    };
}

/**
 * @brief 该函数用于每个线程的临界区中，该函数将根据线程属性
 * 
 * @param threadInfo 线程的四个属性信息
 */
void job_running(const ThreadInfo &threadInfo)
{
    int queueNum;

    {
        lock_guard<mutex> mutex_lg(mutexForJob);
        getTime();
        cout << threadInfo << "开始运行 - " << curTime << endl;
        outputFile << threadInfo << "开始运行 - " << curTime << endl;
    };

    int runningTime = (int)(threadInfo.duration * 1000);
    this_thread::sleep_for(chrono::milliseconds(runningTime));

    {
        lock_guard<mutex> mutex_lg(mutexForJob);
        getTime();
        cout << threadInfo << "运行结束 - " << curTime;
        outputFile << threadInfo << "运行结束 - " << curTime;
        if (threadInfo.isReader)
        {
            if (!randomIntQueue.empty())
            {
                queueNum = randomIntQueue.front();
                randomIntQueue.pop();
                cout << " 获得随机数" << queueNum << endl;
                outputFile << " 获得随机数" << queueNum << endl;
            }
            else
            {
                cout << " 无法从空队列获得随机数" << endl;
                outputFile << " 无法从空队列获得随机数" << endl;
            }
        }
        else
        {
            srand((unsigned)time(0));
            queueNum = rand() % 100;
            randomIntQueue.push(queueNum);
            cout << " 生成随机数" << queueNum << endl;
            outputFile << " 生成随机数" << queueNum << endl;
        }
    };
}

int main(int argc, char const *argv[])
{
    try
    {
        if (argc < 2)
            throw runtime_error(
                "\nHow to use: ./thread <Input File Name> <Lines in Input File (optional)>\n"
                "Every line in input file should be like: <index> <Role (R/W)> <Start Time> <Duration>\n");

        ifstream inputFile(argv[1]);
        if (!inputFile.is_open())
            throw runtime_error(string("\nCan not open file: ") + argv[1] + string(", please check it out.\n"));

        // 从文件读取数据并加载至vector容器
        vector<ThreadInfo> threadInfo_vector;
        char buf[64];
        while (inputFile.getline(buf, 64))
        {
            ThreadInfo tempInfo;
            char role;
            // 利用C++的stringstream格式化输入数据
            stringstream ss(buf);
            ss >> tempInfo.index >> role >> tempInfo.startTime >> tempInfo.duration;
            if (role == 'R' || role == 'r')
                tempInfo.isReader = true;
            else if (role == 'W' || role == 'w')
                tempInfo.isReader = false;
            else
                throw runtime_error("\nThe \"role\" char is invalid, please check it out.\n");

            threadInfo_vector.emplace_back(tempInfo);
        }

        /*
            利用匿名函数实现读者优先功能
        */
        auto readerPreference = [&]() {
            int readCount = 0;
            sem_t semForCount, semForWrite;
            sem_init(&semForCount, 0, 1); // 初始化控制count的信号量为1，即指导书中的mutex
            sem_init(&semForWrite, 0, 1); // 初始化控制写的信号量为1，即指导书中的RP_Write

            // 匿名函数实现RP_ReaderThread()
            auto RP_ReaderThread = [&](const ThreadInfo &threadInfo) {
                job_request(threadInfo);

                sem_wait(&semForCount);
                readCount++;
                if (readCount == 1)
                    sem_wait(&semForWrite);
                sem_post(&semForCount);

                job_running(threadInfo);

                sem_wait(&semForCount);
                readCount--;
                if (readCount == 0)
                    sem_post(&semForWrite);
                sem_post(&semForCount);
            };

            // 匿名函数实现RP_WriterThread()
            auto RP_WriterThread = [&](const ThreadInfo &threadInfo) {
                job_request(threadInfo);

                sem_wait(&semForWrite);

                job_running(threadInfo);

                sem_post(&semForWrite);
            };

            /* 
                调用运行RP_ReaderThread和
            */
            vector<thread> threads_vector;
            for (const auto &i : threadInfo_vector)
            {
                if (i.isReader)
                    threads_vector.emplace_back(thread(RP_ReaderThread, i));
                else
                    threads_vector.emplace_back(thread(RP_WriterThread, i));
            }

            // 子线程全部执行，主线程等待
            for (auto &i : threads_vector)
                if (i.joinable())
                    i.join();
        };

        /*
            利用匿名函数实现写者优先功能
        */
        auto writerPreference = [&]() {
            int readCount = 0, writeCount = 0;
            sem_t semForWriteCount, semForReadCount, semForWrite, semForRead;
            sem_init(&semForWriteCount, 0, 1); // 初始化控制writeCount的信号量为1，即指导书中的mutex1
            sem_init(&semForReadCount, 0, 1);  // 初始化控制readCount的信号量为1，即指导书中的mutex2
            sem_init(&semForWrite, 0, 1);      // 初始化控制写的信号量为1，即指导书中的cs_Write
            sem_init(&semForRead, 0, 1);       // 初始化控制读的信号量为1，即指导书中的cs_Read

            // 匿名函数实现WP_ReaderThread()
            auto WP_ReaderThread = [&](const ThreadInfo &threadInfo) {
                job_request(threadInfo);

                sem_wait(&semForRead);
                sem_wait(&semForReadCount);
                readCount++;
                if (readCount == 1)
                    sem_wait(&semForWrite);
                sem_post(&semForReadCount);
                sem_post(&semForRead);

                job_running(threadInfo);

                sem_wait(&semForReadCount);
                readCount--;
                if (readCount == 0)
                    sem_post(&semForWrite);
                sem_post(&semForReadCount);
            };

            // 匿名函数实现WP_WriterThread()
            auto WP_WriterThread = [&](const ThreadInfo &threadInfo) {
                job_request(threadInfo);

                sem_wait(&semForWriteCount);
                writeCount++;
                if (writeCount == 1)
                    sem_wait(&semForRead);
                sem_post(&semForWriteCount);

                sem_wait(&semForWrite);
                job_running(threadInfo);
                sem_post(&semForWrite);

                sem_wait(&semForWriteCount);
                writeCount--;
                if (writeCount == 0)
                    sem_post(&semForRead);
                sem_post(&semForWriteCount);
            };

            /* 
                writerPreference的主要执行部分
            */
            vector<thread> threads_vector;
            for (const auto &i : threadInfo_vector)
            {
                if (i.isReader)
                    threads_vector.emplace_back(thread(WP_ReaderThread, i));
                else
                    threads_vector.emplace_back(thread(WP_WriterThread, i));
            }

            // 子线程全部执行，主线程等待
            for (auto &i : threads_vector)
                if (i.joinable())
                    i.join();
        };

        /*
            main函数调用匿名函数输出结果
        */
        cout << "Reader Preference" << endl;
        outputFile << "Reader Preference" << endl;
        readerPreference();
        cout << "Writer Preference" << endl;
        outputFile << "Writer Preference" << endl;

        // 清空队列
        queue<int> empty;
        swap(empty, randomIntQueue);

        writerPreference();
        cout << "End" << endl;
        outputFile << "End" << endl;
    }
    catch (const exception &e)
    {
        cerr << e.what() << '\n';
    }

    return 0;
}
