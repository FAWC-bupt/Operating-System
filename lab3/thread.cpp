#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <mutex>
#include <semaphore.h>
#include <thread>

using namespace std;

struct ThreadInfo
{
    // 根据理论分析图设置数据结构
    int index;
    bool isReader;
    double startTime;
    double duration;
};

mutex mutexForJob; // 线程任务中队列操作的互斥锁

// 重载输出流运算符
ostream &operator<<(ostream &os, const ThreadInfo &threadInfo)
{
    return os << "Thread " << threadInfo.index << (threadInfo.isReader ? " - Reader " : " - Writer ");
}

void job_request(const ThreadInfo &threadInfo)
{
}

void job_running(const ThreadInfo &threadInfo)
{
}

int main(int argc, char const *argv[])
{
    try
    {
        if (argc < 2)
            throw std::runtime_error(
                "How to use: ./thread <Input File Name>\n"
                "Every line in input file should be like: <index> <Role (R/W)> <Start Time> <Duration>");

        ifstream inputFile(argv[1]);
        if (!inputFile.is_open())
            throw runtime_error(string("Can not open file: ") + argv[1] + string(", please check it out."));

        // 从文件读取数据并加载至vector容器
        vector<ThreadInfo> threadInfo_vector;
        char buf[64];
        while (inputFile.getline(buf, 64))
        {
            ThreadInfo tempInfo;
            stringstream ss(buf);
            char role;
            ss >> tempInfo.index >> role >> tempInfo.startTime >> tempInfo.duration;
            if (role == 'R' || role == 'r')
                tempInfo.isReader = true;
            else if (role == 'W' || role == 'w')
                tempInfo.isReader = false;
            else
                throw runtime_error("The \"role\" char is invalid, please check it out.");

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
                readerPreference的主要执行部分
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
            sem_init(&semForWriteCount, 0, 1);
            sem_init(&semForReadCount, 0, 1);
            sem_init(&semForWrite, 0, 1);
            sem_init(&semForRead, 0, 1);

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
            };
        };
    }
    catch (const exception &e)
    {
        cerr << e.what() << '\n';
    }

    system("pause");
    return 0;
}
