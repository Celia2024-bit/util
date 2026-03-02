#include "PlatformUtils.h"
#include <fstream>
#include <iostream>
#include <cstdio> 
#include <string> 

namespace PlatformUtils {
    // 1. Cross-platform file existence check
    bool fileExists(const std::string& path) {
        std::ifstream file(path.c_str());
        bool exists = file.is_open() && !file.fail();
        file.close();
        return exists;
    }

    // 2. Cross-platform delete file
    /**
     * @brief Cross-platform deletion of a specified file
     * @param path File path (relative or absolute)
     * @param verbose Whether to print detailed logs (default is true)
     * @return bool Returns true if deleted successfully, false otherwise
     */
    bool deleteFile(const std::string& path, bool verbose) {
        // Step 1: Check if the file exists
        if (!fileExists(path)) {
            if (verbose) {
                std::cerr << "[WARNING] PlatformUtils::deleteFile: File not found - " << path << std::endl;
                flushConsole();
            }
            return false;
        }

        // Step 2: Cross-platform file deletion
        int remove_result = std::remove(path.c_str());
        bool success = (remove_result == 0);

        // Step 3: Log output (controlled by verbose)
        if (verbose) {
            if (success) {
                std::cout << "[INFO] PlatformUtils::deleteFile: File deleted successfully - " << path << std::endl;
            } else {
                #ifdef PLATFORM_WINDOWS
                // Get specific error code on Windows
                DWORD err_code = GetLastError();
                std::cerr << "[ERROR] PlatformUtils::deleteFile: Failed to delete file - " << path 
                          << " (Windows error code: " << err_code << ")" << std::endl;
                #else
                // Get errno on Linux
                std::cerr << "[ERROR] PlatformUtils::deleteFile: Failed to delete file - " << path 
                          << " (errno: " << errno << ")" << std::endl;
                #endif
            }
            flushConsole(); // Force log flush
        }

        return success;
    }

    // 3. Cross-platform set Socket receive timeout
    bool setSocketRecvTimeout(SOCKET_TYPE sock, std::chrono::milliseconds timeout) {
        #ifdef PLATFORM_WINDOWS
        DWORD timeout_ms = static_cast<DWORD>(timeout.count());
        return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, 
                         (const char*)&timeout_ms, sizeof(timeout_ms)) == 0;
        #else
        struct timeval tv;
        tv.tv_sec = timeout.count() / 1000;
        tv.tv_usec = (timeout.count() % 1000) * 1000;
        return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == 0;
        #endif
    }

    // 4. Cross-platform forced output flush
    void flushConsole() {
        std::cout << std::flush;
        std::cerr << std::flush;
        #ifdef PLATFORM_WINDOWS
        fflush(stdout);
        fflush(stderr);
        #endif
    }

    // 5. Socket environment initialization (Windows requires WSAStartup, Linux is no-op)
    bool initSocketEnv() {
        #ifdef PLATFORM_WINDOWS
        WSADATA wsaData;
        return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
        #else
        return true; // Linux does not require initialization
        #endif
    }

    // 6. Socket environment cleanup (Windows requires WSACleanup, Linux is no-op)
    void cleanupSocketEnv() {
        #ifdef PLATFORM_WINDOWS
        WSACleanup();
        #endif
    }
    
    bool setSocketNonBlocking(SOCKET_TYPE sock) {
        #ifdef PLATFORM_WINDOWS
        // Windows: Use ioctlsocket to set non-blocking
        u_long mode = 1; // 1=non-blocking, 0=blocking
        int ret = ioctlsocket(sock, FIONBIO, &mode);
        if (ret != NO_ERROR) {
            std::cerr << "[ERROR] setSocketNonBlocking failed (Windows): " << WSAGetLastError() << std::endl;
            flushConsole();
            return false;
        }
        #else
        // Linux: Use fcntl to set non-blocking
        int flags = fcntl(sock, F_GETFL, 0);
        if (flags == -1) {
            std::cerr << "[ERROR] setSocketNonBlocking failed (Linux): fcntl get flags failed, errno=" << errno << std::endl;
            flushConsole();
            return false;
        }
        if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
            std::cerr << "[ERROR] setSocketNonBlocking failed (Linux): fcntl set flags failed, errno=" << errno << std::endl;
            flushConsole();
            return false;
        }
        #endif
        return true;
    }

    // 8. Cross-platform restore Socket to blocking mode
    bool setSocketBlocking(SOCKET_TYPE sock) {
        #ifdef PLATFORM_WINDOWS
        u_long mode = 0; // 0=blocking
        int ret = ioctlsocket(sock, FIONBIO, &mode);
        if (ret != NO_ERROR) {
            std::cerr << "[ERROR] setSocketBlocking failed (Windows): " << WSAGetLastError() << std::endl;
            flushConsole();
            return false;
        }
        #else
        int flags = fcntl(sock, F_GETFL, 0);
        if (flags == -1) {
            std::cerr << "[ERROR] setSocketBlocking failed (Linux): fcntl get flags failed, errno=" << errno << std::endl;
            flushConsole();
            return false;
        }
        if (fcntl(sock, F_SETFL, flags & ~O_NONBLOCK) == -1) {
            std::cerr << "[ERROR] setSocketBlocking failed (Linux): fcntl set flags failed, errno=" << errno << std::endl;
            flushConsole();
            return false;
        }
        #endif
        return true;
    }
    
    bool isSocketTimeout() {
      #ifdef _WIN32
          return WSAGetLastError() == WSAETIMEDOUT || WSAGetLastError() == WSAEWOULDBLOCK;
      #else
          return errno == EAGAIN || errno == EWOULDBLOCK;
      #endif
    }
}