/**
 * @file ResourceWebSocketServer.hpp
 * @brief WebSocket server for serving metrics to clients
 *
 * Implements a minimal WebSocket server that accepts client connections
 * and periodically sends JSON-formatted system metrics payloads.
 */

#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <string>

/**
 * @class ResourceWebSocketServer
 * @brief WebSocket server for metrics distribution
 *
 * Listens for WebSocket connections and calls a payload provider function
 * repeatedly, sending the results to all connected clients. Operates on a
 * dedicated thread and handles the complete WebSocket protocol handshake
 * and frame encoding.
 *
 * @example
 * @code
 * ResourceWebSocketServer server(9002);
 * server.Run([&] {
 *     return "{\"cpu\":45.3}";  // JSON payload
 * });
 * @endcode
 */
class ResourceWebSocketServer
{
public:
    /// Callback function type for providing JSON payloads
    using PayloadProvider = std::function<std::string()>;

    /**
     * @brief Constructor
     *
     * @param port TCP port for WebSocket server to listen on
     */
    explicit ResourceWebSocketServer(std::uint16_t port);

    /**
     * @brief Run the WebSocket server (blocking call)
     *
     * Starts listening for client connections and repeatedly calls the
     * payload provider to get metrics data, sending it to all connected clients.
     * Blocks until Stop() is called.
     *
     * @param payloadProvider Callback function returning JSON metrics
     * @throws std::runtime_error if server cannot bind to port or other socket errors occur
     *
     * @example
     * @code
     * server.Run([metrics](){ return metrics->ToJson(); });
     * @endcode
     */
    void Run(const PayloadProvider& payloadProvider);

    /**
     * @brief Stop the WebSocket server
     *
     * Signals the server to shut down and close all client connections.
     * Called from signal handler or separate thread.
     */
    void Stop();

private:
    /// Port number to listen on
    std::uint16_t m_port;
    /// Flag indicating if server should continue running
    std::atomic<bool> m_isRunning{true};

    /**
     * @brief Build WebSocket handshake response
     *
     * Constructs the HTTP response required by the WebSocket protocol
     * to complete the connection upgrade. Includes required headers and
     * the derived Sec-WebSocket-Accept value.
     *
     * @param requestText The incoming HTTP handshake request
     * @return std::string Complete HTTP response including headers
     */
    [[nodiscard]] static std::string BuildHandshakeResponse(const std::string& requestText);

    /**
     * @brief Extract the Sec-WebSocket-Key from handshake request
     *
     * Parses the client's HTTP request to find the WebSocket key,
     * which is required to compute the handshake response.
     *
     * @param requestText The incoming HTTP handshake request
     * @return std::string The Sec-WebSocket-Key value
     * @throws std::runtime_error if key is not found
     */
    [[nodiscard]] static std::string ExtractWebSocketKey(const std::string& requestText);

    /**
     * @brief Encode binary data as Base64
     *
     * Used to encode the SHA1 hash in the handshake response.
     *
     * @param data Raw binary data
     * @param length Size of data in bytes
     * @return std::string Base64-encoded representation
     */
    [[nodiscard]] static std::string EncodeBase64(const unsigned char* data, std::size_t length);

    /**
     * @brief Create WebSocket text frame
     *
     * Wraps JSON payload in a WebSocket frame with proper framing bits,
     * opcode, and payload length encoding per RFC 6455.
     *
     * @param payloadText JSON string to send
     * @return std::string Binary WebSocket frame data
     */
    [[nodiscard]] static std::string MakeTextFrame(const std::string& payloadText);
};
