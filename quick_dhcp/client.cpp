#include <Winsock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <iphlpapi.h>
#include <stdio.h>

#define DHCP_PORT "67"

// Have an example client to simulate a dhcp requesting device
// this will be more concise than the server implementation

int main()
{
    WSADATA wsa;
    SOCKET s;

    int iResult;

    // startup wsa
    iResult = WSAStartup(MAKEWORD(2, 2), &wsa);
    if(iResult != 0)
    {
        printf("WSA Startup Failed %d\n", iResult);
        return 1;
    }

    // get a free address n socket
    struct addrinfo *result = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // get the addrinfo of the server we want to connect
    iResult = getaddrinfo("localhost", "67", &hints, &result);
    if(iResult != 0)
    {
        printf("getaddrinfo failed : %d\n", iResult);
        WSACleanup();
        return 1;
    }

    s = INVALID_SOCKET;
    s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(s == INVALID_SOCKET)
    {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

//////////////////////////////////////////////////////////////////////////////////////////////////////

    // connect to server
    iResult = connect(s, result->ai_addr, (int)result->ai_addrlen);
    if(iResult == SOCKET_ERROR) {
        printf("Connect failed\n");
        closesocket(s);
        s = INVALID_SOCKET;
        return 1;
    }

    // if we were more ambiguous about the addrinfo, the connect call might fail a couple times,
    // and we would iterate through the result ptr. to try new addresses
    freeaddrinfo(result);


//////////////////////////////////////////////////////////////////////////////////////////////////////

    #define RECVMAX 512
    int recvbuflen = RECVMAX;

    const char *sendbuf = "this is a test";
    char recvbuf[RECVMAX];

    iResult = send(s, sendbuf, (int)strlen(sendbuf), 0);
    if(iResult == SOCKET_ERROR)
    {
       printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1; 
    }

    printf("bytes sent : %d\n", iResult);
}