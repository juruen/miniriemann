#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <ev++.h>
#include <map>
#include <unordered_map>
#include <tcpconnection.h>
#include <pubsub.h>
#include <wsutil.h>

class ws_connection : public tcp_connection {
  public:
    class ws_util* ws_util;
    allevents_f_t all_events_;
    uint32_t state;
    ev::io io;

    ws_connection(int socket_fd, class ws_util* ws_util, allevents_f_t all_events);
    virtual ~ws_connection();
    void callback(int revents);
    void read_cb();
    void write_cb();
    void read_header();
    void read_frame();
    void send_frame(const std::string& payload);
    void write_response_header();
};

#endif
