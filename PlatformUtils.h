#pragma once
#include <string>
#include <chrono>

// ========== Step 1: Define platform macros (corresponding to PLATFORM_* in Makefile) ==========
// If the Makefile does not pass macros, manually complete them (fallback)
#ifndef PLATFORM_WINDOWS
    #ifndef PLATFORM_LINUX
        #ifdef _WIN32
            #define PLATFORM_WINDOWS 1
        #else
            #define PLATFORM_LINUX 1
        #endif
    #endif
#endif

// ========== Step 2: Strictly isolate platform-specific headers ==========
#ifdef PLATFORM_WINDOWS
    // Windows-specific headers (included only under Windows)
    #include <winsock2.h>
    #include <windows.h>
    // Windows Socket type definitions
    #define SOCKET_TYPE SOCKET
    #define CLOSE_SOCKET(s) closesocket(s)
    #define INVALID_SOCKET_VAL INVALID_SOCKET // Renamed to avoid conflicts
#else
    // Linux-specific headers (included only under Linux)
    #include <sys/socket.h>
    #include <unistd.h>
    #include <fcntl.h>   // F_GETFL/F_SETFL/O_NONBLOCK
    #include <unistd.h>  // fcntl
    #include <errno.h>
    // Linux Socket type definitions
    #define SOCKET_TYPE int
    #define CLOSE_SOCKET(s) close(s)
    #define INVALID_SOCKET_VAL (-1)
#endif

// ========== Step 3: Cross-platform utility function declarations (no platform differences) ==========
namespace PlatformUtils {
    // 1. Cross-platform file existence check (alternative to <filesystem>, compatible with all C++ versions)
    bool fileExists(const std::string& path);
    bool deleteFile(const std::string& path, bool verbose = true);

    // 2. Cross-platform set Socket receive timeout
    bool setSocketRecvTimeout(SOCKET_TYPE sock, std::chrono::milliseconds timeout);

    // 3. Cross-platform forced output flush (solves Windows log "swallowing" issues)
    void flushConsole();

    // 4. Windows-specific: Initialize WSA (No-op on Linux)
    bool initSocketEnv();

    // 5. Windows-specific: Cleanup WSA (No-op on Linux)
    void cleanupSocketEnv();
    
    bool setSocketNonBlocking(SOCKET_TYPE sock);

    // 8. Cross-platform restore Socket to blocking mode
    bool setSocketBlocking(SOCKET_TYPE sock);
    
    bool isSocketTimeout();
}