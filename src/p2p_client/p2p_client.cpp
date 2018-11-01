//
// Created by Jarrett on 22/10/2018.
//

#include "p2p_client.h"
#include <iostream>
#include <fstream>
#include "../core/storage.h"
#include "../core/p2p_request.h"
#include "../core/tracker_entries.h"
#include "p2p_client_helper.h"
#include <map>

#define MAX_BUFFER_SIZE 65536
#undef max

using namespace std;

// Winsock variables
SOCKET connect_socket;
WSAData wsa_data;
struct addrinfo *result = nullptr,
        *ptr = nullptr,
        hints;
int iresult;

char recvbuf[MAX_BUFFER_SIZE];
map <int, tracker_peer_list_entry> peer_list;

void p2p_client::display_menu() {

    printf("******P2P CLIENT******\n"
           "Enter options (1 to 5):\n"
           "\t1. Download a file\n"
           "\t2. Upload a file\n"
           "\t3. Query for a list of files available in tracker\n"
           "\t4. Query for a file available in tracker\n"
           "\t5. Quit\n"
           "Enter option: ");

}

void p2p_client::connection(const char *ip_addr, const char *port_num, bool is_tracker) {

    iresult = WSAStartup(MAKEWORD(2,2), &wsa_data);
    if (iresult != 0) {
        printf("WSAStartup failed with error: %d\n", iresult);
        exit(EXIT_FAILURE);
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_INET;
    if (is_tracker) {
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
    } else {
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
    }


    iresult = getaddrinfo(ip_addr, port_num, &hints, &result);
    if ( iresult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iresult);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    for(ptr=result; ptr != nullptr ; ptr=ptr->ai_next) {

        connect_socket = socket(ptr->ai_family, ptr->ai_socktype,
                               ptr->ai_protocol);
        if (connect_socket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            exit(EXIT_FAILURE);
        }
        break;
    }

    freeaddrinfo(result);

    if (connect_socket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

}

void p2p_client::download_file(char *tracker_port, string filename) {

    this->connection(this->tracker_ip, tracker_port, true);

    string str = "REQUEST 1 " + filename;
    const char *buf = str.c_str();

    iresult = sendto(connect_socket, buf, strlen(buf), 0, ptr->ai_addr, ptr->ai_addrlen);
    iresult = recvfrom(connect_socket, recvbuf, MAX_BUFFER_SIZE, 0, nullptr, nullptr);

    string recv_str(recvbuf);
    int peer_list_size = parse_peer_list(peer_list, recvbuf);

    closesocket(connect_socket);
    memset(recvbuf, '\0', MAX_BUFFER_SIZE); // clears recvbuf
    WSACleanup();

    // The following part deals with connection to p2p server
    // TODO: Connect to p2p_server.. almost done..
//    string p2p_server_ip;
//    string p2p_server_chunk_num;
//    string p2p_server_port_num;
//    vector<bool> check_downloaded_chunks(static_cast<unsigned int>(peer_list_size + 1), false);
//    check_downloaded_chunks[0] = NULL; // since chunk starts from #1; we will not use index 0.
//    int downloaded_chunks = 0;
//
//    while (downloaded_chunks != peer_list_size + 1) {
//
//        choose_random_server(peer_list, p2p_server_ip,
//                p2p_server_chunk_num, p2p_server_port_num);
//
//        // The chunk_num has already been downloaded; choose another chunk
//        if (check_downloaded_chunks[p2p_server_chunk_num]) {
//            continue;
//        } else {
//            // Testing by printing
//            cout << "p2p_server_ip is: " + p2p_server_ip << endl;
//
//            this->connection(p2p_server_ip.c_str(), p2p_server_port_num.c_str(), false);
//
//            str = "DOWNLOAD " + filename + " " + p2p_server_port_num;
//            const char *buf_tcp = str.c_str();
//
//            iresult = send(connect_socket, buf_tcp, strlen(buf_tcp), 0);
//            // TODO: what if connection failed?
//            iresult = recv(connect_socket, recvbuf, strlen(recvbuf), 0);
//
//            cout << recvbuf << endl;
//
//            closesocket(connect_socket);
//            memset(recvbuf, '\0', MAX_BUFFER_SIZE); // clears recvbuf
//            peer_list.clear(); // clears peer_list
//            WSACleanup();
//
//            check_downloaded_chunks[p2p_server_chunk_num] = true;
//            downloaded_chunks++;
//        }
//    }

}

void p2p_client::query_list_of_files(char *tracker_port) {

    this->connection(this->tracker_ip, tracker_port, true);

    string str = "REQUEST 6";
    const char *buf = str.c_str();

    iresult = sendto(connect_socket, buf, strlen(buf), 0, ptr->ai_addr, ptr->ai_addrlen);

    iresult = recvfrom(connect_socket, recvbuf, MAX_BUFFER_SIZE, 0, nullptr, nullptr);

    string recv_str(recvbuf);

    // When tracker only returns "RESPONSE", means there are no files at tracker
    if (recv_str == "RESPONSE ") {
        cout << "No files found" << endl;
    } else {
        string space_delimiter = " ";
        recv_str.erase(0, recv_str.find(space_delimiter) + space_delimiter.length());
        cout << recv_str;
        // TODO: Maybe I have to parse the output to make it look nicer.
    }

    closesocket(connect_socket);
    memset(recvbuf, '\0', MAX_BUFFER_SIZE); // clears recvbuf
    WSACleanup();
}

void p2p_client::query_file(char *tracker_port, string filename) {

//    assert(filename.length() < 256);

    this->connection(this->tracker_ip, tracker_port, true);

    string str = "REQUEST 7 " + filename;
    const char *buf = str.c_str();

    iresult = sendto(connect_socket, buf, strlen(buf), 0, ptr->ai_addr, ptr->ai_addrlen);

    iresult = recvfrom(connect_socket, recvbuf, MAX_BUFFER_SIZE, 0, nullptr, nullptr);
    string recv_str(recvbuf);
    string space_delimiter = " ";
    recv_str.erase(0, recv_str.find(space_delimiter) + space_delimiter.length());
    cout << recv_str << endl;

    closesocket(connect_socket);
    memset(recvbuf, '\0', MAX_BUFFER_SIZE);
    WSACleanup();
}

void p2p_client::upload_file(char *tracker_port, string filename) {

    this->connection(this->tracker_ip, tracker_port, true);

    Storage storage("..\\download");
    int chunk_no_buffer[MAX_BUFFER_SIZE];
    int num_of_chunks = storage.getArrOfChunkNumbers(chunk_no_buffer, MAX_BUFFER_SIZE, filename);

    if (num_of_chunks == -1) {
        cout << "getArrOfChunkNumbers is unsuccessful!";
        exit(EXIT_FAILURE);
    }

    string str = "REQUEST 4 ";

    // Eg. REQUEST 4 test.txt 1 test.txt 2 test.txt 3
    for (auto chunk_no = 1; chunk_no <= num_of_chunks; chunk_no++) {
        str += filename + " " + to_string(chunk_no) + "|"; // TODO: Not exactly what I want..
    }

    const char *buf = str.c_str();
    sendto(connect_socket, buf, strlen(buf), 0, ptr->ai_addr, ptr->ai_addrlen);

    closesocket(connect_socket);
    memset(recvbuf, '\0', MAX_BUFFER_SIZE);
    WSACleanup();
}

void p2p_client::quit() {
    // TODO: The client has to contact the tracker... have to adjust index
    printf("Goodbye!\n");
}

int execute_user_option(p2p_client client) {

       int user_option;
       string filename;
       cin >> user_option;

       if (cin.fail()) {
           printf("Input is not an integer.\n");
           // The following is required to avoid cin fail infinite loop
           cin.clear();
           cin.ignore(numeric_limits<streamsize>::max(), '\n');
       }

       if (user_option < 1 || user_option > 7) {
           printf("No such option.\n");
           return -1;
       }

       switch (user_option) {
           case 1:
               cout << "Enter filename to download: ";
               cin >> filename;
               client.download_file(DEFAULT_TRACKER_PORT, filename);
               return 1;
           case 2:
               cout << "Enter filename to upload: ";
               cin >> filename;
               client.upload_file(DEFAULT_TRACKER_PORT, filename);
               return 2;
           case 3:
               client.query_list_of_files(DEFAULT_TRACKER_PORT);
               return 3;
           case 4:
               cout << "Enter filename: ";
               cin >> filename;
               client.query_file(DEFAULT_TRACKER_PORT, filename);
               return 4;
           default:
               client.quit();
               return 5;
       }

}
