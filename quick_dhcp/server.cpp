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
    s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(s == INVALID_SOCKET)
    {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return -1;
    }
    printf("TCP Socket Created\n");

    // bind the socket to our network interface
    iResult = bind(s, result->ai_addr, (int)result->ai_addrlen);
    if(iResult == SOCKET_ERROR)
    {
        printf("Bind failed with error %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(s);
        return -1;
    }
    freeaddrinfo(result); 

    // set sock opt to receive broadcast
    bool optval = TRUE;
    iResult = setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char*)&optval, sizeof(BOOL));
    if(iResult != 0)
    {
        printf("Error setting sockopt for receive : %d\n", iResult);
        return -1;
    }

    // Create the address profile of the client 
    sockaddr_in client;
    client.sin_family = AF_INET;
    client.sin_port = htons(67);
    client.sin_addr.s_addr = inet_addr("255.255.255.255");
    
    // Listen on socket
    printf("Waiting for DHCP Discovery.....\n");
    
    char buf[512];
    int clsize = sizeof(client);
    iResult = recvfrom(s, buf, sizeof(buf), 0, (sockaddr *)&client, &clsize);
    if(iResult > 0)
    {
        printf("Bytes Received : %d\n", iResult);
        printf("Buf : \n%s\n", buf);
    }else if( iResult < 0) {
        printf("Recv Error : %d\n", iResult);
        return -1;
    }

    printf("done.\n");
    return 0;
}