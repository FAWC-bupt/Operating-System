#include <fstream>
#include <iostream>

using namespace std;

#define ADDRESS_NUM 100

ofstream outputFile("addressInfo.txt");

/**
 * @brief 数组乱序，但数组开头和结尾元素的内容不会改变
 * 
 * @param arr 目标数组
 * @param count 数组长度
 */
void makeRand(int arr[], int count)
{
    srand((unsigned int)time(NULL)); //随机数种子;
    for (int i = 1; i < count - 2; i++)
    {
        int num = i + rand() % (count - 2 - i); // 取随机数
        int temp = arr[i];
        arr[i] = arr[num];
        arr[num] = temp; //交换
    }
}

int main(int argc, char const *argv[])
{
    /**
     * 顺序执行：1
     * 在前地址：0
     * 在后地址：-1
     * 
     */
    int address_state[ADDRESS_NUM], address_next[ADDRESS_NUM];

    // 70个顺序地址，10个在前地址，20个在后地址
    for (size_t i = 0; i < ADDRESS_NUM - 1; i++)
    {
        if (i < 69)
            address_state[i] = 1;
        else if (i < 89)
            address_state[i] = 0;
        else
            address_state[i] = -1;
    }
    address_state[ADDRESS_NUM - 1] = 1;

    // 该乱序仍然保证最后和第一个地址都是顺序执行的地址
    makeRand(address_state, ADDRESS_NUM);

    // 确定每个地址指向的的下一个地址
    for (size_t i = 0; i < ADDRESS_NUM; i++)
    {
        if (address_state[i] == 1)
            address_next[i] = i + 1; // 这意味着最后一个地址会指向ADDRESS_NUM
        else if (address_state[i] == 0)
        {
            srand((unsigned int)time(NULL));
            address_next[i] = rand() % i; // 均等机会取[0,i-1]中的值
        }
        else
        {
            srand((unsigned int)time(NULL));
            address_next[i] = i + 1 + rand() % (ADDRESS_NUM - i - 1); // 均等机会取[i+1,ADDRESS_NUM-1]中的值
        }
    }

    // 输出地址状态
    for (size_t i = 0; i < ADDRESS_NUM - 1; i++)
        outputFile << address_state[i] << ' ';
    outputFile << address_state[ADDRESS_NUM - 1] << endl;

    // 输出每个地址的下一个地址
    for (size_t i = 0; i < ADDRESS_NUM - 1; i++)
        outputFile << address_next[i] << ' ';
    outputFile << address_next[ADDRESS_NUM - 1] << endl;

    outputFile.close();
    return 0;
}
