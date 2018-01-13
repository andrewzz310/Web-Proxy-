// Andrew Zhu, Aaron Pollack, Siyao Xu


Running the Program:
  The project consists of the main cpp file [proxy.cpp], a header file [proxy.h], a MakeFile and a README.
  In this phase, we start by compiling the program using running 'make clean' into the command line. Next
  we will compile the program by running 'make'. We will establish the server connection by running
  ./proxy <port#> into the command line. With a new shell window opened, run 'telnet localhost <port#>'.
  The client connection would then be established with the proxy. You can now request a page with the
  proxy by running GET <space> http://www.google.com/index.html <space> HTTP/1.0. This will return the headers and html
  of the home page back to the client based on the request that was passed into the proxy server.



Code Design Decisions and Contributions:
  Each group member contributed equally as our team met regularly to discuss, contribute, and debug
  accordingly. We used version control through a private github repository. When together, no one
  person would be assigned a definitive task, but rather we often used sprints collectively and divided
  tasks into subcomponents to save time, an example: Yiyun and Siyao worked on parsing the user request
  input to be sent into the proxy server. Aaron, then worked on combining the user request to be sent
  to the web host and then passed back to the client. We choose to eagerly send a 500 back to the user before attempting to establish   the connection with the server, that way we don't use anymore resources than necessary. We also used the getaddrinfo unix call to resolve the server host to an IP. This will do it's best to resolve either the FQDN (ex www.google.com) or just the minimum domain google.com.

  The design descisions were made by using p_threads to hold all information from the client. In order to ensure concurrency of a maximum of 30 users,
  we decided to use Semaphores. If requirements were not satisfied by the user or input was incorrect,
  an standard error 500 message was returned to the client. Any sort of error from bad user input to the inability to connect to the remote host will come back to the user as 500 Internal Error. On the server, you can set `#define DEBUG 1` to enable debug level logging. This is disabled by default.




Notes and Considerations:
1) Port #chosen is between 8001-8999
2) Performance only considered up to a maximum of 30 users.
3) Our Test Cases light pages (firefox):
  http://www.google.com/
  http://www.yahoo.com/
  http://www.washington.edu/
  http://www.msnbc.com/
  http://www.theuselessweb.com/

4) Our Test Cases heavy pages (firefox):
  http://www.foxnews.com/
  http://www.sina.com.cn/
  http://abcnews.go.com/
  http://pbs.org/
