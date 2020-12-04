#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <math.h>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#define ADDRESS_NUM 100  // 地址的数量
#define PAGE_CAPACITY 10 // 一个页容纳地址的数量
#define PAGE_NUM 10      //10个地址为一页，即共有100/10=10页
int frame_num = 3;       // 帧（物理内存块）的数量

typedef struct
{
    int type;  // 地址类型：顺序、在前或在后
    int index; // 地址下标
    int next;  // 指向的下一个地址
    int page;  // 下一个地址所在的页（需要的页）
} Address;

ifstream inputFile("addressInfo.txt");
ofstream outputFile("result.txt");
string line;
stringstream ss;
// int memoryAccessStream[ADDRESS_NUM];
// int pageMapping[ADDRESS_NUM];
Address memoryAccessStream[ADDRESS_NUM];

/**
 * @brief 找到数组最小值的下标，但忽略数组元素-1
 * 
 * @param arr 目标数组
 * @param len 数组长度
 * @return int 最小值下标
 */
int findArrMinIndex(int *arr, int len)
{
    int minValue = INT32_MAX, index = 0;
    for (size_t i = 0; i < len; i++)
    {
        if (arr[i] < minValue && arr[i] != -1)
        {
            minValue = arr[i];
            index = i;
        }
    }
    return index;
}

/**
 * @brief 读取文件，初始化内存，得到内存访问流
 * 
 */
void init()
{
    // 文件中读入地址类型
    if (getline(inputFile, line))
    {
        ss << line;
        int index = 0;
        while (ss >> memoryAccessStream[index].type)
        {
            //  cout << memoryAccessString[index].type << ' ';
            index++;
        }
        //  cout << endl;
        ss.str() = "";
        ss.clear();
    }
    else
        throw "Read file error";

    // 文件中读入地址的下一个地址指针
    if (getline(inputFile, line))
    {
        ss << line;
        int index = 0;
        while (ss >> memoryAccessStream[index].next)
        {
            //  cout << memoryAccessString[index].next << ' ';
            index++;
        }
        //  cout << endl;
        ss.str() = "";
        ss.clear();
    }
    else
        throw "Read file error";

    for (size_t i = 0; i < ADDRESS_NUM; i++)
    {
        memoryAccessStream[i].page = memoryAccessStream[i].next / PAGE_CAPACITY;
        memoryAccessStream[i].index = i;
    }
}

/**
 * @brief 先进先出算法
 * 
 * @param addressStreamLen 内存访问流长度
 * @return int 页错误数
 */
int FIFO(int addressStreamLen)
{
    cout << "\nFIFO:" << endl;
    outputFile << "\nFIFO:" << endl;
    /**
     * 队列是FIFO的，因此用队列实现即可
     * 但是由于要执行队列元素查找操作，因此利用Vector容器模拟队列较为合适
     */
    vector<int> frames;
    int page_fault_count = 0;
    for (size_t i = 0; i < addressStreamLen; i++)
    {
        cout << "Line " << i + 1 << ": ";
        outputFile << "Line " << i + 1 << ": ";
        int page = memoryAccessStream[i].page;
        vector<int>::iterator it = find(frames.begin(), frames.end(), page);
        if (it != frames.end())
        {
            // 页命中
            // DO NOTHING
        }
        else if (frames.size() < frame_num)
        {
            // 页错误-物理内存空闲 在队头加入页
            frames.insert(frames.begin(), page);
            page_fault_count++;
        }
        else
        {
            // 页错误-置换 删去队尾页，新页加入队头
            frames.pop_back();
            frames.insert(frames.begin(), page);
            page_fault_count++;
        }
        for (size_t j = 0; j < frames.size(); j++)
        {
            cout << frames[j] << ' ';
            outputFile << frames[j] << ' ';
        }
        cout << endl
             << "\tPage fault: " << page_fault_count << endl;
        outputFile << endl
                   << "\tPage fault: " << page_fault_count << endl;
    }
    return page_fault_count;
}

/**
 * @brief 最近最少使用算法
 * 
 * @param addressStreamLen 内存访问流长度
 * @return int 页错误数
 */
int LRU(int addressStreamLen)
{
    cout << "\nLRU:" << endl;
    outputFile << "\nLRU:" << endl;
    /**
     * 利用队列模拟LRU
     * 新到的页放入队头（包括物理内存空和页命中两种情况），其它页依次向队列后方移动，
     * 则队尾一定为最近最少使用页，因此当帧队列满时，删除队尾即可
     * 
     */
    vector<int> frames;
    int page_fault_count = 0;
    for (size_t i = 0; i < addressStreamLen; i++)
    {
        cout << "Line " << i + 1 << ": ";
        outputFile << "Line " << i + 1 << ": ";
        int page = memoryAccessStream[i].page;
        vector<int>::iterator it = find(frames.begin(), frames.end(), page);
        if (it != frames.end())
        {
            // 页命中 则把命中页放到队头
            frames.erase(it);
            frames.insert(frames.begin(), page);
        }
        else if (frames.size() < frame_num)
        {
            // 页错误-物理内存空闲
            frames.insert(frames.begin(), page);
            page_fault_count++;
        }
        else
        {
            // 页错误-置换 队尾为最近最少使用页，利用pop_back()删除，新页放入页头
            frames.pop_back();
            frames.insert(frames.begin(), page);
            page_fault_count++;
        }
        for (size_t j = 0; j < frames.size(); j++)
        {
            cout << frames[j] << ' ';
            outputFile << frames[j] << ' ';
        }
        cout << endl
             << "\tPage fault: " << page_fault_count << endl;
        outputFile << endl
                   << "\tPage fault: " << page_fault_count << endl;
    }
    return page_fault_count;
}

