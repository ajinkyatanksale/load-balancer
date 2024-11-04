#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <pthread.h>
#include <mutex>

#include "header_parser.h"

using namespace std;

mutex thread_lock;

int server_socket;
int num_conn = 0;

pthread_t lb_worker_threads[10];

void create_connection (uint16_t port) {
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        cout << "Error while creating socket!!";
        perror(0);
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int bind_status = bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    if (bind_status == -1) {
        perror(0);
    }

    int listen_status = listen(server_socket, 5);
    if (listen_status == -1) {
        cout << "Error in listen call!!";
        perror(0);
    }
}

int accept_connection () {
    int client_socket = accept(server_socket, nullptr, nullptr);
    if (client_socket == -1) {
        cout << "Error while getting client socket!!";
        perror(0);
    }
    thread_lock.lock();
    num_conn++;
    thread_lock.unlock();
    return client_socket;
}

string create_response(request_ incoming_request) {
    if (incoming_request.url == "/") {
        return "HTTP/1.1 200 OK\nDate: Mon, 27 Jul 2009 12:28:53 GMT\nServer: Apache/2.2.14 (Win32)\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\nContent-Length: 20\nContent-Type: application/json\n\n{\"key\":\"Ajinkya\"}";
    }
    else if (incoming_request.url == "/hi") {
        return "HTTP/1.1 200 OK\nDate: Mon, 27 Jul 2009 12:28:53 GMT\nServer: Apache/2.2.14 (Win32)\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\nContent-Length: 20\nContent-Type: application/json\n\n{\"key\":\"HI\"}";
    }
    else if (incoming_request.url == "/bye") {
        return "HTTP/1.1 200 OK\nDate: Mon, 27 Jul 2009 12:28:53 GMT\nServer: Apache/2.2.14 (Win32)\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\nContent-Length: 20\nContent-Type: application/json\n\n{\"key\":\"Bye\"}";
    }
    else {
        return "HTTP/1.1 200 OK\nDate: Mon, 27 Jul 2009 12:28:53 GMT\nServer: Apache/2.2.14 (Win32)\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\nContent-Length: 40\nContent-Type: application/json\n\n{\"key\":\"Kya kar raha hai bhai\"}";
    }
}

string recieve_message (int client_socket) {
    char buffer[2048];
    int read_count = read(client_socket, buffer, sizeof(buffer));
    if (read_count == -1) {
        cout << "Read failed" << endl;
        perror(0);
    }
    request_ incoming_request = parse_header(buffer, read_count);
    string response = create_response(incoming_request);
    return response;
}   

void send_message (int client_socket, string response) {
    write(client_socket, response.c_str(), response.size());
}

void* process_request (void *args) {
    while (1) {
        cout << num_conn << endl;
        int client_socket = accept_connection();
        string response = recieve_message(client_socket);
        send_message(client_socket, response);
        close(client_socket);
    }
}

int main(int argc, char *argv[]) {
    create_connection (atoi(argv[1]));

    for (int i = 0; i < 10; i++) {
        pthread_create(&lb_worker_threads[i], NULL, &process_request, NULL);
    }
    for (int i = 0; i < 10; i++) {
        pthread_join(lb_worker_threads[i], NULL);
    }

    close(server_socket);
    return 0;
}
