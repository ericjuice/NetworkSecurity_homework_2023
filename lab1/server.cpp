#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdlib.h>
#include "help.h"

using namespace std;

int client_socket;
bool exit_flag = false;

void *recv_thread(void *);

void *send_thread(void *);
Block bkey; // 密钥
int main(int argc, char *argv[]) {
    if (argc <= 2) {
        cout << "Usage: " << argv[0] << " [IP] [port]" << endl;
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    // 创建 socket
    int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket < 0) {
        cout << "Error: failed to create socket." << endl;
        return -1;
    }

    // 绑定 socket
    sockaddr_in server_address{};
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &server_address.sin_addr);
    server_address.sin_port = htons(port);
    if (bind(server_socket, (sockaddr *) &server_address, sizeof(server_address)) < 0) {
        cout << "Error: failed to bind socket." << endl;
        return -1;
    }

    // 监听 socket
    if (listen(server_socket, 5) < 0) {
        cout << "Error: failed to listen on socket." << endl;
        return -1;
    }

    cout << "Server listening on " << ip << ":" << port << endl;

    // 接收客户端连接请求
    sockaddr_in client_address{};
    socklen_t client_address_length = sizeof(client_address);
    client_socket = accept(server_socket, (sockaddr *) &client_address, &client_address_length);
    if (client_socket < 0) {
        cout << "Error: failed to accept client connection." << endl;
        close(server_socket);
        return -1;
    }
    cout<<client_socket<<"  "<<server_socket<< endl;
    cout << "Client connected from " << inet_ntoa(client_address.sin_addr)
         << ":" << ntohs(client_address.sin_port) << endl;
    BlockFromStr(bkey, "aeskey"); // 获取密钥
    cout << "Get aeskey!\n";

    pthread_t tid_recv, tid_send;
    pthread_create(&tid_recv, NULL, recv_thread, NULL);
    pthread_create(&tid_send, NULL, send_thread, NULL);

    pthread_join(tid_recv, NULL);
    pthread_join(tid_send, NULL);

    // 关闭连接和socket
    close(client_socket);
    close(server_socket);

    return 0;
}

void *recv_thread(void *) {
    char buffer[BUFFER_SIZE]{};

    while (!exit_flag) {
        // 接收客户端发送的消息
        memset(buffer, 0, BUFFER_SIZE);
        int length = recv(client_socket, (void *)buffer, BUFFER_SIZE, 0);
        if (length < 0) {
            cout << "Error: failed to receive message from client." << endl;
            break;
        }
        if (length == 0) {
            cout << "Client closed the connection." << endl;
            exit(0);
            break;
        }

        // 显示客户端发送的消息
        cout << "Received message from client: " << endl;
        dealMsg(buffer, bkey);
    }
    pthread_exit(NULL);
}

void *send_thread(void *) {
    char buffer[BUFFER_SIZE];

    while (!exit_flag) {
        // 发送消息给客户端
        memset(buffer, 0, BUFFER_SIZE);
        cin.getline(buffer, BUFFER_SIZE - 1);
        if (strcmp(buffer, "exit") == 0 || strcmp(buffer, "exit\n") == 0) {
            exit_flag = true;
            break;
        }
        encryptMsg(buffer, bkey);
        if (send(client_socket, (void*)buffer, BUFFER_SIZE, 0) < 0) {
            cout << "Error: failed to send message to client." << endl;
            break;
        }
    }
    pthread_exit(NULL);
}