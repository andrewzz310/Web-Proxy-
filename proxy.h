#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <ctype.h>
#include <iostream>
#include <stdarg.h>
#include <map>
#include <sstream>
#include <algorithm>
#include <pthread.h>
#include <semaphore.h>

#define DEBUG 0

#define MAX_USERS 1

using namespace std;

#define DEFALUT_PORT "80"
#define PATTERN_ONE "GET http:/"
#define PATTERN_TWO "HTTP/1.0"
#define METHOD "GET"
#define PROTOCOL "HTTP"
#define VERSION "1.0"
#define BREAK "\r\n\r\n"

static int MAX_MESSAGE = 10000;

struct userRequestInfo {
  string method;
  string hostname;
  string path;
  string port;
  string protocol;
  string version;
  map<string, string> headers;
  bool isError;
};


sem_t maxConcurrent;
int MAX_CONCURRENT_USERS = 30;

struct request_args {
  int acceptConn;
  struct sockaddr_in clientAddr;
};


bool IsValidNumber(char *string);

void debug(const char * format, ...);

int createserverSocket(char* pcAddress, char* pcPort);

userRequestInfo parseUserString(string request);

int getSocket(char* hostname, char* port);

void send500ToClient(int cliSocket);

void *handleNewClient(void *cliSocketFromOS);

void sendToClient(const char *buffer, int sockfd, int bufferLength);

void sendToServer(const char *buffer, int sockfd, int bufferLength);

void connectServerAndClient(int clientSocket, int serverSocket);

void *handleNewClient(void *cliSocketFromOS);

int main(int argc, char *argv[]);