#pragma once

#include "core/base.h"

NOVA_NAMESPACE_START

class AzEl
{
public:
    explicit AzEl(const std::string &filename);
    ~AzEl() {};

    bool isReaded() const { return isReadSuccess; }
    
    // 获取时间戳列表
    const VecDouble& getTimeList() const { return m_timeList; }
    
    // 获取指定时间戳的卫星数据
    const std::unordered_map<std::string, std::pair<double, double>>& getSatelliteData(double timestamp) const;
    
    // 获取所有卫星的PRN列表
    VecString getSatellitePRNs() const;
    
    // 获取指定卫星在指定时间戳的数据
    bool getSatelliteDataAtTime(const std::string& prn, double timestamp, double& az, double& el) const;

private:
    void readAzElFile(const std::string &filename);
    bool parseLine(const std::string& line, double& timestamp, std::string& prn, double& az, double& el);

    VecDouble m_timeList;
    // 使用时间戳作为键，卫星PRN到(方位角, 仰角)的映射作为值
    std::unordered_map<double, std::unordered_map<std::string, std::pair<double, double>>> m_azelData;

    bool isReadSuccess = false;
};

NOVA_NAMESPACE_END