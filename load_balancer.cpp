#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <pthread.h>
#include <map>
#include <mutex>

using namespace std;

int server_socket;

mutex conn_map_lock;

const int NUM_THREADS = 10;

pthread_t lb_worker_threads[NUM_THREADS];
map<int, int> connection_count;

int create_server_connection (int port) {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    return clientSocket;
}

string forward_request_to_server(int clientSocket, string request) {
    send(clientSocket, (char *)request.c_str(), request.size(), 0);
    char buffer[2048];
    recv(clientSocket, buffer, sizeof(buffer),0);
    char new_buffer[2048];
    strcpy(new_buffer, buffer);
    string response(new_buffer);
    close(clientSocket);
    return response;
}

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

int select_port () {
    conn_map_lock.lock();
    int count = INT32_MAX, port = -1;
    for (auto itr = connection_count.begin(); itr != connection_count.end(); ++itr) {
        if (count > itr->second) {
            cout << "itr" << itr->first << endl;
            count = itr->second;
            port = itr->first;
        }
    }
    connection_count[port]++;
    conn_map_lock.unlock();
    return port;
}

int accept_connection () {
    int serverSocket = accept(server_socket, nullptr, nullptr);
    if (serverSocket == -1) {
        cout << "Error while getting client socket!!";
        perror(0);
    }
    cout << "Client FD " << serverSocket << endl;
    return serverSocket;
}

string recieve_message (int serverSocket) {
    char buffer1[2048];
    int read_status = read(serverSocket, buffer1, sizeof(buffer1));
    if (read_status == -1) {
        cout << "Read failed" << endl;
        perror(0);
    }
    return string(buffer1);
}   

void send_message (int serverSocket, string response) {
    int write_count = write(serverSocket, (char *)response.c_str(), response.size());
}

void* process_request (void *args) {
    while (1) {
        // Accept incoming connection from client
        int serverSocket = accept_connection();
        // Recieve the request message
        string request = recieve_message(serverSocket);
        // Select server to forward request
        int server_port = select_port ();
        // Establish client request with server
        int clientSocket = create_server_connection(server_port);
        // Forward the request to server and wait till response is recieved
        string response = forward_request_to_server(clientSocket, request);
        // Manage connection count of particular server
        conn_map_lock.lock();
        connection_count[server_port]--;
        conn_map_lock.unlock();
        // Send back the response to client
        send_message(serverSocket, response);
        close(serverSocket);
    }
}

int main(int argc, char *argv[]) {
    create_connection (atoi(argv[1]));

    for (int i = 2; i < argc; i++) {
        cout << argv[i] << endl; 
        connection_count[atoi(argv[i])] = 0;
    }

    for (int i = 0; i < 10; i++) {
        pthread_create(&lb_worker_threads[i], NULL, &process_request, NULL);
    }
    for (int i = 0; i < 10; i++) {
        pthread_join(lb_worker_threads[i], NULL);
    }

    close(server_socket);
    return 0;
}
