#include "proxy.h"

bool IsValidNumber(char *string)
{
  for (int i = 0; i < (int)strlen(string); i++)
  {
    if (string[i] < 48 || string[i] > 57)
      return false;
  }

  return true;
}

void debug(const char *format, ...)
{
  va_list args;
  if (DEBUG)
  {
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
  }
}

userRequestInfo parseUserString(string request)
{
  userRequestInfo requestStruct = userRequestInfo();

  string req_msg = string(request);

  int reqMsgLen = req_msg.length();
  int indexSlash[reqMsgLen];
  int indexSpace[reqMsgLen];
  int indexReturn[reqMsgLen];
  int indexColon = 0;
  int counterSlash = 0;
  int counterSpace = 0;
  int counterReturn = 0;
  int counterColon = 0;

  //find all spaces, slashes, and returns
  for (int i = 0; i < reqMsgLen; i++)
  {
    if (request[i] == '/')
    {
      indexSlash[counterSlash] = i;
      counterSlash++;
    }
    if (request[i] == ' ')
    {
      indexSpace[counterSpace] = i;
      counterSpace++;
    }
    if (request[i] == '\n')
    {
      indexReturn[counterReturn] = i;
      counterReturn++;
    }
  }

  if (counterSlash < 4)
  {
    debug("Too few slashes!"); // Should have at least http://.../ and HTTP/...
    requestStruct.isError = true;
    return requestStruct;
  }

  if (indexSlash[1] > indexSpace[1] && indexSlash[1] < indexSpace[0])
  {
    debug("Wrong order of spaces and slashes!");
    requestStruct.isError = true;
    return requestStruct;
  }

  // truncate request message
  string protocol = PATTERN_TWO;
  size_t protocol_size = protocol.length();
  size_t header_length = 0;
  string trunc1 = req_msg.substr(0, indexSlash[1]);                                     // trunc1 contains method
  string trunc2 = req_msg.substr(indexSlash[1] + 1, indexSpace[1] - indexSlash[1] - 1); // trunc2 contains hostname and port number
  string trunc3 = req_msg.substr(indexSpace[1] + 1, protocol_size);                     // trunc3 contains protocol and version
  string trunc4;

  if (counterReturn > 2)
  {
    header_length = indexReturn[counterReturn - 1] - indexReturn[0] - 4;
    trunc4 = req_msg.substr(indexReturn[0] + 1, header_length);
  } // trunc4 contains headers

  //validate method and URL header
  if (trunc1.compare(PATTERN_ONE) != 0)
  {
    debug("Wrong method or http header!");
    requestStruct.isError = true;
    return requestStruct;
  }
  else
  {
    requestStruct.method = METHOD;
  }

  //validate protocol and version
  if (trunc3.compare(PATTERN_TWO) != 0)
  {
    debug("Wrong protocol!");
    requestStruct.isError = true;
    return requestStruct;
  }
  else
  {
    requestStruct.protocol = PROTOCOL;
    requestStruct.version = VERSION;
  }

  char *trunc2c = new char[trunc2.length() + 1];
  strcpy(trunc2c, trunc2.c_str());
  //find index and occurrence of colon
  counterSlash = 0;
  for (int i = 0; i < (int)trunc2.length(); i++)
  {
    if (trunc2c[i] == ':')
    {
      indexColon = i;
      counterColon++;
    }
    if (counterSlash == 0 && trunc2c[i] == '/')
    {
      indexSlash[0] = i;
      counterSlash++;
    }
  }

  // based on results, we define port number for user
  if (counterColon > 2)
  {
    debug("Wrong port number in URL!");
    requestStruct.isError = true;
    return requestStruct;
  }

  requestStruct.hostname = trunc2.substr(0, indexSlash[0]);

  if (requestStruct.hostname.length() == 0)
  {
    debug("Empty hostname!");
    requestStruct.isError = true;
    return requestStruct;
  }

  if (counterColon == 0)
  {
    requestStruct.port = DEFALUT_PORT;
    requestStruct.path = trunc2.substr(indexSlash[0], trunc2.length());
  }
  else if (counterColon == 1)
  {

    requestStruct.port = trunc2.substr(indexColon + 1, trunc2.length());
    requestStruct.path = trunc2.substr(indexSlash[0], indexColon - indexSlash[0]);

    if (indexColon == (int)trunc2.length())
    {
      debug("invalid port number!");
      requestStruct.isError = true;
      return requestStruct;
    }

    char *portc = new char[requestStruct.port.length() + 1];
    strcpy(portc, requestStruct.port.c_str());

    if (!IsValidNumber(portc))
    {
      debug("invalid port number!");
      requestStruct.isError = true;
      return requestStruct;
    }
  }
  else
  {
    debug("invalid port number!");
    requestStruct.isError = true;
    return requestStruct;
  }

  // processing headers
  if (counterReturn > 2)
  {
    istringstream f(trunc4);
    string line;

    while (getline(f, line))
    {
      string::iterator end_pos = remove(line.begin(), line.end(), ' ');
      line.erase(end_pos, line.end());

      size_t n = count(line.begin(), line.end(), ':');
      if (n != 1)
      {
        //error
      }
      else
      {
        string header = line.substr(0, line.find(':'));
        size_t value_len = line.length() - line.find(':') - 2;
        string value = line.substr(line.find(':') + 1, value_len);
        if (header.compare("Connection") == 0 || header.compare("connection") == 0)
        {
          requestStruct.headers.insert(make_pair("Connection", "close"));
        }
        else
        {
          if (value.length() < 50) {
            requestStruct.headers.insert(make_pair(header, value));
          }
        }
      }
    }
  }

  return requestStruct;
}

