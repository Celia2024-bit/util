#pragma once
#include <string>
#include <chrono>

// ========== 第一步：先定义平台宏（和Makefile的PLATFORM_*对应） ==========
// 如果Makefile没传宏，手动补全（兜底）
#ifndef PLATFORM_WINDOWS
    #ifndef PLATFORM_LINUX
        #ifdef _WIN32
            #define PLATFORM_WINDOWS 1
        #else
            #define PLATFORM_LINUX 1
        #endif
    #endif
#endif

// ========== 第二步：严格隔离平台专属头文件 ==========
#ifdef PLATFORM_WINDOWS
    // Windows 专属头文件（只在Windows下包含）
    #include <winsock2.h>
    #include <windows.h>
    // Windows Socket类型定义
    #define SOCKET_TYPE SOCKET
    #define CLOSE_SOCKET(s) closesocket(s)
    #define INVALID_SOCKET_VAL INVALID_SOCKET // 重命名避免冲突
#else
    // Linux 专属头文件（只在Linux下包含）
    #include <sys/socket.h>
    #include <unistd.h>
    #include <fcntl.h>   // F_GETFL/F_SETFL/O_NONBLOCK
    #include <unistd.h>  // fcntl
    #include <errno.h>
    // Linux Socket类型定义
    #define SOCKET_TYPE int
    #define CLOSE_SOCKET(s) close(s)
    #define INVALID_SOCKET_VAL (-1)
#endif

// ========== 第三步：跨平台工具函数声明（无平台差异） ==========
namespace PlatformUtils {
    // 1. 跨平台文件存在性检查（替代filesystem，兼容所有C++版本）
    bool fileExists(const std::string& path);
    bool deleteFile(const std::string& path, bool verbose = true);

    // 2. 跨平台设置Socket接收超时
    bool setSocketRecvTimeout(SOCKET_TYPE sock, std::chrono::milliseconds timeout);

    // 3. 跨平台强制刷新输出（解决Windows日志吞掉问题）
    void flushConsole();

    // 4. Windows专属：初始化WSA（Linux下空实现）
    bool initSocketEnv();

    // 5. Windows专属：清理WSA（Linux下空实现）
    void cleanupSocketEnv();
    
    bool setSocketNonBlocking(SOCKET_TYPE sock);

    // 8. 跨平台恢复Socket为阻塞模式
    bool setSocketBlocking(SOCKET_TYPE sock);
    
    bool isSocketTimeout();
}