#include <iostream>
#include<string>
#include "pcap.h"
#include<iomanip>
#include<cstring>
#pragma comment(lib,"wpcap")
#pragma warning(disable:4996)
using namespace std;
/* packet handler 函数原型 */
void packet_handler(u_char* param, const struct pcap_pkthdr* header, const u_char* pkt_data);
//功能函数原型
void extractURL(const char* httpPacket); //提取拼接URL
void extractContent(const char* httpResponse, const struct pcap_pkthdr* header, const u_char* pkt_data);//提取响应包内容
void ex_extractContent(const char* httpResponse, const struct pcap_pkthdr* header, int headerTotalLen);
/* 4字节的IP地址 */
typedef struct ip_address {
    u_char byte1;
    u_char byte2;
    u_char byte3;
    u_char byte4;
}ip_address;

/* IPv4 首部 */
typedef struct ip_header {
    u_char  ver_ihl;        // 版本 (4 bits) + 首部长度 (4 bits)
    u_char  tos;            // 服务类型(Type of service) 
    u_short tlen;           // 总长(Total length) 
    u_short identification; // 标识(Identification)
    u_short flags_fo;       // 标志位(Flags) (3 bits) + 段偏移量(Fragment offset) (13 bits)
    u_char  ttl;            // 存活时间(Time to live)
    u_char  proto;          // 协议(Protocol)
    u_short crc;            // 首部校验和(Header checksum)
    ip_address  saddr;      // 源地址(Source address)
    ip_address  daddr;      // 目的地址(Destination address)
    u_int   op_pad;         // 选项与填充(Option + Padding)
}ip_header;

/*tcp 首部*/
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

/*http首部*/
typedef struct http_header {
    u_int protocol; //仅用于判断是GET请求还是应答报文
}http_header;

int  main()
{
    pcap_if_t* alldevs;
    pcap_if_t* d;
    int i = 0;
    char errbuf[PCAP_ERRBUF_SIZE];
    struct bpf_program fcode;

    //过滤器配置
    char packet_filter[] = "tcp and host 10.160.2.157 and (src or dst port 80)";


    //获取设备列表
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

    //找到要捕获的适配器指针
    int j;
    for (d = alldevs, j = 1; j < inum; d = d->next, j++) {}

    //打开设备
    cout << d->addresses->netmask << endl;
    if ((adhandle = pcap_open(d->name, 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf)) == NULL) {
        cout << "\nUnable to open the adapter. " << d->name << " is not supported by WinPcap\n";
        pcap_freealldevs(alldevs);
        return -1;
    }

    u_int netmask;
    if (d->addresses != NULL)
        /* 获得接口第一个地址的掩码 */
        netmask = ((struct sockaddr_in*)(d->addresses->netmask))->sin_addr.S_un.S_addr;
    else
        /* 如果接口没有地址，那么我们假设一个B类的掩码 */
        netmask = 0xffff0000;
    cout << hex << "netmask:" << netmask << endl;

    //编译过滤器
    if (pcap_compile(adhandle, &fcode, packet_filter, 0, netmask) < 0)
    {
        fprintf(stderr, "\nUnable to compile the packet filter. Check the syntax.\n");
        /* 释放设备列表 */
        pcap_freealldevs(alldevs);
        return -1;
    }

    //设置过滤器
    if (pcap_setfilter(adhandle, &fcode) < 0)
    {
        fprintf(stderr, "\nError setting the filter.\n");
        /* 释放设备列表 */
        pcap_freealldevs(alldevs);
        return -1;
    }

    cout << "\nListening on " << d->description << "\n";
    pcap_freealldevs(alldevs);
    pcap_loop(adhandle, 0, packet_handler, NULL);

    return 0;
}

/*由于没有找到现成的过滤规则单独捕获http，在tcp包中筛选*/

