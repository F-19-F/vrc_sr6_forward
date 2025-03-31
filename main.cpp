#include <iostream>
#include <thread>
#include <set>
#define _WEBSOCKETPP_CPP11_INTERNAL_
#define ASIO_STANDALONE


#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <nlohmann/json.hpp>
#include <asio.hpp>
using json = nlohmann::json;
using websocketpp::connection_hdl;
typedef websocketpp::server<websocketpp::config::asio> server;

#pragma pack(push, 1)
struct Packet {
    uint32_t src;
    float x;
    float y;
    float z;
    float pitch;
    float yaw;
    float roll;
};
#pragma pack(pop)

server ws_server;
std::set<connection_hdl, std::owner_less<connection_hdl>> connections;

void udp_listener(unsigned short port) {
    asio::io_context io_context;
    asio::ip::udp::socket socket(io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), port));

    while (true) {
        Packet pkt;
        asio::ip::udp::endpoint sender_endpoint;
        socket.receive_from(asio::buffer(&pkt, sizeof(pkt)), sender_endpoint);

        json data = {
            {"x", pkt.x},
            {"y", pkt.y},
            {"z", pkt.z},
            {"pitch", pkt.pitch},
            {"yaw", pkt.yaw},
            {"roll", pkt.roll}
        };

        std::string message = data.dump();

        // 发送给所有连接的WebSocket客户端
        for (auto& hdl : connections) {
            ws_server.send(hdl, message, websocketpp::frame::opcode::text);
        }
    }
}

int main() {
    asio::io_context io_context;

    ws_server.init_asio(&io_context);

    ws_server.set_open_handler([](connection_hdl hdl) {
        connections.insert(hdl);
        });

    ws_server.set_close_handler([](connection_hdl hdl) {
        connections.erase(hdl);
        });
    
    ws_server.clear_access_channels(websocketpp::log::alevel::all);
    ws_server.clear_error_channels(websocketpp::log::elevel::all);
    ws_server.set_access_channels(websocketpp::log::alevel::connect);

    ws_server.listen(8000);
    ws_server.start_accept();

    std::thread udp_thread(udp_listener, 12345);

    io_context.run();

    udp_thread.join();

    return 0;
}