/**
 * @brief 最优置换算法
 * 
 * @param addressStreamLen 内存访问流长度
 * @return int 页错误数
 */
int Optimal(int addressStreamLen)
{
    cout << "\nOptimal:" << endl;
    outputFile << "\nOptimal:" << endl;

    vector<int> frames;
    int page_fault_count = 0;
    for (size_t i = 0; i < addressStreamLen; i++)
    {
        cout << "Line " << i + 1 << ": ";
        outputFile << "Line " << i + 1 << ": ";
        int page = memoryAccessStream[i].page, future_knowledge[PAGE_NUM];
        // future_knowledge指示了每个页在下一次访问时memoryAccessString的下标，下标越大说明需要越长时间才能访问
        for (size_t j = 0; j < PAGE_NUM; j++)
        {
            future_knowledge[j] = INT32_MAX; // future_knowledge初始化为“无限”，这意味着如果这个页未来不可能被使用，则到达其下一次访问的下标为无穷大
            for (size_t k = i + 1; k < addressStreamLen; k++)
            {
                if (memoryAccessStream[k].page == j)
                {
                    future_knowledge[j] = k;
                    break;
                }
            }
        }

        vector<int>::iterator it = find(frames.begin(), frames.end(), page);
        if (it != frames.end())
        {
            // 页命中
            // Do Nothing
        }
        else if (frames.size() < frame_num)
        {
            // 页错误-物理内存空闲
            frames.insert(frames.begin(), page);
            page_fault_count++;
        }
        else
        {
            // 页错误-置换

            // 遍历帧队列，找到其future_knowledge最大的帧
            int max_future_knowledge = INT32_MIN, page_to_replace = -1;
            for (size_t j = 0; j < frames.size(); j++)
            {
                if (future_knowledge[frames[j]] > max_future_knowledge)
                {
                    max_future_knowledge = future_knowledge[frames[j]];
                    page_to_replace = frames[j];
                }
            }

            vector<int>::iterator page_to_replace_it = find(frames.begin(), frames.end(), page_to_replace);
            if (page_to_replace_it == frames.end())
                throw "future_knowledge error";

            frames.erase(page_to_replace_it);
            frames.insert(frames.begin(), page);
            page_fault_count++;
        }
        for (size_t j = 0; j < frames.size(); j++)
        {
            cout << frames[j] << ' ';
            outputFile << frames[j] << ' ';
        }
        cout << endl
             << "\tPage fault: " << page_fault_count << endl;
        outputFile << endl
                   << "\tPage fault: " << page_fault_count << endl;
    }
    return page_fault_count;
}

int main(int argc, char const *argv[])
{
    cout << "请输入可用帧（物理内存块）数量: " << endl;
    while (!(cin >> frame_num))
    {                //cin输入错误时执行下边语句
        cin.clear(); //清除流标记
        cin.ignore();
        cin.sync();                         //清空流
        cout << "输入了非数字字符" << endl; //打印错误提示
    }
    int fifo_fault, lru_fault, optimal_fault;
    init();
    cout << "设定随机生成指令共" << ADDRESS_NUM << "个" << endl
         << "页大小: " << PAGE_CAPACITY << endl
         << "可用帧（物理内存块）数量: " << frame_num << endl
         << "内存访问串：" << endl;
    outputFile << "设定随机生成指令共" << ADDRESS_NUM << "个" << endl
               << "页大小: " << PAGE_CAPACITY << endl
               << "可用帧（物理内存块）数量: " << frame_num << endl
               << "内存访问串：" << endl;
    for (size_t i = 0; i < ADDRESS_NUM; i++)
    {
        cout << memoryAccessStream[i].next << ' ';
        outputFile << memoryAccessStream[i].next << ' ';
    }
    cout << endl;
    outputFile << endl;

    fifo_fault = FIFO(ADDRESS_NUM);
    lru_fault = LRU(ADDRESS_NUM);
    optimal_fault = Optimal(ADDRESS_NUM);

    cout << "缺页率如下: " << endl;
    outputFile << "缺页率如下: " << endl;

    cout << "FIFO: " << fifo_fault << '%' << endl;
    outputFile << "FIFO: " << fifo_fault << '%' << endl;

    cout << "LRU: " << lru_fault << '%' << endl;
    outputFile << "LRU: " << lru_fault << '%' << endl;

    cout << "Optimal: " << optimal_fault << '%' << endl;
    outputFile << "Optimal: " << optimal_fault << '%' << endl;

    return 0;
}
