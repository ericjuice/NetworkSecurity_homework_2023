#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "help.h"

using namespace std;

int server_socket;
bool exit_flag = false;

void *recv_thread(void *);
void *send_thread(void *);
Block bkey; // 密钥
int main(int argc, char *argv[])
{
    if (argc <= 2)
    {
        cout << "Usage: " << argv[0] << " [IP] [port]" << endl;
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);
    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket < 0)
    {
        cout << "Error: failed to create socket." << endl;
        return -1;
    }

    // 连接服务器
    sockaddr_in server_address{};
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &server_address.sin_addr);
    server_address.sin_port = htons(port);
    if (connect(server_socket, (sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        cout << "Error: failed to connect to server." << endl;
        close(server_socket);
        return -1;
    }
    cout<<server_socket<<endl;
    cout << "Connected to server at " << ip << ":" << port << endl;
    BlockFromStr(bkey, "aeskey"); // 获取密钥
    cout << "Get aeskey!\n";

    pthread_t tid_recv, tid_send;
    pthread_create(&tid_recv, NULL, recv_thread, NULL);
    pthread_create(&tid_send, NULL, send_thread, NULL);

    pthread_join(tid_recv, NULL);
    pthread_join(tid_send, NULL);

    // 关闭连接和socket 
    close(server_socket);

    return 0;
}

void *recv_thread(void *)
{
    char buffer[BUFFER_SIZE]{};

    while (!exit_flag)
    {
        // 接收服务器发送的消息
        memset(buffer, 0, BUFFER_SIZE);
        int length = recv(server_socket, (void *)buffer, BUFFER_SIZE, 0);
        if (length < 0)
        {
            cout << "Error: failed to receive message from server." << endl;
            break;
        }
        if (length == 0)
        {
            cout << "Server closed the connection." << endl;
            exit(0);
            break;
        }
        // 显示服务器发送的消息
        cout << "Received message from server: " << endl;
        dealMsg(buffer, bkey);
    }
    pthread_exit(NULL);
}

void *send_thread(void *)
{
    char buffer[BUFFER_SIZE];

    while (!exit_flag)
    {
        // 发送消息给服务器
        memset(buffer, 0, BUFFER_SIZE);
        cin.getline(buffer, BUFFER_SIZE - 1);
        if (strcmp(buffer, "exit") == 0 || strcmp(buffer, "exit\n") == 0)
        {
            exit_flag = true;
            break;
        }
        encryptMsg(buffer, bkey);
        if (send(server_socket, (void *)buffer, BUFFER_SIZE, 0) < 0)
        {
            cout << "Error: failed to send message to server." << endl;
            break;
        }
    }
    pthread_exit(NULL);
}
