#include <iostream>
#include <thread>
#include "protocol.h"
#include "./DES/aes.h"
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "RSA/bigInt.h"
#include "RSA/gcd.h"
#include "RSA/mrTest.h"
#include "RSA/power.h"
#include "RSA/random.h"

typedef unsigned char uchar;
using namespace std;

class client
{
private:
    int endflag = 1;
    int act; // 1:active，2：passive

    int text[4][4];     // temp array for AES
    int key[4][4];      // AES key
    MsgStruct msg_send; // msg to send or recv
    MsgStruct msg_recv;

    int sock_my;
    int sock_des;
    int Active_port = 8888; // network
    int Passive_port = 8888;
    string Active_ip = "127.0.0.1";
    string Passive_ip = "127.0.0.1";
    sockaddr_in address_des{};
    sockaddr_in address_my{};
    socklen_t addlen = sizeof(address_des);

public:
    client();
    ~client()
    {
        close(sock_my);
        close(sock_des);
    }
    void working();
    void recvmsg(int skt);
    void packmsg(int ctrl, char *msg); // pack msg and AES encrypt
    void decode_msg();                 // AES decode
    void aes_encode(uchar a[16]);
    void aes_decode(uchar a[16]);

    void RSA_keygen();
    void RSA_decrypt(string aeskey);
    string RSA_encrypt(string src);

    void printhelp();
};

int main()
{
    client myclient;
    myclient.working();
}

client::client()
{
    sock_my = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // Ipv4,tcp
    if (sock_my < 0)
    {
        cout << "Socket fail";
        return;
    }

    printf("\nPlease select your state:\nCase 1: Active one\nCase 2: Passive one\n");
    cin >> act;
    switch (act)
    {
    case 2: // passively waiting for connection
    {
        bzero(&address_my, sizeof(address_my));
        address_my.sin_family = AF_INET;
        inet_pton(AF_INET, Passive_ip.c_str(), &address_my.sin_addr);
        address_my.sin_port = htons(Passive_port);
        if (bind(sock_my, (sockaddr *)&address_my, sizeof(address_my)) < 0)
        {
            cout << "Error: failed to bind socket." << endl;
            return ;
        }
        // printf("\nPassive init success!\nwaiting for AES key...\n");
        printf("\nPassive waiting...\n");
        break;
    }
    case 1: // connect client actively
    {
        bzero(&address_des, sizeof(address_des));
        address_des.sin_family = AF_INET;
        inet_pton(AF_INET, Passive_ip.c_str(), &address_des.sin_addr);
        address_des.sin_port = htons(Passive_port);
        if (connect(sock_my, (sockaddr *)&address_des, sizeof(address_des)) < 0)
        {
            cout << "Error: failed to connect to server." << endl;
            close(sock_my);
            exit(-1) ;
        }
        // printf("\nRandom AES key:\n"); // generate random aes key
        printf("starting......\n");
        memset(&msg_send, 0, sizeof(msg_send));
        string aes_key;
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                key[i][j] = rand() % 255;
                aes_key.append(1, key[i][j] / 100 + '0'); // number to string,so we can decode successfully
                aes_key.append(1, (key[i][j] % 100) / 10 + '0');
                aes_key.append(1, key[i][j] % 10 + '0');
                aes_key.append(1, ' ');
                // printf("%d ", key[i][j]);
            }
        }
        string key_send = RSA_encrypt(aes_key);
        for (int i = 0; i < key_send.length(); i++)
        {
            msg_send.MsgBuf[i] = (uchar)key_send[i];
        }
        msg_send.ctrl = MyAES_key;
        msg_send.MsgLen=key_send.length();
        // printf("\nAES key has been encrypted,msg length:%d\n",msg_send.MsgLen);
        int err = send(sock_my, (char *)&msg_send, sizeof(msg_send), 0 );
        // cout<<err;
        if (err < 0)
        {
            printf("sending AES key fail");
            return;
        }
        else
        {
            // printf("\nsend AES key to client succeed\n");
            ;
        }

        printf("Active init succeed! You can chat now!\n");
        break;
    }
    default:
        printf("\nerror input,pls restart\n");
        break;
    }
}

