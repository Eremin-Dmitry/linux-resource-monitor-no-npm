#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <string>

class ResourceWebSocketServer
{
public:
    using PayloadProvider = std::function<std::string()>;

    explicit ResourceWebSocketServer(std::uint16_t port);

    void Run(const PayloadProvider& payloadProvider);
    void Stop();

private:
    std::uint16_t m_port;
    std::atomic<bool> m_isRunning{true};

    [[nodiscard]] static std::string BuildHandshakeResponse(const std::string& requestText);
    [[nodiscard]] static std::string ExtractWebSocketKey(const std::string& requestText);
    [[nodiscard]] static std::string EncodeBase64(const unsigned char* data, std::size_t length);
    [[nodiscard]] static std::string MakeTextFrame(const std::string& payloadText);
};
