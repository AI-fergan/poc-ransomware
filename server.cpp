#include "pch.h"

static map<string, unsigned long> keys;

/*
This function handle client connection.
input:
socket - the client socket.
output: null.
*/
void client_handle(int socket){
    unsigned long key = 0;
    
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    //get the client ip
    if (getpeername(socket, (struct sockaddr *)&client_addr, &addr_len) == -1) {
        std::cerr << "getpeername failed" << std::endl;
        close(socket);
        return;
    }

    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), ip_str, INET_ADDRSTRLEN);

    //read unsigned long from the client
    if (read(socket, &key, sizeof(unsigned long)) < 0) {
        std::cerr << "read failed" << std::endl;
        close(socket);
        return;
    }

    string ip = ip_str;
    keys[ip] = key;
    //print the client data
    cout << "ip: " << ip << ", key = " << key << endl;

    close(socket);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    unsigned long value = 0;

    //create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "socket failed" << std::endl;
        return 1;
    }

    //attach socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        std::cerr << "setsockopt" << std::endl;
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    //bind the socket to the address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "bind failed" << std::endl;
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        std::cerr << "listen" << std::endl;
        return 1;
    }

    while (true) {
        //accept incoming connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "accept" << std::endl;
            continue;
        }


        //handling the client with thread
        thread client(client_handle, new_socket);
        client.detach();
    }

    return 0;
}
