#include "quick_dhcp.h"

int udp_test(){
    SOCKET raw_s = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if(raw_s == INVALID_SOCKET)
    {
        printf("Error creating raw socker : %d\n", WSAGetLastError());
        return -1;
    }
    printf("UDP Socket Created\n");

    // bind socket to the network interface
    sockaddr_in my_nic;
    my_nic.sin_family = AF_INET;
    my_nic.sin_addr.s_addr = inet_addr("192.168.1.150");
    my_nic.sin_port = 68;
    int err = bind(raw_s, (SOCKADDR*)&my_nic, sizeof(my_nic));
    if(err != 0){
        printf("Error binding raw socker %d\n", err);
        return -1;
    }
    printf("Raw socket bound\n");

    bool optval = TRUE;
    err = setsockopt(raw_s, SOL_SOCKET, SO_BROADCAST, (char*)&optval, sizeof(BOOL)); // allow socket to send broadcasts
    if(err != 0)
    {
        printf("error setting sockopt : %d\n", err);
        return -1;
    }
    printf("Sockopt set\n");

    sockaddr_in receiver;               // create broadcast destination 
    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(68);
    receiver.sin_addr.s_addr = inet_addr("255.255.255.255");

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Create a udp packet

    char test_sendbuff[512];

    udp_packet *test_pkt = (udp_packet*)test_sendbuff;
    // udp header
    test_pkt->src_port = htons(67);
    test_pkt->dest_port = htons(68);
    test_pkt->len = htons(32);
    test_pkt->checksum = htons(32);
    // dhcp payload
    test_pkt->payload.op = 0x02;
    test_pkt->payload.htype = 0x01;
    test_pkt->payload.hlen = 0x06;
    test_pkt->payload.hops = 0x00;
    test_pkt->payload.xid = 0x12345678;
    test_pkt->payload.segs = 0x0001;
    test_pkt->payload.flags = 0x0;
    test_pkt->payload.ciaddr = 0x0;
    test_pkt->payload.yiaddr = inet_addr("192.168.1.150");
    test_pkt->payload.siaddr = inet_addr("192.168.1.150");

    err = sendto(raw_s, test_sendbuff, sizeof(test_sendbuff), 0, (SOCKADDR *)&receiver, sizeof(receiver));
    if(err != 0)
    {
        printf("UDP Bytes Sent : %d\n", err);
    }
    return 0;
}