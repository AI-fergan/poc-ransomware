#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <map>
#include <thread>
#include <arpa/inet.h>

#define PORT 1717

using std::map;
using std::cout;
using std::cin;
using std::string;
using std::thread;
using std::endl;