void client::working()
{
    // server接收连接请求
    if (act == 2)
    {
        if (listen(sock_my, 5) < 0)
        {
            cout << "Error: failed to listen on socket." << endl;
            return;
        }
        socklen_t des_address_length = sizeof(address_des);
        sock_des = accept(sock_my, (sockaddr *) &address_des, &des_address_length);
         cout << "Client connected from " << inet_ntoa(address_des.sin_addr)
         << ":" << ntohs(address_des.sin_port) <<". Plz waiting..."<< endl;
        if (sock_des < 0) {
            cout << "Error: failed to accept client connection." << endl;
            return ;
        }
    }
    else{ // for active one ,we only need one skt
        sock_des = sock_my;
    }

    thread t_recvmsg(&client::recvmsg ,this, sock_des); // new thread to receive msg
    t_recvmsg.detach();
    while (endflag)
    {
        printhelp();
        // for elegant, default select == 1
        int select;
        // cin.sync();
        // cin >> select;
        select = 1;
        switch (select)
        {
        case 0:
            endflag = 0;
            printf("\nExited!\n");
            break;
        case 1:
        {
            if(cin.peek()=='\n')
                cin.ignore();
            char msg[256] = {0};
            cin.getline(msg, 256);
            packmsg(Crypt_Msg, msg);
            if (send(sock_des, (char *)&msg_send, sizeof(msg_send), 0) < 0)
            {
                printf("\nsend data fail,pls restart\n");
                return;
            }
            else
            {
                printf("Send success");
                // fflush(stdin);
            }
            break;
        }
        case 2:
            RSA_keygen();
            break;
        default:
            printf("\nerror input\n");
            break;
        }
    }
    // sleep(50);
}

void client::recvmsg(int skt)
{
    // 监听 socket
    while (endflag)
    {
        int fk = recv(skt, (char *)&msg_recv, sizeof(msg_recv), 0);
        if (fk < 0)
        {
            cout << "\nerror recv msg" << endl;
            break;
        }
        if (fk == 0)
        {
            cout << "\nexit" << endl;
            break;
        }
        printf("\n\033[31mRecv Msg: \033[0m");
        switch (msg_recv.ctrl)
        {
        case Error_Msg:
            printf("\nerror msg\n");
            break;
        case Plain_Msg:
        {
            printf("\nreceived plain msg:\n%s\n", msg_recv.MsgBuf);
            break;
        }
        case Crypt_Msg:
        {
            // printf("\nreceived encrypted msg,len:%d,trying to decode:\n", msg_recv.MsgLen);
            decode_msg();
            break;
        }
        case MyAES_key:
        {
            // printf("\nAES key received!starting decode...\n"); // convert uchar to char.
            string aes_key;
            for (int i = 0; i < msg_recv.MsgLen; i++)
            {
                aes_key.append(1, (char)msg_recv.MsgBuf[i]);
            }
            RSA_decrypt(aes_key);
            break;
        }
        }
    }
}

void client::aes_decode(uchar ipt_msg[16]) // only for 128 bits msg, saving result in text[][]
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            text[i][j] = (int)ipt_msg[i * 4 + j];
        }
    }

    Decode(text, key);
    return;
}

void client::aes_encode(uchar ipt_msg[16]) // only for 128 bits msg, saving result in text[][]
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            text[i][j] = (int)ipt_msg[i * 4 + j];
        }
    }

    Encode(text, key);
    cout << endl;
    return;
}

void client::packmsg(int ctrl, char *msg)
{
    memset(&msg_send, 0, sizeof(msg_send));
    msg_send.ctrl = ctrl;
    msg_send.MsgLen = strlen(msg);
    int round = msg_send.MsgLen / 16 + (msg_send.MsgLen % 16 == 0 ? 0 : 1); // long msg,need AES-CBC encrypt
    uchar msg_temp[16];
    uchar init_vector[16]; // init vetor for CBC of AES
    memcpy(msg_temp, msg, 16);
    aes_encode(msg_temp);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
            msg_send.MsgBuf[4 * i + j] = (uchar)text[i][j];
    }
    if (round > 1) // need AES-CBC
    {
        for (int i = 1; i < round; i++)
        {
            memcpy(init_vector, msg_send.MsgBuf + (i - 1) * 16, 16); // IV=last encrypt msg
            memcpy(msg_temp, msg + 16 * i, 16);                      // put the next group to msg_temp
            for (int j = 0; j < 16; j++)
            {
                msg_temp[j] = (msg_temp[j] ^ init_vector[j]); // XOR by bits
            }
            aes_encode(msg_temp);
            for (int m = 0; m < 4; m++)
            {
                for (int n = 0; n < 4; n++)
                    msg_send.MsgBuf[16 * i + m * 4 + n] = text[m][n];
            }
        }
    }
    // printf("msg has been packed successfully,ctrl:%d,len:%d\n", ctrl, msg_send.MsgLen);
}

