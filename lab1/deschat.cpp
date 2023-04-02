#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <cstdlib>
#include "des.h"

using namespace std;

void StrFromBlock(char *str, const Block &block)
{
    memset(str, 0, 8); // 将8个字节全部置0
    for (size_t i = 0; i < block.size(); ++i)
    {
        if (true == block[i]) // 第i位为1
            *((unsigned char *)(str) + i / 8) |= (1 << (7 - i % 8));
    }
}

void BlockFromStr(Block &block, const char *str)
{
    for (size_t i = 0; i < block.size(); ++i)
    {
        if (0 != (*((unsigned char *)(str) + i / 8) & (1 << (7 - i % 8))))
            block[i] = true;
        else
            block[i] = false;
    }
}

int main(int argc, char *argv[])
{

    Method methode=e;
    Method methodd=d;
    Block block, bkey;           // 数据块和密钥
    BlockFromStr(bkey, "aeskey"); // 获取密钥
    char buffer[8]="hellowd";
    printf("%s\n",buffer);
    BlockFromStr(block, buffer); // 构建数据块
    des(block, bkey, methode);
    StrFromBlock(buffer, block); // 获取运算后的数据
    printf("%s\n",buffer);
    des(block, bkey, methodd);
    StrFromBlock(buffer, block); // 获取运算后的数据
    printf("%s\n",buffer);
    return 0;
}