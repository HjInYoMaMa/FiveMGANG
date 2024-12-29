#include <Windows.h>
#include <WinInet.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstdlib>
#include <ctime>

#pragma comment(lib, "wininet.lib")

void SetConsoleColor(int textColor, int bgColor) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, (WORD)((bgColor << 4) | textColor));
}

std::string GenerateRandomTitle(int length) {
    const std::string characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::string title;
    for (int i = 0; i < length; ++i) {
        title += characters[rand() % characters.size()];
    }
    return title;
}

std::vector<std::string> FetchServerResources(const std::string& ip, const std::string& port) {
    std::vector<std::string> resources;
    std::string url = "http://" + ip + ":" + port + "/info.json";

    HINTERNET hInternet = InternetOpenA("HTTP", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet) {
        HINTERNET hConnect = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (hConnect) {
            char buffer[4096];
            DWORD bytesRead;
            std::string response;

            while (InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead != 0) {
                buffer[bytesRead] = '\0';
                response += buffer;
            }

            size_t pos = response.find("\"resources\":");
            if (pos != std::string::npos) {
                pos += 13;
                size_t endPos = response.find("]", pos);
                if (endPos != std::string::npos) {
                    std::string resourcesList = response.substr(pos, endPos - pos);

                    size_t start = 0;
                    size_t commaPos;
                    while ((commaPos = resourcesList.find(",", start)) != std::string::npos) {
                        std::string resource = resourcesList.substr(start, commaPos - start);
                        resource.erase(remove(resource.begin(), resource.end(), '\"'), resource.end());
                        resources.push_back(resource);
                        start = commaPos + 1;
                    }

                    std::string lastResource = resourcesList.substr(start);
                    lastResource.erase(remove(lastResource.begin(), lastResource.end(), '\"'), lastResource.end());
                    resources.push_back(lastResource);
                }
            }
            InternetCloseHandle(hConnect);
        }
        InternetCloseHandle(hInternet);
    }
    return resources;
}

std::vector<std::string> FetchServerPlayers(const std::string& ip, const std::string& port) {
    std::vector<std::string> response;
    std::string url = "http://" + ip + ":" + port + "/players.json";

    HINTERNET hInternet = InternetOpenA("HTTP", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet) {
        HINTERNET hConnect = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (hConnect) {
            char buffer[4096];
            DWORD bytesRead;
            std::string jsonResponse;

            while (InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead != 0) {
                buffer[bytesRead] = '\0';
                jsonResponse += buffer;
            }

            InternetCloseHandle(hConnect);
            response.push_back(jsonResponse);
        }
        else {
            std::cerr << "Failed to connect to the server.\n";
        }
        InternetCloseHandle(hInternet);
    }
    else {
        std::cerr << "Failed to initialize internet connection.\n";
    }

    return response;
}

void SaveResourcesToFile(const std::string& ip, const std::string& port, const std::vector<std::string>& resources) {
    std::string filename = ip + "_" + port + "_resources.txt";
    std::ofstream outFile(filename);

    if (outFile.is_open()) {
        for (const auto& resource : resources) {
            outFile << resource << "\n";
        }
        outFile.close();
        SetConsoleColor(10, 0);
        std::cout << "Resources saved to " << filename << "\n";
    }
    else {
        SetConsoleColor(12, 0);
        std::cout << "Failed to save resources to file.\n";
    }
    SetConsoleColor(15, 0);
}

void SavePlayersToFile(const std::string& ip, const std::string& port, const std::vector<std::string>& players) {
    std::string filename = ip + "_" + port + "_players.txt";
    std::ofstream outFile(filename);

    if (outFile.is_open()) {
        for (const auto& player : players) {
            outFile << player << "\n";
        }
        outFile.close();
        SetConsoleColor(10, 0);
        std::cout << "Players saved to " << filename << "\n";
    }
    else {
        SetConsoleColor(12, 0);
        std::cout << "Failed to save players to file.\n";
    }
    SetConsoleColor(15, 0);
}

int main() {

    srand(static_cast<unsigned int>(time(0)));

    std::string randomTitle = GenerateRandomTitle(10);
    SetConsoleTitle(randomTitle.c_str());

    std::string serverIP;
    std::string serverPort;

    while (true) {

        SetConsoleColor(10, 0);
        std::cout << "Enter Server IP: ";
        SetConsoleColor(15, 0);
        std::getline(std::cin, serverIP);

        SetConsoleColor(10, 0);
        std::cout << "Enter Server Port: ";
        SetConsoleColor(15, 0);
        std::getline(std::cin, serverPort);

        SetConsoleColor(10, 0);
        std::cout << "Do you want to check (1) Resources, (2) Players, or (3) Both? (Enter 1, 2, or 3): ";
        SetConsoleColor(15, 0);
        int choice;
        std::cin >> choice;
        std::cin.ignore();

        if (choice == 1 || choice == 3) {
            SetConsoleColor(14, 0);
            std::cout << "Fetching resources from " << serverIP << ":" << serverPort << "...\n";
            SetConsoleColor(15, 0);

            std::vector<std::string> resources = FetchServerResources(serverIP, serverPort);

            SetConsoleColor(11, 0);
            std::cout << "Current Resources:\n";
            SetConsoleColor(15, 0);
            for (const auto& resource : resources) {
                std::cout << " - " << resource << "\n";
            }

            SaveResourcesToFile(serverIP, serverPort, resources);
        }

        if (choice == 2 || choice == 3) {
            SetConsoleColor(14, 0);
            std::cout << "Fetching players from " << serverIP << ":" << serverPort << "...\n";
            SetConsoleColor(15, 0);

            std::vector<std::string> players = FetchServerPlayers(serverIP, serverPort);

            SetConsoleColor(11, 0);
            std::cout << "Current Players:\n";
            SetConsoleColor(15, 0);
            for (const auto& player : players) {
                std::cout << " - " << player << "\n";
            }

            SavePlayersToFile(serverIP, serverPort, players);
        }

        SetConsoleColor(10, 0);
        std::cout << "Do you want to check another server? (y/n): ";
        SetConsoleColor(15, 0);
        char continueChoice;
        std::cin >> continueChoice;
        std::cin.ignore();

        if (continueChoice != 'y' && continueChoice != 'Y') {
            break;
        }
    }

    SetConsoleColor(10, 0);
    std::cout << "Thanks For Using It.\n";
    SetConsoleColor(15, 0);
    return 0;
}