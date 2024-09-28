#include <iostream>
#include<string>
#include "pcap.h"
#include<iomanip>
#include<cstring>
#pragma comment(lib,"wpcap")
#pragma warning(disable:4996)
using namespace std;
/* packet handler ����ԭ�� */
void packet_handler(u_char* param, const struct pcap_pkthdr* header, const u_char* pkt_data);
//���ܺ���ԭ��
void extractURL(const char* httpPacket); //��ȡƴ��URL
void extractContent(const char* httpResponse, const struct pcap_pkthdr* header, const u_char* pkt_data);//��ȡ��Ӧ������
void ex_extractContent(const char* httpResponse, const struct pcap_pkthdr* header, int headerTotalLen);
/* 4�ֽڵ�IP��ַ */
typedef struct ip_address {
    u_char byte1;
    u_char byte2;
    u_char byte3;
    u_char byte4;
}ip_address;

/* IPv4 �ײ� */
typedef struct ip_header {
    u_char  ver_ihl;        // �汾 (4 bits) + �ײ����� (4 bits)
    u_char  tos;            // ��������(Type of service) 
    u_short tlen;           // �ܳ�(Total length) 
    u_short identification; // ��ʶ(Identification)
    u_short flags_fo;       // ��־λ(Flags) (3 bits) + ��ƫ����(Fragment offset) (13 bits)
    u_char  ttl;            // ���ʱ��(Time to live)
    u_char  proto;          // Э��(Protocol)
    u_short crc;            // �ײ�У���(Header checksum)
    ip_address  saddr;      // Դ��ַ(Source address)
    ip_address  daddr;      // Ŀ�ĵ�ַ(Destination address)
    u_int   op_pad;         // ѡ�������(Option + Padding)
}ip_header;

/*tcp �ײ�*/
typedef struct tcp_header {
    u_short src_port;
    u_short dst_port;
    u_int seq_num;
    u_int ack_num;
    u_short hlen_flags;
    u_short win;
    u_short crc;
    u_short u_point;
}tcp_header;

/*http�ײ�*/
typedef struct http_header {
    u_int protocol; //�������ж���GET������Ӧ����
}http_header;

int  main()
{
    pcap_if_t* alldevs;
    pcap_if_t* d;
    int i = 0;
    char errbuf[PCAP_ERRBUF_SIZE];
    struct bpf_program fcode;

    //����������
    char packet_filter[] = "tcp and host 10.160.2.157 and (src or dst port 80)";


    //��ȡ�豸�б�
    if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) == -1)
    {
        cout << "Error in pacp_findalldevs_ex:" << errbuf << endl;
        exit(1);
    }

    for (d = alldevs; d != NULL; d = d->next)
    {
        cout << endl;
        cout << ++i << " " << d->name << endl;
        if (d->description)
            cout << " <" << d->description << ">" << endl;
        else
            cout << " <No description>" << endl;
    }
    if (i == 0)
    {
        cout << "No interfaces found!" << endl;
        return -1;
    }
    int inum;
    pcap_t* adhandle = nullptr;
    cout << "Enter the interface number ( 1 -- " << i << " ) :" << endl;
    cin >> inum;
    if (inum<1 || inum>i) {
        cout << "\nInterface number out of range.\n";
        pcap_freealldevs(alldevs);
        return -1;
    }

    //�ҵ�Ҫ�����������ָ��
    int j;
    for (d = alldevs, j = 1; j < inum; d = d->next, j++) {}

    //���豸
    cout << d->addresses->netmask << endl;
    if ((adhandle = pcap_open(d->name, 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf)) == NULL) {
        cout << "\nUnable to open the adapter. " << d->name << " is not supported by WinPcap\n";
        pcap_freealldevs(alldevs);
        return -1;
    }

    u_int netmask;
    if (d->addresses != NULL)
        /* ��ýӿڵ�һ����ַ������ */
        netmask = ((struct sockaddr_in*)(d->addresses->netmask))->sin_addr.S_un.S_addr;
    else
        /* ����ӿ�û�е�ַ����ô���Ǽ���һ��B������� */
        netmask = 0xffff0000;
    cout << hex << "netmask:" << netmask << endl;

    //���������
    if (pcap_compile(adhandle, &fcode, packet_filter, 0, netmask) < 0)
    {
        fprintf(stderr, "\nUnable to compile the packet filter. Check the syntax.\n");
        /* �ͷ��豸�б� */
        pcap_freealldevs(alldevs);
        return -1;
    }

    //���ù�����
    if (pcap_setfilter(adhandle, &fcode) < 0)
    {
        fprintf(stderr, "\nError setting the filter.\n");
        /* �ͷ��豸�б� */
        pcap_freealldevs(alldevs);
        return -1;
    }

    cout << "\nListening on " << d->description << "\n";
    pcap_freealldevs(alldevs);
    pcap_loop(adhandle, 0, packet_handler, NULL);

    return 0;
}