void client::decode_msg()
{
    int round = msg_recv.MsgLen / 16 + (msg_recv.MsgLen % 16 == 0 ? 0 : 1); // long msg,need AES-CBC encrypt
    uchar msg_temp[16];
    uchar init_vector[16]; // init vetor for CBC of AES
    memcpy(msg_temp, msg_recv.MsgBuf, 16);
    aes_decode(msg_temp);
    for (int m = 0; m < 4; m++)
    {
        for (int n = 0; n < 4; n++)
            printf("%c", (uchar)text[m][n]); // the first group is not XORed
    }
    if (round > 1) // need AES-CBC
    {
        for (int i = 1; i < round; i++)
        {
            memcpy(init_vector, msg_recv.MsgBuf + (i - 1) * 16, 16); // IV=last encrypt msg
            memcpy(msg_temp, msg_recv.MsgBuf + 16 * i, 16);          // put the next group to msg_temp
            aes_decode(msg_temp);
            // for (int m = 0; m < 4; m++)
            // {
            //     for (int n = 0; n < 4; n++)
            //         printf("%c", (uchar)text[m][n] ^ init_vector[4 * m + n]); // now we need recover XORed text
            // }
        }
    }
    // printf("\n\nmsg has been decoded successfully,len:%d\n", msg_recv.MsgLen);
    printhelp();
}

void client::RSA_decrypt(string aes_key)
{
    bigInt c(aes_key);
    bigInt d, n;
    ifstream i("./RSA/prikey.txt");
    i >> n >> d;
    i.close();
    clock_t start, finish;
    // cout << "AES key is been decrypting...len:" <<aes_key.length()<< endl;
    start = clock();
    bigInt text = power(c, d, n);
    string my_key = bigInt2string(text);
    // cout << "128 bits AES key(dec):\n" << my_key << endl;
    finish = clock();
    // cout << "decryption costs " << (finish - start)/1000 << "ms \n";
    // printf("\nSaving AES key..\n");
    int str_ptr = 0; // ptr to aes key
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            int temp = 0;
            for (int k = 0; k < 3; k++)
            {
                temp += int(my_key.c_str()[str_ptr] - '0');
                // the aes key is saved as string which is divided by space,such as:000 111 222
                str_ptr++;
                if (k < 2)
                    temp *= 10;
            }
            str_ptr++; // this is the space
            key[i][j] = temp;
            // printf("%d ", key[i][j]);
        }
    }
    printf("Init OK!You can chat now!\n");
    printhelp();
}

string client::RSA_encrypt(string src)
{
    bigInt b = string2bigInt(src);
    bigInt e, n;
    ifstream i("./RSA/pubkey.txt");
    i >> n >> e;
    i.close();
    clock_t start, finish;
    // cout << "\nAES key is been encrypting..." << endl;
    start = clock();
    bigInt c = power(b, e, n);
    finish = clock();
    // cout << "encryption costs " << (finish - start)/1000 << " ms \n";
    return c.getnum(); // return the num string
}

void client::RSA_keygen()
{
    ofstream o1("./RSA/pubkey.txt");
    ofstream o2("./RSA/prikey.txt");
    setRandom(time(NULL));
    clock_t start, finish;

    cout << "generating p and q...(this may take a minites)" << endl;
    start = clock();
    bigInt p, q;
    p = createprime();
    q = createprime();
    while (p == q)
        q = createprime();
    finish = clock();
    cout << "generate p and q costs " << finish - start << "ms \n";

    bigInt n = p * q;
    bigInt fai = (p - bigInt("1")) * (q - bigInt("1"));

    cout << "generating d and e..." << endl;
    bigInt d, e;
    start = clock();
    d = bigInt("3");
    while (!(gcd(d, fai) == bigInt("1")))
        d = d + bigInt("2");
    e = inv(d, fai);
    finish = clock();
    cout << "generate d and e costs " << finish - start << "ms \n";

    o1 << n << " " << e;
    o2 << n << " " << d << " " << p << " " << q;
    printf("\ngen RSA succeed\n");
    o1.close();
    o2.close();
    ;
}

void client::printhelp(){
    // printf("\n\033[34mPlease input: \n");
    // printf("plain text to send message;\n");
    // printf("0: exit;\n");
    // printf("1:send msg(max:512 byte);\n");
    // printf("2--regenerate RSA key(need restart)\033[0m\n");
    printf("\n\033[1;32mUser@localhost:\033[0m$ ");
    fflush(stdout);
}