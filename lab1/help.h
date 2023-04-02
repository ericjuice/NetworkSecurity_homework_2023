#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include "des.h"

#ifndef help_h
#define help_h
void StrFromBlock(char *str, const Block &block);
void BlockFromStr(Block &block, const char *str);
Method methode = e;
Method methodd = d;
const int BUFFER_SIZE = 1024;
void dealMsg(char *msg,Block bkey){
    printf("Msg decrypted!\n");
    int len = BUFFER_SIZE;
    for(int i =0; i<len/8+1 ;i++){
        if(strncmp(msg+i*8,"\0\0\0\0\0\0\0\0",8)==0){//if null, then end
            printf("\n");
            return;
        }
        Block block;
        BlockFromStr(block, msg+i*8);
        des(block, bkey, methodd);  //decrypt
        char temp[8];
        StrFromBlock(temp, block);
        printf("%s",temp);
    }
}
void encryptMsg(char *msg,Block bkey){
    int len = strlen(msg);
    for(int i =0; i<len/8+1 ;i++){
        Block block;
        BlockFromStr(block, msg+i*8);
        des(block, bkey, methode);  //encrypt
        StrFromBlock(msg+i*8, block);
    }
    printf("Msg encrypted\n");
}
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

#endif