/*����û���ҵ��ֳɵĹ��˹��򵥶�����http����tcp����ɸѡ*/

void packet_handler(u_char* param, const struct pcap_pkthdr* header, const u_char* pkt_data) {
    struct tm* ltime;
    char timestr[16];
    time_t local_tv_sec;
    ip_header* ih;
    tcp_header* th;
    u_int ip_len;
    u_int tcp_len;

    /* ���IP���ݰ�ͷ����λ�� */
    ih = (ip_header*)(pkt_data + 14); //��̫��ͷ������

    /*���tcp���ݰ�ͷ��λ��*/
    ip_len = (ih->ver_ihl & 0x0f) * 4;
    th = (tcp_header*)((u_char*)ih + ip_len);

    /* ���http��ʼλ��*/
    th->hlen_flags = ntohs(th->hlen_flags);
    tcp_len = ((th->hlen_flags & 0xf000) >> 12) * 4;
    http_header* http = (http_header*)((u_char*)th + tcp_len);

    //����õ��ܰ�ͷ����
    long int headerTotalLen = static_cast<int>((const u_char*)http - pkt_data + 1);

    if (headerTotalLen < header->len) {
        /*�ж����ݱ����ͣ���GET������Ӧ����*/
        char method[5];
        method[4] = '\0';

        //��ȡhttp��ʼ�ĸ���ĸ
        sprintf(method, "%c%c%c%c", http->protocol & 0xFF, (http->protocol >> 8) & 0xFF, (http->protocol >> 16) & 0xFF, (http->protocol >> 24) & 0xFF);

        //�����GET������
        if (strncmp(method, "GET", 3) == 0) {
            printf("\n%s", method);
            /* ��ʱ���ת���ɿ�ʶ��ĸ�ʽ */
            local_tv_sec = header->ts.tv_sec;
            ltime = localtime(&local_tv_sec);
            strftime(timestr, sizeof timestr, "%H:%M:%S", ltime);

            /* ��ӡ���ݰ���ʱ����ͳ��� */
            printf("%s,%.6d len:%d\n", timestr, header->ts.tv_usec, header->len);


            //��ӡhttp�����е�url�������кͱ�ͷ������urlҪƴ��
            const char* httpGet = (const char*)http;
            extractURL(httpGet);

            printf("------------------GET�����Ľ���----------------------------\n");
        }

        //�������Ӧ���ģ���ͷ��HTTP
        else if ((strncmp(method, "HTTP", 4)) == 0) {
            printf("\n%s Response ", method);
            /* ��ʱ���ת���ɿ�ʶ��ĸ�ʽ */
            local_tv_sec = header->ts.tv_sec;
            ltime = localtime(&local_tv_sec);
            strftime(timestr, sizeof timestr, "%H:%M:%S", ltime);

            /* ��ӡ���ݰ���ʱ����ͳ��� */
            printf("%s,%.6d len:%d\n", timestr, header->ts.tv_usec, header->len);
            const char* httpRes = (const char*)http;

            extractContent(httpRes, header, pkt_data);

            printf("----------------��Ӧ���Ľ���------------------------------\n");
        }

        else if ((strncmp(method, "POST", 4)) == 0);
        else if ((http->protocol & 0xFFFF) == 0x0000);//���HTTPЭ��İ汾���Ƿ�Ϊ1.0

        else {
            printf("\nContinuation ");
            /* ��ʱ���ת���ɿ�ʶ��ĸ�ʽ */
            local_tv_sec = header->ts.tv_sec;
            ltime = localtime(&local_tv_sec);
            strftime(timestr, sizeof timestr, "%H:%M:%S", ltime);

            /* ��ӡ���ݰ���ʱ����ͳ��� */
            printf("%s,%.6d len:%d\n", timestr, header->ts.tv_usec, header->len);
            const char* httpRes = (const char*)http;

            ex_extractContent(httpRes, header, headerTotalLen);

            printf("----------------��Ӧ����������------------------------------\n");
        }
    }
}

