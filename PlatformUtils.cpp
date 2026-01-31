#include "PlatformUtils.h"
#include <fstream>
#include <iostream>
#include <cstdio> 
#include <string> 

namespace PlatformUtils {
    // 1. 跨平台文件存在性检查
    bool fileExists(const std::string& path) {
        std::ifstream file(path.c_str());
        bool exists = file.is_open() && !file.fail();
        file.close();
        return exists;
    }

    // 2. 跨平台删除文件
    /**
     * @brief 跨平台删除指定文件
     * @param path 文件路径（相对/绝对均可）
     * @param verbose 是否打印详细日志（默认true）
     * @return bool 删除成功返回true，失败返回false
     */
    bool deleteFile(const std::string& path, bool verbose) {
        // 第一步：检查文件是否存在
        if (!fileExists(path)) {
            if (verbose) {
                std::cerr << "[WARNING] PlatformUtils::deleteFile: File not found - " << path << std::endl;
                flushConsole();
            }
            return false;
        }

        // 第二步：跨平台删除文件
        int remove_result = std::remove(path.c_str());
        bool success = (remove_result == 0);

        // 第三步：日志输出（根据verbose控制）
        if (verbose) {
            if (success) {
                std::cout << "[INFO] PlatformUtils::deleteFile: File deleted successfully - " << path << std::endl;
            } else {
                #ifdef PLATFORM_WINDOWS
                // Windows下获取具体错误码
                DWORD err_code = GetLastError();
                std::cerr << "[ERROR] PlatformUtils::deleteFile: Failed to delete file - " << path 
                          << " (Windows error code: " << err_code << ")" << std::endl;
                #else
                // Linux下获取errno
                std::cerr << "[ERROR] PlatformUtils::deleteFile: Failed to delete file - " << path 
                          << " (errno: " << errno << ")" << std::endl;
                #endif
            }
            flushConsole(); // 强制刷新日志
        }

        return success;
    }
    // 3. 跨平台设置Socket接收超时
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

    // 4. 跨平台强制刷新输出
    void flushConsole() {
        std::cout << std::flush;
        std::cerr << std::flush;
        #ifdef PLATFORM_WINDOWS
        fflush(stdout);
        fflush(stderr);
        #endif
    }

    // 5. Socket环境初始化（Windows需WSAStartup，Linux空实现）
    bool initSocketEnv() {
        #ifdef PLATFORM_WINDOWS
        WSADATA wsaData;
        return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
        #else
        return true; // Linux无需初始化
        #endif
    }

    // 6. Socket环境清理（Windows需WSACleanup，Linux空实现）
    void cleanupSocketEnv() {
        #ifdef PLATFORM_WINDOWS
        WSACleanup();
        #endif
    }
}