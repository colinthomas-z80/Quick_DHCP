#include "quick_dhcp.h"

extern char *host_ip;
extern char *offer_ip;

USHORT client_mac_low;
USHORT client_mac_mid;
USHORT client_mac_hi;
ULONG32 transaction_id;

int init_rx_socket(SOCKET *s_ptr)
{
    int iResult;

    // Resolve address information for socket.
    // We need a socket that can listen for broadcasts
    struct addrinfo *result = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM; 
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;

   
    // get address profile of our host 
    iResult = getaddrinfo(host_ip, DHCP_PORT_RX, &hints, &result);
    if(iResult != 0)
    {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return -1;
    }

    // create socket
    *s_ptr = INVALID_SOCKET;
    *s_ptr = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(*s_ptr == INVALID_SOCKET)
    {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return -1;
    }

    // bind the socket to our network interface
    iResult = bind(*s_ptr, result->ai_addr, (int)result->ai_addrlen);
    if(iResult == SOCKET_ERROR)
    {
        printf("Bind failed with error %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(*s_ptr);
        return -1;
    }
    freeaddrinfo(result); 

    // set sock opt to receive broadcast
    bool optval = TRUE;
    iResult = setsockopt(*s_ptr, SOL_SOCKET, SO_BROADCAST, (char*)&optval, sizeof(BOOL));
    if(iResult != 0)
    {
        printf("Error setting sockopt for receive : %d\n", iResult);
        return -1;
    }

    printf("RX Socket Created\n");
    return 0;
}

int init_tx_socket(SOCKET *s_ptr)
{
    *s_ptr = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(*s_ptr == INVALID_SOCKET)
    {
        printf("Error creating raw socker : %d\n", WSAGetLastError());
        return -1;
    }
    //printf("UDP Socket Created\n");

    // bind socket to the network interface
    sockaddr_in my_nic;
    my_nic.sin_family = AF_INET;
    my_nic.sin_addr.s_addr = inet_addr(host_ip);
    my_nic.sin_port = 67;
    int err = bind(*s_ptr, (SOCKADDR*)&my_nic, sizeof(my_nic));
    if(err != 0){
        printf("Error binding tx socket %d\n", err);
        return -1;
    }
    //printf("Raw socket bound\n");

    bool optval = TRUE;
    err = setsockopt(*s_ptr, SOL_SOCKET, SO_BROADCAST, (char*)&optval, sizeof(BOOL)); // allow socket to send broadcasts
    if(err != 0)
    {
        printf("error setting sockopt tx: %d\n", err);
        return -1;
    }

    printf("TX Socket Created\n");
    return 0;
}

int send_dhcp_offer(SOCKET *s_ptr){
    int err;

    sockaddr_in receiver;               // create broadcast destination 
    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(68);
    receiver.sin_addr.s_addr = inet_addr("255.255.255.255");

    // Create a udp packet

    char test_sendbuff[512];
    ZeroMemory(test_sendbuff, 512);

    dhcp_payload *test_pkt = (dhcp_payload*)test_sendbuff;
    test_pkt->op = 0x02;
    test_pkt->htype = 0x01;
    test_pkt->hlen = 0x06;
    test_pkt->hops = 0x00;
    test_pkt->xid = htonl(transaction_id);
    test_pkt->segs = 0x0000;
    test_pkt->flags = 0x0;
    test_pkt->ciaddr = 0x0;
    test_pkt->yiaddr = inet_addr(offer_ip);
    test_pkt->siaddr = 0x0;
    test_pkt->giaddr = 0x0;
    test_pkt->chaddr_first = htons(client_mac_low);
    test_pkt->chaddr_second = htons(client_mac_mid);
    test_pkt->chaddr_third = htons(client_mac_hi);
    test_pkt->magic = inet_addr(DHCP_MAGIC_COOKIE);
    
    char *option_ptr = test_pkt->option_ptr;

    *option_ptr++ = 53; // dhcp offer
    *option_ptr++ = 1;
    *option_ptr++ = 2;

    *option_ptr++ = 1; // subnet mask
    *option_ptr++ = 4;
    *option_ptr++ = 0xFF;
    *option_ptr++ = 0xFF;
    *option_ptr++ = 0xFF;
    *option_ptr++ = 0x0;

    *option_ptr++ = 3; // router
    *option_ptr++ = 4;
    *option_ptr++ = 192;
    *option_ptr++ = 168;
    *option_ptr++ = 1;
    *option_ptr++ = 0;

    *option_ptr++ = 51; // lease time
    *option_ptr++ = 4;
    *option_ptr++ = 0x0;
    *option_ptr++ = 0x0;
    *option_ptr++ = 0xFF;
    *option_ptr++ = 0xFF;
    
    *option_ptr++ = 0; // padding (not necessary)
    *option_ptr++ = 0;

    *option_ptr++ = 54; // dhcp server
    *option_ptr++ = 4;
    *option_ptr++ = 192;
    *option_ptr++ = 168;
    *option_ptr++ = 1;
    *option_ptr++ = 150;

    *option_ptr++ = 0xFF; // end
    

    err = sendto(*s_ptr, test_sendbuff, sizeof(test_sendbuff), 0, (SOCKADDR *)&receiver, sizeof(receiver));
    if(err != 0)
    {
        printf("UDP Bytes Sent : %d\n", err);
    }else{
        printf("UDP Send Error : %d\n", err);
    }
    return 0;
}

int client_ack(SOCKET *s_ptr){
    int err;

    sockaddr_in receiver;               // create broadcast destination 
    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(68);
    receiver.sin_addr.s_addr = inet_addr("255.255.255.255");

    // Create a udp packet

    char test_sendbuff[512];
    ZeroMemory(test_sendbuff, 512); // need to "allocate" because of the indeterminate length of option

    dhcp_payload *test_pkt = (dhcp_payload*)test_sendbuff;
    test_pkt->op = 0x02;
    test_pkt->htype = 0x01;
    test_pkt->hlen = 0x06;
    test_pkt->hops = 0x00;
    test_pkt->xid = htonl(transaction_id);
    test_pkt->segs = 0x0000;
    test_pkt->flags = 0x0;
    test_pkt->ciaddr = 0x0;
    test_pkt->yiaddr = inet_addr(offer_ip);
    test_pkt->siaddr = 0x0;
    test_pkt->giaddr = 0x0;
    test_pkt->chaddr_first = htons(client_mac_low);
    test_pkt->chaddr_second = htons(client_mac_mid);
    test_pkt->chaddr_third = htons(client_mac_hi);
    test_pkt->magic = inet_addr(DHCP_MAGIC_COOKIE);

    // Instead of this pointer hack, I could use a large amount of padding each option, exceeding
    // any potential size of info

    char *option_ptr = test_pkt->option_ptr;

    *option_ptr++ = 53; // dhcp ack
    *option_ptr++ = 1;
    *option_ptr++ = 5;

    *option_ptr++ = 1; // subnet mask
    *option_ptr++ = 4;
    *option_ptr++ = 0xFF;
    *option_ptr++ = 0xFF;
    *option_ptr++ = 0xFF;
    *option_ptr++ = 0x0;

    *option_ptr++ = 3; // router
    *option_ptr++ = 4;
    *option_ptr++ = 192;
    *option_ptr++ = 168;
    *option_ptr++ = 1;
    *option_ptr++ = 0;

    *option_ptr++ = 51; // lease time
    *option_ptr++ = 4;
    *option_ptr++ = 0x0;
    *option_ptr++ = 0x0;
    *option_ptr++ = 0xFF;
    *option_ptr++ = 0xFF;
    
    *option_ptr++ = 0; // padding (not necessary)
    *option_ptr++ = 0;

    *option_ptr++ = 54; // dhcp server
    *option_ptr++ = 4;
    *option_ptr++ = 192;
    *option_ptr++ = 168;
    *option_ptr++ = 1;
    *option_ptr++ = 150;

    *option_ptr++ = 0xFF; // end
    

    err = sendto(*s_ptr, test_sendbuff, sizeof(test_sendbuff), 0, (SOCKADDR *)&receiver, sizeof(receiver));
    if(err != 0)
    {
        printf("UDP Bytes Sent : %d\n", err);
    }
    return 0;
}

int discover_client(SOCKET *s_ptr)
{
    int iResult;

    // Create the address profile of the client 
    sockaddr_in client;
    client.sin_family = AF_INET;
    client.sin_port = htons(67);
    client.sin_addr.s_addr = inet_addr("255.255.255.255");
    
    // Listen on socket
    printf("Waiting for DHCP Discovery.....\n");
    
    char buf[512];
    int clsize = sizeof(client);
    iResult = recvfrom(*s_ptr, buf, sizeof(buf), 0, (sockaddr *)&client, &clsize);
    if(iResult > 0)
    {
        printf("Bytes Received : %d\n", iResult);
    }else if( iResult < 0) {
        printf("Recv Error : %d\n", iResult);
        return -1;
    }

    // interpret the discover packet
    // udp header is not in buffer because the socket is already udp type, not raw.
    dhcp_payload *received = (dhcp_payload*)buf;

    // recurse until a discover packet is caught.
    if(received->option_ptr[OFFSET_DHCP_MSG_TYPE] != DHCP_MSG_TYPE_DISCOVER) 
    {
        printf("Message from client was not dhcp discover! : %d\n", received->option_ptr[OFFSET_DHCP_MSG_TYPE]);
        discover_client(s_ptr);
    }
    else
    {

    client_mac_low = ntohs(received->chaddr_first);
    client_mac_mid = ntohs(received->chaddr_second);
    client_mac_hi = ntohs(received->chaddr_third);

    transaction_id = ntohl(received->xid);

    printf("\n---------------- DHCP Discover Packet Contents ------------------\n");
    printf("Message Type : %X\n", received->op);
    printf("Hardware Type : %X\n", received->htype);
    printf("Address Length : %X\n", received->hlen);
    printf("Hops : %X\n", received->hops);
    printf("Transaction ID : %X\n", ntohl(received->xid));
    printf("Magic Cookie : %X\n", received->magic);
    printf("Mac Last Nib : %X\n", client_mac_hi);
    printf("DHCP Message Type : %d\n", received->option_ptr[OFFSET_DHCP_MSG_TYPE]);
    printf("done.\n");
    }
    return 0;
}

int client_request(SOCKET *s_ptr)
{
    int iResult;

    // Create the address profile of the client 
    sockaddr_in client;
    client.sin_family = AF_INET;
    client.sin_port = htons(67);
    client.sin_addr.s_addr = inet_addr("255.255.255.255");
    
    // Listen on socket
    printf("Waiting for DHCP Request.....\n");
    
    char buf[512];
    int clsize = sizeof(client);
    iResult = recvfrom(*s_ptr, buf, sizeof(buf), 0, (sockaddr *)&client, &clsize);
    if(iResult > 0)
    {
        printf("Bytes Received : %d\n", iResult);
    }else if( iResult < 0) {
        printf("Recv Error : %d\n", iResult);
        return -1;
    }

    // interpret the discover packet
    dhcp_payload *received = (dhcp_payload*)buf;

    USHORT mac_low = ntohs(received->chaddr_first);
    USHORT mac_mid = ntohs(received->chaddr_second);
    USHORT mac_hi = ntohs(received->chaddr_third);

    printf("\n---------------- DHCP REQ Packet Contents ------------------\n");
    printf("Message Type : %X\n", received->op);
    printf("Hardware Type : %X\n", received->htype);
    printf("Address Length : %X\n", received->hlen);
    printf("Hops : %X\n", received->hops);
    printf("Transaction ID : %X\n", ntohl(received->xid));
    printf("Magic Cookie : %X\n", received->magic);
    printf("Mac Last Nib : %X\n", mac_hi);
    printf("DHCP Message Type : %d\n", received->option_ptr[OFFSET_DHCP_MSG_TYPE]);

    // Need to verify that this packet is a dhcp request, and from the original client
    if(mac_hi & client_mac_hi != mac_hi || received->option_ptr[OFFSET_DHCP_MSG_TYPE] != DHCP_MSG_TYPE_REQUEST)
    {
        printf("Packet Received was not request or not from discover client.\n");
        return 1;
    }

    printf("done.\n");
    return 0;
}