void packet_handler(u_char* param, const struct pcap_pkthdr* header, const u_char* pkt_data) {
    struct tm* ltime;
    char timestr[16];
    time_t local_tv_sec;
    ip_header* ih;
    tcp_header* th;
    u_int ip_len;
    u_int tcp_len;

    /* 获得IP数据包头部的位置 */
    ih = (ip_header*)(pkt_data + 14); //以太网头部长度

    /*获得tcp数据包头部位置*/
    ip_len = (ih->ver_ihl & 0x0f) * 4;
    th = (tcp_header*)((u_char*)ih + ip_len);

    /* 获得http开始位置*/
    th->hlen_flags = ntohs(th->hlen_flags);
    tcp_len = ((th->hlen_flags & 0xf000) >> 12) * 4;
    http_header* http = (http_header*)((u_char*)th + tcp_len);

    //计算得到总包头长度
    long int headerTotalLen = static_cast<int>((const u_char*)http - pkt_data + 1);

    if (headerTotalLen < header->len) {
        /*判断数据报类型，是GET请求还是应答报文*/
        char method[5];
        method[4] = '\0';

        //获取http开始四个字母
        sprintf(method, "%c%c%c%c", http->protocol & 0xFF, (http->protocol >> 8) & 0xFF, (http->protocol >> 16) & 0xFF, (http->protocol >> 24) & 0xFF);

        //如果是GET请求报文
        if (strncmp(method, "GET", 3) == 0) {
            printf("\n%s", method);
            /* 将时间戳转换成可识别的格式 */
            local_tv_sec = header->ts.tv_sec;
            ltime = localtime(&local_tv_sec);
            strftime(timestr, sizeof timestr, "%H:%M:%S", ltime);

            /* 打印数据包的时间戳和长度 */
            printf("%s,%.6d len:%d\n", timestr, header->ts.tv_usec, header->len);


            //打印http报文中的url，请求行和报头两部分url要拼接
            const char* httpGet = (const char*)http;
            extractURL(httpGet);

            printf("------------------GET请求报文结束----------------------------\n");
        }

        //如果是响应报文，开头是HTTP
        else if ((strncmp(method, "HTTP", 4)) == 0) {
            printf("\n%s Response ", method);
            /* 将时间戳转换成可识别的格式 */
            local_tv_sec = header->ts.tv_sec;
            ltime = localtime(&local_tv_sec);
            strftime(timestr, sizeof timestr, "%H:%M:%S", ltime);

            /* 打印数据包的时间戳和长度 */
            printf("%s,%.6d len:%d\n", timestr, header->ts.tv_usec, header->len);
            const char* httpRes = (const char*)http;

            extractContent(httpRes, header, pkt_data);

            printf("----------------响应报文结束------------------------------\n");
        }

        else if ((strncmp(method, "POST", 4)) == 0);
        else if ((http->protocol & 0xFFFF) == 0x0000);//检查HTTP协议的版本号是否为1.0

        else {
            printf("\nContinuation ");
            /* 将时间戳转换成可识别的格式 */
            local_tv_sec = header->ts.tv_sec;
            ltime = localtime(&local_tv_sec);
            strftime(timestr, sizeof timestr, "%H:%M:%S", ltime);

            /* 打印数据包的时间戳和长度 */
            printf("%s,%.6d len:%d\n", timestr, header->ts.tv_usec, header->len);
            const char* httpRes = (const char*)http;

            ex_extractContent(httpRes, header, headerTotalLen);

            printf("----------------响应包后续结束------------------------------\n");
        }
    }
}

void extractURL(const char* httpPacket) {

    // 提取请求行中的URL
    const char* requestLineUrlEnd = strstr(httpPacket, "HTTP/1.1");
    std::string requestLine(httpPacket, requestLineUrlEnd);//将httpPacket和requestLineUrlEnd之间的字符赋给requestLine

    // 提取Host
    const char* hostHeader = strstr(httpPacket, "Host: ");
    if (hostHeader) {
        const char* hostValueStart = hostHeader + 6;
        const char* hostValueEnd = strchr(hostValueStart, '\r');
        if (hostValueEnd) {
            std::string hostValue(hostValueStart, hostValueEnd);

            // 建立拼接URL并输出
            std::string fullURL = "http://" + hostValue + requestLine.substr(4);
            std::cout << "Full URL: " << fullURL << std::endl;
        }
    }

}

void extractContent(const char* httpResponse, const struct pcap_pkthdr* header, const u_char* pkt_data) {
    // 找到HTTP响应头的结尾
    const char* headersEnd = strstr(httpResponse, "\r\n\r\n");

    if (headersEnd) {
        // 定位到HTTP响应正文位置
        const char* contentStart = headersEnd + 4;
        long int headerTotalLen = static_cast<int>((const u_char*)contentStart - pkt_data);
        long int contentLen = header->len - headerTotalLen;//获得HTTP响应正文长度

        //打印HTTP响应正文十六进制的值
        for (size_t i = 0; i < contentLen; i++)
        {
            unsigned char byteValue = static_cast<unsigned char>(contentStart[i]);

            printf("%02x ", byteValue);
            if ((i + 1) % 16 == 0 || i == contentLen - 1) {
                printf("\n");
            }
        }

        //打印HTTP响应正文的ASCII字符
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