int getSocket(char *hostname, char *port)
{
  struct addrinfo hints;
  struct addrinfo *resolvedHostAddr;

  int iSockfd;

  // Set aside memory for the hints protocol for hostname resolution
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  if (getaddrinfo(hostname, port, &hints, &resolvedHostAddr) != 0)
  {
    debug("Error with dns resolution \n");
    return -1;
  }

  // Establish socket and connect to host
  if ((iSockfd = socket(resolvedHostAddr->ai_family, resolvedHostAddr->ai_socktype, resolvedHostAddr->ai_protocol)) < 0)
  {
    debug("Error creating socket\n");
    return -1;
  }
  if (connect(iSockfd, resolvedHostAddr->ai_addr, resolvedHostAddr->ai_addrlen) < 0)
  {
    debug("Error connecting to server\n");
    return -1;
  }

  freeaddrinfo(resolvedHostAddr);

  return iSockfd;
}

void sendToServer(const char *buffer, int sockfd, int bufferLength)
{
  string s;
  s.append(buffer);

  int totalsent = 0;
  int senteach;

  while (totalsent < bufferLength)
  {
    if ((senteach = send(sockfd, (void *)(buffer + totalsent), bufferLength - totalsent, 0)) < 0)
    {
      fprintf(stderr, " Error in sending to server ! \n");
      return;
    }
    totalsent += senteach;
  }
}

void sendToClient(const char *buffer, int sockfd, int bufferLength)
{
  string s;
  s.append(buffer);

  int totalsent = 0;

  int senteach;

  while (totalsent < bufferLength)
  {
    if ((senteach = send(sockfd, (void *)(buffer + totalsent), bufferLength - totalsent, 0)) < 0)
    {
      fprintf(stderr, " Error in sending to server ! \n");
    }
    totalsent += senteach;
  }
}

void connectServerAndClient(int clientSocket, int serverSocket)
{
  int bitsReceived;
  char buf[MAX_MESSAGE];
  while ((bitsReceived = recv(serverSocket, buf, MAX_MESSAGE, 0)) > 0)
  {
    sendToClient(buf, clientSocket, bitsReceived); // writing to client
    memset(buf, 0, sizeof buf);
  }

  if (bitsReceived < 0)
  {
    debug("Error from server\n");
  }
}

void send500ToClient(int cliSocket)
{
  string HTTP_500 = string("HTTP/1.0 500 Internal Error");
  sendToClient((char *)HTTP_500.c_str(), cliSocket, HTTP_500.length());
  close(cliSocket);
  sem_post(&maxConcurrent); // Allow more users in
}

