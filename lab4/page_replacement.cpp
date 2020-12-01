#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#define ADDRESS_NUM 100  // 地址的数量
#define PAGE_CAPACITY 10 // 一个页容纳地址的数量
/*
    10个地址为一页，即共有100/10=10页
*/
#define FRAME_NUM 5 // 帧（物理内存块）的数量

typedef struct
{
    int type;
    int index;
    int next;
    bool isUsed;
} Address;

ifstream inputFile("addressInfo.txt");
string line;
stringstream ss;
int memoryAccessStream[ADDRESS_NUM];
int pageMapping[ADDRESS_NUM];

/**
 * @brief 读取文件，初始化内存，得到内存访问流（数组）
 * 
 * @return int 页地址流长度
 */
int init()
{
    Address memoryAccessString[ADDRESS_NUM];
    for (size_t i = 0; i < ADDRESS_NUM; i++)
    {
        memoryAccessStream[i] = -1;
        memoryAccessString[i].isUsed = false;
        memoryAccessString[i].index = i;
    }

    // 文件中读入地址类型
    if (getline(inputFile, line))
    {
        ss << line;
        int index = 0;
        while (ss >> memoryAccessString[index].type)
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
        while (ss >> memoryAccessString[index].next)
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

    Address *curAddress = &memoryAccessString[0];
    int arr_index = 0;
    // 为了防止访问地址死循环的情况，设定一个地址只能访问一次，若一个地址被多次访问，则该内存的next就指向其下一个地址（顺序执行）
    // 以下算法保证了内存访问从0开始，从ADDRESS_NUM-1结束，但是此时内存访问次数不一定为ADDRESS_NUM
    while (curAddress->next != ADDRESS_NUM)
    {
        if (!curAddress->isUsed)
        {
            memoryAccessStream[arr_index] = curAddress->index;
            curAddress->isUsed = true;
            curAddress = &memoryAccessString[curAddress->next];
            arr_index++;
        }
        else
        {
            curAddress = &memoryAccessString[curAddress->index + 1];
        }
    }
    memoryAccessStream[arr_index] = ADDRESS_NUM - 1;
    return arr_index + 1;
}

int FIFO(int addressStreamLen)
{
    cout << "\nFIFO:" << endl;
    vector<int> frames;
    int page_fault_count = 0;
    for (size_t i = 0; i < addressStreamLen; i++)
    {
        cout << "Line " << i + 1 << ": ";
        int page = pageMapping[i];
        vector<int>::iterator it = find(frames.begin(), frames.end(), page);
        if (it != frames.end())
        {
            // 页命中
            frames.erase(it);
            frames.insert(frames.begin(), page);
        }
        else if (frames.size() < FRAME_NUM)
        {
            // 页错误-物理内存空闲
            frames.push_back(page);
            page_fault_count++;
        }
        else
        {
            // 页错误-置换
            frames.pop_back();
            frames.insert(frames.begin(), page);
            page_fault_count++;
        }
        for (size_t j = 0; j < frames.size(); j++)
            cout << frames[j] << ' ';
        cout << endl
             << "\tPage fault: " << page_fault_count << endl;
    }
    return page_fault_count;
}

int main(int argc, char const *argv[])
{
    int accessedMemoryNum = init(); // 该值即页地址流长度
    cout << "设定随机生成内存共" << ADDRESS_NUM << "个，移除其中内存访问死循环，得到访问" << accessedMemoryNum << "个地址的访问串如下" << endl;
    cout << "内存访问串：" << endl;
    for (size_t i = 0; i < accessedMemoryNum; i++)
    {
        if (memoryAccessStream[i] != -1)
        {
            pageMapping[i] = memoryAccessStream[i] / PAGE_CAPACITY; // 页映射方式：每顺序10个地址为一个页
            cout << memoryAccessStream[i] << ' ';
        }
    }
    cout << endl;
    FIFO(accessedMemoryNum);

    return 0;
}
