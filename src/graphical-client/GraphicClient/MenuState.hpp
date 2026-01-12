/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** MenuState
*/

#pragma once

#include <string>
#include <functional>

enum class MenuPage {
    CONNECTION,
    LOBBY,
    SETTINGS
};

class MenuState {
public:
    MenuState() = default;
    ~MenuState() = default;

    MenuPage getCurrentPage() const { return _currentPage; }
    void setCurrentPage(MenuPage page) { _currentPage = page; }

    std::string getServerIp() const { return _serverIp; }
    void setServerIp(const std::string& ip) { _serverIp = ip; }

    int getServerPort() const { return _serverPort; }
    void setServerPort(int port) { _serverPort = port; }

    bool isConnected() const { return _isConnected; }
    void setConnected(bool connected) { _isConnected = connected; }

    std::string getConnectionError() const { return _connectionError; }
    void setConnectionError(const std::string& error) { _connectionError = error; }

private:
    MenuPage _currentPage = MenuPage::CONNECTION;
    std::string _serverIp = "127.0.0.1";
    int _serverPort = 4243;
    bool _isConnected = false;
    std::string _connectionError;
};
