#include "PlatformUtils.h"
#include <fstream>
#include <iostream>

namespace PlatformUtils {
    // 1. 跨平台文件存在性检查
    bool fileExists(const std::string& path) {
        std::ifstream file(path.c_str());
        bool exists = file.is_open() && !file.fail();
        file.close();
        return exists;
    }

    // 2. 跨平台设置Socket接收超时
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

    // 3. 跨平台强制刷新输出
    void flushConsole() {
        std::cout << std::flush;
        std::cerr << std::flush;
        #ifdef PLATFORM_WINDOWS
        fflush(stdout);
        fflush(stderr);
        #endif
    }

    // 4. Socket环境初始化（Windows需WSAStartup，Linux空实现）
    bool initSocketEnv() {
        #ifdef PLATFORM_WINDOWS
        WSADATA wsaData;
        return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
        #else
        return true; // Linux无需初始化
        #endif
    }

    // 5. Socket环境清理（Windows需WSACleanup，Linux空实现）
    void cleanupSocketEnv() {
        #ifdef PLATFORM_WINDOWS
        WSACleanup();
        #endif
    }
}