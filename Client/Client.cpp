#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <fstream>
#include <string>
#include <map>
#include <csignal>
#include <filesystem>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")
using namespace std;
using namespace std::filesystem;

#define BUFFER_SIZE 1024
#define PORT 55000

SOCKET clientSock;
bool running = true;
const string outputFolder = "output/";
map<string, long> listFile;
map<string, bool> downloadedFile;

void getListFile(const string& msg) {
    stringstream ss(msg);
    string line;
    while (getline(ss, line)) {
        auto it = line.find(" ");
        if (it != string::npos) {
            string fileName = line.substr(0, it);
            long fileSize = stol(line.substr(it + 1));
            listFile[fileName] = fileSize;
        }
    }
}

void receive_file(SOCKET clientSock, const string& filename) {
    string fullFilePath = outputFolder + filename;
    ofstream outfile(fullFilePath, ios::binary);
    if (!outfile) {
        cerr << "Error opening file for writing: " << filename << endl;
        return;
    }

    char buffer[BUFFER_SIZE];
    long fileSize = listFile[filename];
    cout << "Downloading " << filename << endl;

    long totalBytesReceived = 0;
    while (fileSize > 0)
    {
        int bytesReceived = recv(clientSock, buffer, BUFFER_SIZE, 0);
        if (bytesReceived <= 0) break;
        outfile.write(buffer, bytesReceived);
        if (!outfile) {
            cerr << "Error writing to file: " << filename << endl;
            break;
        }
        fileSize -= bytesReceived;
        totalBytesReceived += bytesReceived;
        int percent = static_cast<int>((static_cast<double>(totalBytesReceived) / (totalBytesReceived + fileSize)) * 100);
        cout << "Progress: " << percent << "%\r";
    }
    cout << endl;
}

string getRequest(int& countLine) {
    fstream infile("input.txt", ios::in);
    if (!infile.is_open()) {
        cout << "Cannot open file input\n";
        return "";
    }
    int count = 0;
    string tmp;
    while (getline(infile, tmp)) {
        count++;
        if (count > countLine) {
            countLine = count;
            infile.close();
            return tmp;
        }
    }
    infile.close();
    return "";
}

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
    switch (fdwCtrlType) {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
        running = false;
        send(clientSock, "QUIT", 4, 0);
        closesocket(clientSock);
        WSACleanup();
        return TRUE;
    default:
        return FALSE;
    }
}

void init()
{
    if (!exists(outputFolder))
    {
        cout << "Directory created: " << outputFolder << endl;
        create_directory(outputFolder);
    }
    fstream file;
    file.open("input.txt", ios::out);
    if (!file.is_open()) cout << "Cannot create input.txt\n";
    return;
}

int main() {
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);
    if (WSAStartup(ver, &wsData) != 0)
    {
        cerr << "Error starting winsock!" << endl;
        return -1;
    }

    clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSock == INVALID_SOCKET)
    {
        cerr << "Error creating socket! " << WSAGetLastError() << endl;
        WSACleanup();
        return -1;
    }

    if (!SetConsoleCtrlHandler(CtrlHandler, TRUE))
    {
        cerr << "Error setting control handler!" << endl;
        closesocket(clientSock);
        WSACleanup();
        return -1;
    }

    char serverAddress[NI_MAXHOST];
    cout << "Enter server address: ";
    cin.getline(serverAddress, NI_MAXHOST);

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(PORT);
    inet_pton(AF_INET, serverAddress, &hint.sin_addr.s_addr);

    if (connect(clientSock, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
        cerr << "Error connecting to server! " << WSAGetLastError() << endl;
        closesocket(clientSock);
        WSACleanup();
        return -1;
    }

    char welcomeMsg[BUFFER_SIZE];
    int bytesReceived = recv(clientSock, welcomeMsg, BUFFER_SIZE, 0);
    if (bytesReceived > 0) {
        welcomeMsg[bytesReceived] = '\0';
        cout << string(welcomeMsg) << endl;
    }

    bytesReceived = recv(clientSock, welcomeMsg, BUFFER_SIZE, 0);
    if (bytesReceived > 0) {
        welcomeMsg[bytesReceived] = '\0';
        getListFile(string(welcomeMsg));
    }

    for (const auto& x : listFile)
    {
        cout << x.first << " " << static_cast<long>(x.second / 1024) << "KB" << endl;
    }
    cout << endl;
    int countLine = 0;

    init();
    while (running) {
        string filename = getRequest(countLine);
        if (!filename.empty())
        {
            if (listFile.find(filename) != listFile.end() && downloadedFile.find(filename) == downloadedFile.end())
            {
                downloadedFile[filename] = true;
                send(clientSock, filename.c_str(), filename.size(), 0);
                receive_file(clientSock, filename);
            }
            else if (listFile.find(filename) != listFile.end() && downloadedFile.find(filename) != downloadedFile.end())
            {
                cout << filename << " already exist\n";
            }
            else cout << filename << " not found in server\n";
        }
    }
    closesocket(clientSock);
    WSACleanup();
    return 0;
}
