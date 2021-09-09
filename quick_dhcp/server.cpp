#include "quick_dhcp.h"

#define DHCP_PORT "67"

int get_client_discover();

int main(){
    WSADATA wsa;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsa);
    if(iResult != 0)
    {
        printf("WSAStartup Failed : %d\n", iResult);
        return 1;
    }

    // Listen for a DHCP Discover 
    iResult = get_client_discover();
    if(iResult != 0)
    {
        printf("Error Listening for DHCP Discover : %d\n", iResult);
        WSACleanup();
        return 1;
    }
    
    // Create a RAW socket for server responses
    // iResult = udp_test();
    // if(iResult != 0)
    // {
    //     printf("Error Sending UDP Response : %d\n", iResult);
    //     WSACleanup();
    //     return 1;
    // }
    
    return 0;
}

int get_client_discover()
{
    SOCKET s;
    int iResult;

    // Resolve address information for socket.
    // We need a socket that can listen for broadcasts
    struct addrinfo *result = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM; 
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;

    // getaddrinfo(string host addr, string port, hint, return)
    // when the node name is set to NULL, and the hints.ai_flags is AI_PASSIVE, the 
    // ip address of the returned structure is set to INADDR_ANY
    // 
    iResult = getaddrinfo("192.168.1.150", DHCP_PORT, &hints, &result);
    if(iResult != 0)
    {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return -1;
    }

    s = INVALID_SOCKET;
    // socket( ipv4, tcp type, tcp protocol)
    s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(s == INVALID_SOCKET)
    {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return -1;
    }
    printf("TCP Socket Created\n");

    // connect the acquired address to the created socket
    // in either condition, free addr info, since our socket is created.
    iResult = bind(s, result->ai_addr, (int)result->ai_addrlen);
    if(iResult == SOCKET_ERROR)
    {
        printf("Bind failed with error %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(s);
        WSACleanup();
        return -1;
    }
    freeaddrinfo(result); 

    // Listen on socket
    printf("Waiting for DHCP Discovery.....\n");
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

    // ///////////////////////////////////////////////////////////////////////////////////////

    // // client is connected

    #define DEFAULT_BUFLEN 512

    char recvbuf[DEFAULT_BUFLEN];
    int iSendResult;
    
    do {
        // this call blocks until data is received
        iResult = recv(client, recvbuf, DEFAULT_BUFLEN, 0);
        
        if(iResult > 0) 
        {
            printf("Bytes Received: %d\n", iResult);
            printf("%s\n", recvbuf);
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
    return 0;
}