void extractURL(const char* httpPacket) {

    // ��ȡ�������е�URL
    const char* requestLineUrlEnd = strstr(httpPacket, "HTTP/1.1");
    std::string requestLine(httpPacket, requestLineUrlEnd);//��httpPacket��requestLineUrlEnd֮����ַ�����requestLine

    // ��ȡHost
    const char* hostHeader = strstr(httpPacket, "Host: ");
    if (hostHeader) {
        const char* hostValueStart = hostHeader + 6;
        const char* hostValueEnd = strchr(hostValueStart, '\r');
        if (hostValueEnd) {
            std::string hostValue(hostValueStart, hostValueEnd);

            // ����ƴ��URL�����
            std::string fullURL = "http://" + hostValue + requestLine.substr(4);
            std::cout << "Full URL: " << fullURL << std::endl;
        }
    }

}

void extractContent(const char* httpResponse, const struct pcap_pkthdr* header, const u_char* pkt_data) {
    // �ҵ�HTTP��Ӧͷ�Ľ�β
    const char* headersEnd = strstr(httpResponse, "\r\n\r\n");

    if (headersEnd) {
        // ��λ��HTTP��Ӧ����λ��
        const char* contentStart = headersEnd + 4;
        long int headerTotalLen = static_cast<int>((const u_char*)contentStart - pkt_data);
        long int contentLen = header->len - headerTotalLen;//���HTTP��Ӧ���ĳ���

        //��ӡHTTP��Ӧ����ʮ�����Ƶ�ֵ
        for (size_t i = 0; i < contentLen; i++)
        {
            unsigned char byteValue = static_cast<unsigned char>(contentStart[i]);

            printf("%02x ", byteValue);
            if ((i + 1) % 16 == 0 || i == contentLen - 1) {
                printf("\n");
            }
        }

        //��ӡHTTP��Ӧ���ĵ�ASCII�ַ�
        for (size_t i = 0; i < contentLen; i++)
        {
            unsigned char byteValue = static_cast<unsigned char>(contentStart[i]);

            if (byteValue > 127 || byteValue < 32)
                printf(".");
            else
                printf("%c", byteValue);
            if ((i + 1) % 16 == 0 || i == contentLen - 1) {
                printf("\n");
            }

        }
    }
}

void ex_extractContent(const char* httpResponse, const struct pcap_pkthdr* header, int headerTotalLen) {

    long int contentLen = header->len - headerTotalLen;
    for (size_t i = 0; i < contentLen; i++)
    {
        unsigned char byteValue = static_cast<unsigned char>(httpResponse[i]);

        printf("%02x ", byteValue);
        if ((i + 1) % 16 == 0 || i == contentLen - 1) {
            printf("\n");
        }
    }
    for (size_t i = 0; i < contentLen; i++)
    {
        unsigned char byteValue = static_cast<unsigned char>(httpResponse[i]);

        if (byteValue > 127 || byteValue < 32)
            printf(".");
        else
            printf("%c", byteValue);
        if ((i + 1) % 16 == 0 || i == contentLen - 1) {
            printf("\n");
        }
    }
}