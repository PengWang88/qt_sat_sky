#pragma once

#define NOVA_NAMESPACE_START \
    namespace nova           \
    {
#define NOVA_NAMESPACE_END }

#if NOVA_DEBUG
    #define NOVA_LOG(msg) std::cout << msg << std::endl
#else
    #define NOVA_LOG(msg)
#endif

#include <vector>
#include <string>
#include <time.h>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <unordered_map>
#include <map>
#include <iostream>

template <typename T>
using Vec = std::vector<T>;

template <typename T>
using Vec2d = Vec<Vec<T>>;

using VecDouble = Vec<double>;

using Vec2dDouble = Vec2d<double>;

using VecString = Vec<std::string>;

using VecInt = Vec<int>;

using Timestamp = double; // 时间戳

inline Timestamp toTimestamp(VecInt epochVec)
{
    if (epochVec.size() < 7) return 0.0;

    tm timeinfo = {0};
    timeinfo.tm_year = epochVec[0] - 1900; // std::tm 的年是从 1900 开始算的
    timeinfo.tm_mon = epochVec[1] - 1;     // 月从 0 开始（0-11）
    timeinfo.tm_mday = epochVec[2];
    timeinfo.tm_hour = epochVec[3];
    timeinfo.tm_min = epochVec[4];
    timeinfo.tm_sec = epochVec[5];
    timeinfo.tm_isdst = 0; // UTC 时间，不启用夏令时

    time_t t = mktime(&timeinfo);
    if (t == -1) return 0.0;

    t = _mkgmtime(&timeinfo);
    if (t == -1) return 0.0;

    double sec = static_cast<double>(t);
    double nsec = static_cast<double>(epochVec[6]);

    return sec + nsec / 1e9;
}

/**
 * 安全地将字符串转换为 double，不抛出异常。
 * @param str 输入字符串
 * @param outValue 用于输出转换后的 double 值
 * @return bool 转换成功返回 true，失败返回 false
 */
inline bool stringToDouble(const std::string& str, double& outValue) {
    // 1. 空字符串检查
    if (str.empty()) return false;

    char* endPtr = nullptr; 
    // 重置 errno 以便检测溢出
    errno = 0; 

    // 2. 执行转换
    // std::strtod 会自动跳过前导空白符
    double result = std::strtod(str.c_str(), &endPtr);

    // 3. 安全性检查
    // 情况 A: 没有进行任何转换（例如输入 "abc"）
    if (str.c_str() == endPtr) return false;

    // 情况 B: 字符串没有被完全消耗（例如 "12.3abc"，endPtr 指向 'a'）
    // 如果允许尾部空格，可以修改此处逻辑检查是否全为空格
    if (*endPtr != '\0') return false; 

    // 情况 C: 数值越界（溢出或下溢）
    if (errno == ERANGE) return false;

    outValue = result;
    return true;
}

inline std::string tsToStr(Timestamp ts)
{
    if (ts <= 0.0) return {};
    double s_floor = std::floor(ts);
    long long nsec = static_cast<long long>(std::llround((ts - s_floor) * 1e9));
    if (nsec >= 1000000000LL) { s_floor += 1.0; nsec -= 1000000000LL; }
    time_t t = static_cast<time_t>(s_floor);
    tm* ptm = gmtime(&t);
    if (!ptm) return {};
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(4) << (ptm->tm_year + 1900) << '-'
        << std::setw(2) << (ptm->tm_mon + 1) << '-'
        << std::setw(2) << ptm->tm_mday << ' '
        << std::setw(2) << ptm->tm_hour << ':'
        << std::setw(2) << ptm->tm_min << ':'
        << std::setw(2) << ptm->tm_sec << '.'
        << std::setw(9) << nsec;
    return oss.str();
}
