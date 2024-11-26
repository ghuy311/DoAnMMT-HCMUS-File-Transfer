#include <iostream>
#include <fstream>
#include <ws2tcpip.h>
#include <WinSock2.h>
#include <string>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

#define BUFFER_SIZE 1024
#define PORT 55000

string listFile(const string& filename) {
    ifstream f(filename);
    if (!f.is_open()) {
        cout << filename << " Not found\n";
        return "";
    }
    string buf, line;
    while (getline(f, line)) {
        ifstream file(line, ios::binary);
        if (!file.is_open()) {
            cout << "File " << line << " Not found\n";
            continue;
        }
        file.seekg(0, ios::end);
        streampos fileSize = file.tellg();
        line += " " + to_string(fileSize);
        buf += line + '\n';
        file.close();
    }
    f.close();
    return buf;
}

void send_file(SOCKET clientSock, const string& filename) {
    ifstream file(filename, ios::binary);
    char bufferFile[BUFFER_SIZE];
    if (file.is_open()) {
        while (file.read(bufferFile, BUFFER_SIZE) || file.gcount() > 0) {
            send(clientSock, bufferFile, file.gcount(), 0);
        }
        cout << "Sent file " << filename << " successfully\n";
        file.close();
    }
    return;
}

int main() {
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);

    if (WSAStartup(ver, &wsData) != 0) {
        cerr << "Error starting winsock!" << endl;
        return -1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Error creating listener socket! " << WSAGetLastError() << endl;
        WSACleanup();
        return -1;
    }

    string ip_server;
    cout << "Input IP address to bind: ";
    cin >> ip_server;

    sockaddr_in listenerHint;
    listenerHint.sin_family = AF_INET;
    listenerHint.sin_port = htons(PORT);
    inet_pton(AF_INET, ip_server.c_str(), &listenerHint.sin_addr.s_addr);

    if (bind(serverSocket, (sockaddr*)&listenerHint, sizeof(listenerHint)) == SOCKET_ERROR) {
        cerr << "Error binding server socket! " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    listen(serverSocket, SOMAXCONN);
    cout << "Server is ready...\n";
    cout << "Server is running on port " << PORT << endl;

    sockaddr_in clientHint;
    int clientSize = sizeof(clientHint);
    int numClients = 0;

    while (true)
    {
        cout << "----------------------------------\n";
        cout << "Waiting for connection...\n";
        SOCKET clientSock = accept(serverSocket, (sockaddr*)&clientHint, &clientSize);

        if (clientSock == INVALID_SOCKET) {
            cerr << "Error accepting socket! " << WSAGetLastError() << endl;
            closesocket(clientSock);
            WSACleanup();
            return -1;
        }

        numClients++;
        cout << "Server successfully connected to client " << numClients << endl;
        cout << "Waiting for request to download file\n";

        const char* welcomeMsg = "Welcome to file server. Please request file to download\n";
        string fileList = listFile("input.txt");
        const char* fileListMsg = fileList.c_str();

        send(clientSock, welcomeMsg, strlen(welcomeMsg), 0);
        send(clientSock, fileListMsg, strlen(fileListMsg), 0);

        char fileRequested[FILENAME_MAX];
        bool clientClose = false;

        do {
            memset(fileRequested, 0, FILENAME_MAX);
            int byRecv = recv(clientSock, fileRequested, FILENAME_MAX, 0);

            if (byRecv == 0 || byRecv == -1) {
                clientClose = true;
            }
            else {
                fileRequested[byRecv] = '\0';
                if (string(fileRequested) == "QUIT") {
                    cout << "Client " << numClients << " ended session.\n";
                    clientClose = true;
                }
                else {
                    send_file(clientSock, fileRequested);
                }
            }
        } while (!clientClose);

        closesocket(clientSock);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
