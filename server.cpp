#include <Winsock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <Winsock.h>
#include <iphlpapi.h>
#include <stdio.h>

#define DHCP_PORT "67"

int main(){
    WSADATA wsa;
    SOCKET s;

    int iResult;

    /////////////////////////////////////////////////////////////////////////////////////

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsa);
    if(iResult != 0)
    {
        printf("WSAStartup Failed : %d\n", iResult);
        return 1;
    }

    // Resolve address information for socket.
    // A "hint" is created with our known parameters. It is passed to getaddrinfo
    // along with an emptry addrinfo, where the kernel allocated parameters get filled in.
    struct addrinfo *result = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET; // ipv4
    hints.ai_socktype = SOCK_STREAM; // Two way connection, TCP. SOCK_DGRAM is UDP
    hints.ai_protocol = IPPROTO_TCP; // TCP Protocol, UDP is IPPROTO_UDP
    hints.ai_flags = AI_PASSIVE; // this addrinfo is going to be used with bind

    // getaddrinfo(string host addr, string port, hint, return)
    // when the node name is set to NULL, and the hints.ai_flags is AI_PASSIVE, the 
    // ip address of the returned structure is set to INADDR_ANY
    // 
    // our node is localhost for testing purposes
    iResult = getaddrinfo("localhost", DHCP_PORT, &hints, &result);
    if(iResult != 0)
    {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    s = INVALID_SOCKET;
    // socket( ipv4, tcp type, tcp protocol)
    s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(s == INVALID_SOCKET)
    {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // connect the acquired address to the created socket
    // in either condition, free addr info, since our socket is created.
    iResult = bind(s, result->ai_addr, (int)result->ai_addrlen);
    if(iResult == SOCKET_ERROR)
    {
        printf("Bind failed with error %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(s);
        WSACleanup();
        return 1;
    }
    freeaddrinfo(result); 

    ////////////////////////////////////////////////////////////////////////////////////

    // Listen on socket
    if(listen( s, SOMAXCONN) == SOCKET_ERROR)
    {
        printf("Listen failed error : %ld\n", WSAGetLastError());
        closesocket(s);
        WSACleanup();
        return 1;
    }

    // Accept a client
    // Accept will wait until a connection is made
    SOCKET client = INVALID_SOCKET;
    client = accept(s, NULL, NULL);
    if(client == INVALID_SOCKET)
    {
        printf("failed accept %d\n", WSAGetLastError());
        closesocket(s);
        WSACleanup();
        return 1;
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    // client is connected

    #define DEFAULT_BUFLEN 512

    char recvbuf[DEFAULT_BUFLEN];
    int iSendResult;
    
    do {
        // this call blocks until data is received
        iResult = recv(client, recvbuf, DEFAULT_BUFLEN, 0);
        
        if(iResult > 0) 
        {
            printf("Bytes Received: %d\n", iResult);

            iSendResult = send(client, recvbuf, iResult, 0);
            if(iSendResult == SOCKET_ERROR) 
            {
                printf("send failed: %d\n", WSAGetLastError());
                closesocket(client);
                WSACleanup();
                return 1;
            }
            printf("Bytes Sent : %d\n", iSendResult);
        } 
        else if (iResult == 0)
        {
            printf("Closing Connection....\n");
        }
        else 
        {
            printf("recv failed: %d\n", WSAGetLastError());
            closesocket(client);
            WSACleanup();
            return 1;
        }
    } while(iResult > 0);

    printf("done.\n");
}