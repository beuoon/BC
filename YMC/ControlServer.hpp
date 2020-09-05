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
        // Set logging settings
        m_endpoint.set_error_channels(websocketpp::log::elevel::none);
        m_endpoint.set_access_channels(websocketpp::log::alevel::none);

        // Initialize Asio
        m_endpoint.init_asio();

        // Set the default message handler to the echo handler
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

    void control() {
        while (isControl) {
            if (GetAsyncKeyState(VK_OEM_5) && GetAsyncKeyState(0x50)) { // '\' and 'p'
                for (auto hdl : con_list) {
                    string msg = "prev";
                    m_endpoint.send(hdl, msg, websocketpp::frame::opcode::text);
                }
            }
            if (GetAsyncKeyState(VK_OEM_5) && GetAsyncKeyState(VK_OEM_4)) { // '\' and '['
                for (auto hdl : con_list) {
                    string msg = "pause";
                    m_endpoint.send(hdl, msg, websocketpp::frame::opcode::text);
                }
            }
            if (GetAsyncKeyState(VK_OEM_5) && GetAsyncKeyState(VK_OEM_6)) { // '\' and ']'
                for (auto hdl : con_list) {
                    string msg = "next";
                    m_endpoint.send(hdl, msg, websocketpp::frame::opcode::text);
                }
            }

            Sleep(100);
        }
    }

    void run() {
        // Listen on port 9002
        m_endpoint.listen(9002);

        // Queues a connection accept operation
        m_endpoint.start_accept();

        controlThread = new thread(&ControlServer::control, this);

        // Start the Asio io_service run loop
        m_endpoint.run();
    }
    void stop() {
        isControl = false;
        controlThread->join();
        m_endpoint.stop();
    }
private:
    server m_endpoint;
    set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> con_list;
    bool isControl = true;
    thread *controlThread;
};