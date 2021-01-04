#pragma once

#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_INTERNAL_

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <set>

using namespace std;

typedef websocketpp::server<websocketpp::config::asio> server;

class ControlServer {
public:
    ControlServer() {
        m_endpoint.set_error_channels(websocketpp::log::elevel::none);
        m_endpoint.set_access_channels(websocketpp::log::alevel::none);

        m_endpoint.init_asio();

        m_endpoint.set_open_handler(std::bind(
            &ControlServer::open_handler, this,
            std::placeholders::_1
        ));
        m_endpoint.set_close_handler(std::bind(
            &ControlServer::close_haldner, this,
            std::placeholders::_1
        ));
    }

    void open_handler(websocketpp::connection_hdl hdl) {
        con_list.insert(hdl);
    }
    void close_haldner(websocketpp::connection_hdl hdl) {
        con_list.erase(hdl);
    }
    void broadcast(string msg) {
        for (auto hdl : con_list)
            m_endpoint.send(hdl, msg, websocketpp::frame::opcode::text);
    }
    void sendPrev() {
        broadcast("prev");
    }
    void sendPause() {
        broadcast("pause");
    }
    void sendNext() {
        broadcast("next");
    }
    void sendVolumeDown() {
        broadcast("volume_down");
    }
    void sendVolumeUp() {
        broadcast("volume_up");
    }
    void sendSpeedDown() {
        broadcast("speed_down");
    }
    void sendSpeedUp() {
        broadcast("speed_up");
    }
    void sendBackward() {
        broadcast("backward");
    }
    void sendForward() {
        broadcast("forward");
    }
    void sendRepeat() {
        broadcast("repeat");
    }
    void sendShuffle() {
        broadcast("shuffle");
    }
    void sendStartMix() {
        broadcast("start_mix");
    }
    void sendStart(int index) {
        char buff[20];
        sprintf_s(buff, "start %d", index);

        broadcast(buff);
    }

    void run() {
        m_endpoint.listen(9002);

        m_endpoint.start_accept();

        m_endpoint.run();
    }
    void stop() {
        con_list.clear();
        m_endpoint.stop();
    }
private:
    server m_endpoint;

    set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> con_list;
};