void *handleNewClient(void *arg)//*cliSocketFromOS)
{
  int sock = *(int*)arg; // Dereference pointer so local copy of sock num is held.
  delete (int*)arg;

  int LOCAL_MAX_MESSAGE = 10000;
  char buffer[LOCAL_MAX_MESSAGE];
  char *reqMessage; // Get message from URL

  string rm;

  reqMessage = (char *)malloc(LOCAL_MAX_MESSAGE);

  if (reqMessage == NULL)
  {
    debug(" Error in memory allocation ! \n");
  }

  int totalReceived = 0;

  while (strstr(reqMessage, "\r\n\r\n") == NULL)
  {

    int recvd = recv(sock, buffer, MAX_MESSAGE, 0);

    if (recvd < 0)
    {
      fprintf(stderr, " Error while recieving ! \n");
      break;
    }
    else if (recvd == 0)
    {
      break;
    }
    else
    {

      totalReceived += recvd;

      buffer[recvd] = '\0';
      if (totalReceived > LOCAL_MAX_MESSAGE)
      {
        LOCAL_MAX_MESSAGE *= 2;
        reqMessage = (char *)realloc(reqMessage, LOCAL_MAX_MESSAGE);

        if (reqMessage == NULL)
        {
          debug(" Error in memory re-allocation!\n");
        }
      }
    }

    strcat(reqMessage, buffer);
    rm += string(buffer);
  }

  free(reqMessage); // Free memory

  userRequestInfo serverInfo = parseUserString(rm);

  if (serverInfo.isError == true)
  { // Error condition met. Return a 500 to the user.
    send500ToClient(sock);
    return NULL;
  }

  int iServerfd;

  iServerfd = getSocket((char *)serverInfo.hostname.c_str(), (char *)serverInfo.port.c_str());

  if (iServerfd < 0)
  {
    send500ToClient(sock);
    return NULL;
  }
  // Build request for remote server
  // GET http://{Host}{Path} {HTTP VERION}
  // Host: {hostname}
  // Connection: close
  // Extra: Headers
  debug("HOSTNAME: %s\n", serverInfo.hostname.c_str());
  string fullHTTPBody = serverInfo.method + string(" http://")  + serverInfo.hostname + serverInfo.path + string(" ") + serverInfo.protocol + string("/") + serverInfo.version + string("\r\n\r\n") + string("Host: ") + serverInfo.hostname + string("\r\n\r\n");

  serverInfo.headers.insert(make_pair("Connection", "close")); // Force connection close

  for (auto const &header : serverInfo.headers)
  {
    fullHTTPBody += header.first + string(": ") + header.second + string("\r\n\r\n");
  }

  debug(fullHTTPBody.c_str());
  try {
    sendToServer((char *)fullHTTPBody.c_str(), iServerfd, totalReceived);
    connectServerAndClient(sock/*clientSocket*/, iServerfd);
  } catch (const std::exception& e) {
    debug("Error with attempt to  connect send");
  }

  sem_post(&maxConcurrent); // Allow more users in
  close(sock/*clientSocket*/); // close the sockets
  close(iServerfd);

  return NULL;
}


int main(int argc, char *argv[])
{
  /**
  pthread_t tid;
  pthread_attr_t attr;
*/

  if (argc < 2)
  {
    cerr << "Proxy must be started with a port! Ex: ./proxy 9000" << endl;
    exit(-1);
  }


  struct sockaddr_in serv_addr;
  //struct sockaddr cli_addr;

  int sock = socket(AF_INET, SOCK_STREAM, 0); // create a socket

  if (sock < 0)
  {
    cerr << "Unable to bind to port" << endl;
    exit(-1);
  }

  memset(&serv_addr, 0, sizeof serv_addr);

  int portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno); // Convert string to network order short

  if (::bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    fprintf(stderr, "Error on binding! \n");
    return 1;
  }

  debug("Server now listening on port %d\n", portno);

  if (listen(sock, MAX_CONCURRENT_USERS) <0)
  {
    fprintf(stderr,"Error on listening!\n");
  }

  struct sockaddr_in client;
  memset(&client, 0, sizeof(client));
  socklen_t clientSize = sizeof(client);

  sem_init(&maxConcurrent, 0, MAX_CONCURRENT_USERS); // Only allow 30 users at once.
  while (1)
  {
    sem_wait(&maxConcurrent); // Wait for available processor before accepting request. Only 30 for now.
    int newSock = accept(sock, (struct sockaddr*)&client, &clientSize);

    if (newSock < 0) {
      cerr << "Error with incoming message, ignoring request" << endl;
    } else {
      pthread_t clientThread;

      pthread_create(&clientThread, NULL, &handleNewClient, (void*) new int(newSock));
      pthread_detach(clientThread);
    }
  }

  sem_close(&maxConcurrent);
  return 0;
}