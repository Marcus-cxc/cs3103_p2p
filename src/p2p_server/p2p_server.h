#ifndef CS3103_P2P_P2P_SERVER_H
#define CS3103_P2P_P2P_SERVER_H

#include <WS2tcpip.h>
#include "../core/core_functions.h"
#include "../core/P2P_proto_packet.h"
#include "../core/storage.h"

#define PATH_TO_STORAGE_DIRECTORY "..\\src\\p2p_server\\files_for_seeding"

using namespace std;

const char FILE_NOT_FOUND_ERROR[] = "FILE NOT FOUND";
const int MAX_BUFFER_LEN = 2048;

class P2P_Server {
private:
    bool online;
    SOCKET listen_sock;
    IN_ADDR p2p_server_ip;
    char *recv_buffer;
    char *send_buffer;
    char *chunk_buffer;
    Storage storage;
public:
    /**
     * Constructor for P2P server
     */
    P2P_Server() : listen_sock(INVALID_SOCKET), online(false), storage(PATH_TO_STORAGE_DIRECTORY)   {

        WSADATA wsock;
        int status = WSAStartup(MAKEWORD(2,2),&wsock);

        if ( status != 0)
            cout << "[ERROR]: " << status << " Unable to start Winsock.\n";
        else
            online = true;

        get_private_IP(p2p_server_ip);

        recv_buffer = (char *)malloc(MAX_BUFFER_LEN);
        send_buffer = (char *)malloc(FIXED_CHUNK_CONTENT_SIZE);
        chunk_buffer = (char *)malloc(FIXED_CHUNK_SIZE);
        //CreateDirectory(PATH_TO_STORAGE_DIRECTORY, nullptr);
    };

    /**
     * destructor for P2P server
     */
    ~P2P_Server() {
        stop();
        if (online)
            WSACleanup();
        if (recv_buffer != nullptr)
            recv_buffer = nullptr;
        if (send_buffer != nullptr)
            send_buffer = nullptr;
        if (chunk_buffer != nullptr)
            chunk_buffer = nullptr;
    };

    bool start(const char *port);
    void stop();
    bool listen();

    /**
     * Process datagram in recv_buffer and reply peer.
     * @param client_addr
     * @param sin_size
     * @return true as long as a reply is sent (even if chunk retrieval is unsuccessful). false otherwise.
     */
    bool process_request(sockaddr_in client_addr, int sin_size);
};

#endif //CS3103_P2P_P2P_SERVER_H
