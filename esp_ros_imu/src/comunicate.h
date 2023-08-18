#ifndef __COMUNICATE_H__
#define __COMUNICATE_H__

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "AsyncUDP.h"

class Remote_node
{
public:
    String name;
    String ip;
    String connection_type;
    IPAddress node_ip;

    uint32_t port;
    Remote_node();
    Remote_node(String _name, String _ip, uint32_t _port);
    void init_node(String _name, String _ip, uint32_t _port);
    uint8_t inited = 0;
    size_t send_data(String data);
    size_t send_data(unsigned char *data, size_t len);
    // handle data callback
    void (*data_cb)(AsyncUDPPacket *);
};

class UDPComunicate : public AsyncUDP
{
public:
    static const int MAX_REMOTE_NODE = 4;
    const char *ssid = WIFI_SSID;
    const char *pass = WIFI_PASS;
    IPAddress remote_IP;
    unsigned int localPort = 8888;
    UDPComunicate();
    String self_node_name = NODE_NAME;
    bool adversting=true,node_finding=true;

    void begin(int port);
    size_t write(uint8_t *data, size_t len);
    size_t write(String data);
    size_t broadcast(uint8_t *data, size_t len);
    size_t  write_node(uint8_t *data,int len,Remote_node &node);
    size_t write_node(String data,Remote_node &node);
    size_t broadcast(String data);
    void print_node_list();
    Remote_node remote_node_list[MAX_REMOTE_NODE];
    Remote_node *get_node(String name);
    bool add_node(String name, String ip, uint32_t port);
    bool add_node(Remote_node node);
    void del_node(String name);
    bool has_node(String name);
    void broadcast_self_node();
};
extern UDPComunicate udp;
#endif