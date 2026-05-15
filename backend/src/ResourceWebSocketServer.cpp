#include "ResourceWebSocketServer.hpp"

#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <openssl/sha.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

namespace
{
constexpr int s_connectionBacklog = 8;
constexpr auto s_updateInterval = std::chrono::seconds(1);
constexpr const char* s_webSocketGuid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

void CloseSocketIfValid(int socketDescriptor)
{
    if (socketDescriptor >= 0)
    {
        close(socketDescriptor);
    }
}
}

ResourceWebSocketServer::ResourceWebSocketServer(std::uint16_t port)
    : m_port(port)
{
}

void ResourceWebSocketServer::Run(const PayloadProvider& payloadProvider)
{
    const int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        throw std::runtime_error("Failed to create socket");
    }

    int reuseAddress = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAddress, sizeof(reuseAddress));

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(m_port);

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) < 0)
    {
        CloseSocketIfValid(serverSocket);
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(serverSocket, s_connectionBacklog) < 0)
    {
        CloseSocketIfValid(serverSocket);
        throw std::runtime_error("Failed to listen on socket");
    }

    std::cout << "WebSocket backend is listening on ws://localhost:" << m_port << "/ws\n";

    while (m_isRunning)
    {
        sockaddr_in clientAddress{};
        socklen_t clientAddressSize = sizeof(clientAddress);
        const int clientSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddress), &clientAddressSize);

        if (clientSocket < 0)
        {
            continue;
        }

        std::thread([clientSocket, payloadProvider]()
        {
            char requestBuffer[4096]{};
            const auto receivedBytes = recv(clientSocket, requestBuffer, sizeof(requestBuffer) - 1, 0);

            if (receivedBytes <= 0)
            {
                CloseSocketIfValid(clientSocket);
                return;
            }

            const std::string requestText(requestBuffer, static_cast<std::size_t>(receivedBytes));
            const auto responseText = ResourceWebSocketServer::BuildHandshakeResponse(requestText);

            if (responseText.empty())
            {
                CloseSocketIfValid(clientSocket);
                return;
            }

            send(clientSocket, responseText.data(), responseText.size(), MSG_NOSIGNAL);

            while (true)
            {
                const auto payloadText = payloadProvider();
                const auto frameText = ResourceWebSocketServer::MakeTextFrame(payloadText);
                const auto sentBytes = send(clientSocket, frameText.data(), frameText.size(), MSG_NOSIGNAL);

                if (sentBytes <= 0)
                {
                    break;
                }

                std::this_thread::sleep_for(s_updateInterval);
            }

            CloseSocketIfValid(clientSocket);
        }).detach();
    }

    CloseSocketIfValid(serverSocket);
}

void ResourceWebSocketServer::Stop()
{
    m_isRunning = false;
}

std::string ResourceWebSocketServer::BuildHandshakeResponse(const std::string& requestText)
{
    const auto webSocketKey = ExtractWebSocketKey(requestText);
    if (webSocketKey.empty())
    {
        return {};
    }

    const auto acceptSource = webSocketKey + s_webSocketGuid;
    unsigned char sha1Hash[SHA_DIGEST_LENGTH];

    SHA1(reinterpret_cast<const unsigned char*>(acceptSource.data()), acceptSource.size(), sha1Hash);

    const auto acceptValue = EncodeBase64(sha1Hash, SHA_DIGEST_LENGTH);

    return "HTTP/1.1 101 Switching Protocols\r\n"
           "Upgrade: websocket\r\n"
           "Connection: Upgrade\r\n"
           "Sec-WebSocket-Accept: " + acceptValue + "\r\n\r\n";
}

std::string ResourceWebSocketServer::ExtractWebSocketKey(const std::string& requestText)
{
    const std::string keyHeaderName = "Sec-WebSocket-Key:";
    const auto headerStartPosition = requestText.find(keyHeaderName);

    if (headerStartPosition == std::string::npos)
    {
        return {};
    }

    auto valueStartPosition = headerStartPosition + keyHeaderName.size();
    while (valueStartPosition < requestText.size() && requestText[valueStartPosition] == ' ')
    {
        ++valueStartPosition;
    }

    const auto valueEndPosition = requestText.find("\r\n", valueStartPosition);
    if (valueEndPosition == std::string::npos)
    {
        return {};
    }

    return requestText.substr(valueStartPosition, valueEndPosition - valueStartPosition);
}

std::string ResourceWebSocketServer::EncodeBase64(const unsigned char* data, std::size_t length)
{
    static constexpr char s_base64Table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string encodedText;
    encodedText.reserve(((length + 2) / 3) * 4);

    for (std::size_t byteIndex = 0; byteIndex < length; byteIndex += 3)
    {
        const auto firstByte = data[byteIndex];
        const auto secondByte = byteIndex + 1 < length ? data[byteIndex + 1] : 0;
        const auto thirdByte = byteIndex + 2 < length ? data[byteIndex + 2] : 0;
        const auto combinedBytes = (firstByte << 16) | (secondByte << 8) | thirdByte;

        encodedText.push_back(s_base64Table[(combinedBytes >> 18) & 0x3F]);
        encodedText.push_back(s_base64Table[(combinedBytes >> 12) & 0x3F]);
        encodedText.push_back(byteIndex + 1 < length ? s_base64Table[(combinedBytes >> 6) & 0x3F] : '=');
        encodedText.push_back(byteIndex + 2 < length ? s_base64Table[combinedBytes & 0x3F] : '=');
    }

    return encodedText;
}

std::string ResourceWebSocketServer::MakeTextFrame(const std::string& payloadText)
{
    std::string frameText;
    frameText.push_back(static_cast<char>(0x81));

    const auto payloadSize = payloadText.size();
    if (payloadSize <= 125)
    {
        frameText.push_back(static_cast<char>(payloadSize));
    }
    else if (payloadSize <= 65535)
    {
        frameText.push_back(static_cast<char>(126));
        frameText.push_back(static_cast<char>((payloadSize >> 8) & 0xFF));
        frameText.push_back(static_cast<char>(payloadSize & 0xFF));
    }
    else
    {
        frameText.push_back(static_cast<char>(127));
        for (int byteIndex = 7; byteIndex >= 0; --byteIndex)
        {
            frameText.push_back(static_cast<char>((payloadSize >> (8 * byteIndex)) & 0xFF));
        }
    }

    frameText += payloadText;
    return frameText;
}
