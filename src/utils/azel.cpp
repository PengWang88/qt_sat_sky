#include "azel.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

using namespace std;
using namespace nova;

AzEl::AzEl(const std::string &filename)
{
    readAzElFile(filename);
}

void AzEl::readAzElFile(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        NOVA_LOG("Error: Could not open file " << filename);
        return;
    }

    string line;
    double currentTimestamp = 0.0;
    
    while (getline(file, line))
    {
        // 跳过空行
        if (line.empty()) continue;
        
        // 去除行首尾的空白字符
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        // 检查是否是时间戳行（以'>'开头）
        if (line[0] == '>')
        {
            // 提取时间戳
            string timestampStr = line.substr(1); // 去掉'>'字符
            timestampStr.erase(0, timestampStr.find_first_not_of(" \t"));
            timestampStr.erase(timestampStr.find_last_not_of(" \t") + 1);
            
            if (stringToDouble(timestampStr, currentTimestamp))
            {
                m_timeList.push_back(currentTimestamp);
                // 为这个时间戳创建空的卫星数据映射
                m_azelData[currentTimestamp] = std::unordered_map<string, pair<double, double>>();
            }
            else
            {
                NOVA_LOG("Error: Invalid timestamp format: " << timestampStr);
                currentTimestamp = 0.0;
            }
        }
        else if (currentTimestamp > 0.0)
        {
            // 解析卫星数据行
            string prn;
            double az, el;
            
            if (parseLine(line, currentTimestamp, prn, az, el))
            {
                m_azelData[currentTimestamp][prn] = make_pair(az, el);
            }
        }
    }
    
    file.close();
    
    // 检查是否成功读取数据
    if (!m_timeList.empty() && !m_azelData.empty())
    {
        isReadSuccess = true;
        NOVA_LOG("Successfully read " << m_timeList.size() << " timestamps with satellite data");
    }
    else
    {
        NOVA_LOG("Error: No valid data found in file");
    }
}

bool AzEl::parseLine(const std::string& line, double& timestamp, std::string& prn, double& az, double& el)
{
    istringstream iss(line);
    
    // 读取PRN（卫星标识符）
    if (!(iss >> prn))
    {
        return false;
    }
    
    // 读取方位角（Azimuth）
    if (!(iss >> az))
    {
        return false;
    }
    
    // 读取仰角（Elevation）
    if (!(iss >> el))
    {
        return false;
    }
    
    // 验证数据范围
    if (az < 0 || az > 360 || el < 0 || el > 90)
    {
        NOVA_LOG("Warning: Invalid data range for PRN " << prn << " at timestamp " << timestamp);
        return false;
    }
    
    return true;
}

const std::unordered_map<std::string, std::pair<double, double>>& AzEl::getSatelliteData(double timestamp) const
{
    static const std::unordered_map<std::string, std::pair<double, double>> emptyMap;
    
    auto it = m_azelData.find(timestamp);
    if (it != m_azelData.end())
    {
        return it->second;
    }
    
    return emptyMap;
}

VecString AzEl::getSatellitePRNs() const
{
    VecString prns;
    
    // 从第一个时间戳的数据中获取所有PRN
    if (!m_timeList.empty())
    {
        double firstTimestamp = m_timeList[0];
        auto it = m_azelData.find(firstTimestamp);
        if (it != m_azelData.end())
        {
            for (const auto& pair : it->second)
            {
                prns.push_back(pair.first);
            }
        }
    }
    
    return prns;
}

bool AzEl::getSatelliteDataAtTime(const std::string& prn, double timestamp, double& az, double& el) const
{
    auto timestampIt = m_azelData.find(timestamp);
    if (timestampIt == m_azelData.end())
    {
        return false;
    }
    
    auto prnIt = timestampIt->second.find(prn);
    if (prnIt == timestampIt->second.end())
    {
        return false;
    }
    
    az = prnIt->second.first;
    el = prnIt->second.second;
    
    return true;
}