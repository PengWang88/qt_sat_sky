#include "satsky.h"
#include "polarplotwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QPixmap>
#include <QPainter>
#include <QDebug>
#include <QFile>
#include <QTimer>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QDir>
#include "core/base.h"
#include "utils/azel.h"

using namespace nova;
using namespace std;

SatSky::SatSky(QWidget *parent)
    : QMainWindow(parent)
    , m_polarPlotArea(nullptr)
    , m_controlArea(nullptr)
    , m_timeInfoLabel(nullptr)
    , m_prevButton(nullptr)
    , m_nextButton(nullptr)
    , m_playButton(nullptr)
    , m_timer(nullptr)
    , m_azel(nullptr)
    , m_currentEpochIndex(0)
    , m_isPlaying(false)
    , m_showGPS(true)
    , m_showBDS(true)
    , m_showGAL(true)
    , m_showGLO(true)
    , m_showTrajectory(false)
    , m_trajectoryButton(nullptr)
{
    // 设置窗口标题和大小
    setWindowTitle("SatSky - 卫星天空图");
    resize(1200, 800);

    // 设置UI布局
    setupUI();

    // 加载AzEl数据并显示第一个历元
    loadAzElData();
}

void SatSky::setupUI()
{
    // 设置菜单栏
    setupMenuBar();
    
    // 创建主窗口的中心部件
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 创建主水平布局
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);

    // 创建左部分：极坐标区域（固定比例70%）
    m_polarPlotArea = new PolarPlotWidget(centralWidget);
    m_polarPlotArea->setMinimumWidth(600);
    m_polarPlotArea->setMinimumHeight(600);
    m_polarPlotArea->setStyleSheet("background-color: #f0f0f0; border: 2px solid #ccc;");

    // 创建右部分：控制区域（固定比例30%）
    m_controlArea = new QWidget(centralWidget);

    // 设置控制区域的布局
    QVBoxLayout *controlLayout = new QVBoxLayout(m_controlArea);

    // 控制区域标题
    QLabel *controlTitle = new QLabel("控制面板", m_controlArea);
    controlTitle->setAlignment(Qt::AlignCenter);
    controlTitle->setStyleSheet("font-size: 16px; font-weight: bold; padding: 10px;");

    // 当前时间信息显示
    QGroupBox *timeInfoGroup = new QGroupBox("当前时间", m_controlArea);
    QVBoxLayout *timeInfoLayout = new QVBoxLayout(timeInfoGroup);
    
    m_timeInfoLabel = new QLabel("时间: 加载中...", timeInfoGroup);
    m_timeInfoLabel->setAlignment(Qt::AlignCenter);
    m_timeInfoLabel->setStyleSheet("font-size: 14px; padding: 10px; background-color: #e8f4f8; border: 1px solid #b0d0e0; border-radius: 5px;");
    timeInfoLayout->addWidget(m_timeInfoLabel);

    // 时间控制分组
    QGroupBox *timeGroup = new QGroupBox("时间控制", m_controlArea);
    QVBoxLayout *timeLayout = new QVBoxLayout(timeGroup);

    // 创建时间控制按钮
    m_prevButton = new QPushButton("上一历元", timeGroup);
    m_nextButton = new QPushButton("下一历元", timeGroup);
    m_playButton = new QPushButton("播放", timeGroup);
    QPushButton *m_resetButton = new QPushButton("重置", timeGroup);
    
    // 设置按钮样式
    m_prevButton->setStyleSheet("QPushButton { padding: 8px; font-size: 14px; }");
    m_nextButton->setStyleSheet("QPushButton { padding: 8px; font-size: 14px; }");
    m_playButton->setStyleSheet("QPushButton { padding: 8px; font-size: 14px; background-color: #4CAF50; color: white; }");
    m_resetButton->setStyleSheet("QPushButton { padding: 8px; font-size: 14px; background-color: #FF9800; color: white; }");

    // 连接按钮信号到槽函数
    connect(m_prevButton, &QPushButton::clicked, this, &SatSky::onPrevEpoch);
    connect(m_nextButton, &QPushButton::clicked, this, &SatSky::onNextEpoch);
    connect(m_playButton, &QPushButton::clicked, this, &SatSky::onPlayPause);
    connect(m_resetButton, &QPushButton::clicked, this, &SatSky::onReset);

    // 创建历元跳转输入框
    QHBoxLayout *jumpLayout = new QHBoxLayout();
    QLabel *jumpLabel = new QLabel("跳转到历元:", timeGroup);
    QLineEdit *epochInput = new QLineEdit(timeGroup);
    epochInput->setObjectName("epochInput");
    epochInput->setPlaceholderText("输入历元编号 (1-2880)");
    epochInput->setValidator(new QIntValidator(1, 2880, this));
    QPushButton *jumpButton = new QPushButton("跳转", timeGroup);
    jumpButton->setStyleSheet("QPushButton { padding: 6px; font-size: 12px; }");
    
    jumpLayout->addWidget(jumpLabel);
    jumpLayout->addWidget(epochInput);
    jumpLayout->addWidget(jumpButton);

    // 创建按钮布局
    QHBoxLayout *buttonLayout1 = new QHBoxLayout();
    buttonLayout1->addWidget(m_prevButton);
    buttonLayout1->addWidget(m_nextButton);
    
    QHBoxLayout *buttonLayout2 = new QHBoxLayout();
    buttonLayout2->addWidget(m_playButton);
    buttonLayout2->addWidget(m_resetButton);

    timeLayout->addLayout(jumpLayout);
    timeLayout->addLayout(buttonLayout1);
    timeLayout->addLayout(buttonLayout2);
    
    // 连接跳转功能
    connect(jumpButton, &QPushButton::clicked, this, &SatSky::onJumpToEpoch);



    // 卫星系统选择分组
    QGroupBox *satelliteGroup = new QGroupBox("卫星系统选择", m_controlArea);
    QVBoxLayout *satelliteLayout = new QVBoxLayout(satelliteGroup);

    // 创建卫星系统选择复选框
    QCheckBox *gpsCheckBox = new QCheckBox("GPS (美国)", satelliteGroup);
    gpsCheckBox->setObjectName("gpsCheckBox");
    QCheckBox *bdsCheckBox = new QCheckBox("BDS (中国)", satelliteGroup);
    bdsCheckBox->setObjectName("bdsCheckBox");
    QCheckBox *galCheckBox = new QCheckBox("Galileo (欧洲)", satelliteGroup);
    galCheckBox->setObjectName("galCheckBox");
    QCheckBox *gloCheckBox = new QCheckBox("GLONASS (俄罗斯)", satelliteGroup);
    gloCheckBox->setObjectName("gloCheckBox");
    
    // 设置默认选中状态
    gpsCheckBox->setChecked(true);
    bdsCheckBox->setChecked(true);
    galCheckBox->setChecked(true);
    gloCheckBox->setChecked(true);
    
    // 设置复选框样式
    QString checkboxStyle = "QCheckBox { font-size: 12px; padding: 4px; } QCheckBox::indicator { width: 16px; height: 16px; }";
    gpsCheckBox->setStyleSheet(checkboxStyle);
    bdsCheckBox->setStyleSheet(checkboxStyle);
    galCheckBox->setStyleSheet(checkboxStyle);
    gloCheckBox->setStyleSheet(checkboxStyle);

    satelliteLayout->addWidget(gpsCheckBox);
    satelliteLayout->addWidget(bdsCheckBox);
    satelliteLayout->addWidget(galCheckBox);
    satelliteLayout->addWidget(gloCheckBox);
    
    // 连接复选框信号
    connect(gpsCheckBox, &QCheckBox::toggled, this, &SatSky::onSatelliteSystemToggled);
    connect(bdsCheckBox, &QCheckBox::toggled, this, &SatSky::onSatelliteSystemToggled);
    connect(galCheckBox, &QCheckBox::toggled, this, &SatSky::onSatelliteSystemToggled);
    connect(gloCheckBox, &QCheckBox::toggled, this, &SatSky::onSatelliteSystemToggled);

    // 轨迹显示分组
    QGroupBox *trajectoryGroup = new QGroupBox("轨迹显示", m_controlArea);
    QVBoxLayout *trajectoryLayout = new QVBoxLayout(trajectoryGroup);
    
    // 创建轨迹显示按钮
    m_trajectoryButton = new QPushButton("显示所有卫星轨迹", trajectoryGroup);
    m_trajectoryButton->setStyleSheet("QPushButton { padding: 8px; font-size: 12px; background-color: #4CAF50; color: white; border: none; border-radius: 4px; } QPushButton:hover { background-color: #45a049; }");
    
    trajectoryLayout->addWidget(m_trajectoryButton);
    
    // 连接轨迹显示按钮信号
    connect(m_trajectoryButton, &QPushButton::clicked, this, &SatSky::onToggleTrajectory);

    // 添加到控制区域布局
    controlLayout->addWidget(controlTitle);
    controlLayout->addWidget(timeInfoGroup);
    controlLayout->addWidget(timeGroup);
    controlLayout->addWidget(satelliteGroup);
    controlLayout->addWidget(trajectoryGroup);
    controlLayout->addStretch(); // 添加弹性空间

    // 设置固定比例布局（左:右 = 7:3）
    mainLayout->addWidget(m_polarPlotArea, 7); // 左部分占7份
    mainLayout->addWidget(m_controlArea, 3);   // 右部分占3份
    
    // 创建定时器
    m_timer = new QTimer(this);
    m_timer->setInterval(50); // 50ms间隔
    connect(m_timer, &QTimer::timeout, this, &SatSky::onTimerTimeout);
}

void SatSky::addTestSatellitePoints()
{
    if (!m_polarPlotArea) {
        return;
    }
    
    // 测试资源图标
    QString resourceIconPath = ":/icons/icons/flag_China.png";
    m_polarPlotArea->addSatellitePoint(300, 45, "BDS", resourceIconPath);
    resourceIconPath = ":/icons/icons/flag_EuropeanUnion.png";
    m_polarPlotArea->addSatellitePoint(270, 45, "GAL", resourceIconPath);
    resourceIconPath = ":/icons/icons/flag_Russia.png";
    m_polarPlotArea->addSatellitePoint(240, 45, "GLONASS", resourceIconPath);
    resourceIconPath = ":/icons/icons/flag_USA.png";
    m_polarPlotArea->addSatellitePoint(210, 45, "GPS", resourceIconPath);
}

void SatSky::test()
{
    NOVA_LOG("开始测试AzEl类功能");

    string filename = "C:\\Users\\peng\\Desktop\\azel.txt";
    AzEl azel(filename);
    
    if (!azel.isReaded()) {
        NOVA_LOG("错误：无法读取文件 " << filename);
        NOVA_LOG("请确保文件存在且格式正确");
        return;
    }

    // 获取时间戳列表
    const VecDouble& timeList = azel.getTimeList();
    NOVA_LOG("成功读取 " << timeList.size() << " 个时间戳");
    
    if (!timeList.empty()) {
        // 显示第一个时间戳
        double firstTimestamp = timeList[0];
        NOVA_LOG("第一个时间戳: " << firstTimestamp);
        NOVA_LOG("时间字符串: " << tsToStr(firstTimestamp));
        
        // 获取该时间戳的卫星数据
        const auto& satelliteData = azel.getSatelliteData(firstTimestamp);
        NOVA_LOG("该时间戳有 " << satelliteData.size() << " 颗卫星");
        
        // 清除之前的测试卫星点
        if (m_polarPlotArea) {
            m_polarPlotArea->clearSatellitePoints();
        }
        
        // 绘制第一个时间戳的卫星，根据PRN前缀选择国旗
        for (const auto& [prn, data] : satelliteData) {
            double az = data.first;
            double el = data.second;
            NOVA_LOG("卫星 " << prn << ": 方位角=" << az << "°, 仰角=" << el << "°");
            
            // 根据PRN前缀选择国旗
            QString flagPath = getFlagByPRN(QString::fromStdString(prn));
            
            // 在极坐标图上添加卫星点
            if (m_polarPlotArea) {
                m_polarPlotArea->addSatellitePoint(az, el, QString::fromStdString(prn), flagPath);
            }
        }
        
        // 获取所有卫星的PRN列表
        VecString prns = azel.getSatellitePRNs();
        NOVA_LOG("所有卫星PRN列表: ");
        for (const auto& prn : prns) {
            NOVA_LOG("  - " << prn);
        }
        
        // 测试获取特定卫星的数据
        if (!prns.empty()) {
            string testPRN = prns[0];
            double testAz, testEl;
            if (azel.getSatelliteDataAtTime(testPRN, firstTimestamp, testAz, testEl)) {
                NOVA_LOG("测试卫星 " << testPRN << " 的数据: 方位角=" << testAz << "°, 仰角=" << testEl << "°");
            }
        }
    }
    
    NOVA_LOG("AzEl类测试完成");
}

// 根据PRN前缀选择对应的国旗图标路径
QString SatSky::getFlagByPRN(const QString &prn)
{
    // 提取PRN前缀（前1-3个字符）
    QString prefix;
    if (prn.length() >= 3) {
        prefix = prn.left(3);
    } else {
        prefix = prn;
    }
    
    // 根据PRN前缀选择国旗
    if (prefix.startsWith("C") || prefix.startsWith("B")) {
        // 中国卫星系统（BDS/北斗）
        return ":/icons/icons/flag_China.png";
    } else if (prefix.startsWith("G")) {
        // 美国GPS系统
        return ":/icons/icons/flag_USA.png";
    } else if (prefix.startsWith("R")) {
        // 俄罗斯GLONASS系统
        return ":/icons/icons/flag_Russia.png";
    } else if (prefix.startsWith("E")) {
        // 欧洲Galileo系统
        return ":/icons/icons/flag_EuropeanUnion.png";
    } else if (prefix.startsWith("J")) {
        // 日本QZSS系统 - 使用欧洲国旗作为替代（因为缺少日本国旗）
        return ":/icons/icons/flag_EuropeanUnion.png";
    } else if (prefix.startsWith("I")) {
        // 印度IRNSS系统 - 使用中国国旗作为替代（因为缺少印度国旗）
        return ":/icons/icons/flag_China.png";
    } else {
        // 默认使用中国国旗
        return ":/icons/icons/flag_China.png";
    }
}

// 加载AzEl数据
void SatSky::loadAzElData()
{
    string filename = "C:\\Users\\peng\\Desktop\\azel.txt";
    m_azel = new AzEl(filename);
    
    if (!m_azel->isReaded()) {
        NOVA_LOG("错误：无法读取文件 " << filename);
        if (m_timeInfoLabel) {
            m_timeInfoLabel->setText("错误：无法加载数据文件");
        }
        return;
    }

    // 获取时间戳列表
    const VecDouble& timeList = m_azel->getTimeList();
    NOVA_LOG("成功读取 " << timeList.size() << " 个时间戳");
    
    if (timeList.empty()) {
        if (m_timeInfoLabel) {
            m_timeInfoLabel->setText("错误：数据文件为空");
        }
        return;
    }

    // 设置当前历元为第一个
    m_currentEpochIndex = 0;
    
    // 显示第一个历元
    displayCurrentEpoch();
}

// 显示当前历元的卫星数据
void SatSky::displayCurrentEpoch()
{
    if (!m_azel || !m_polarPlotArea) {
        return;
    }

    const VecDouble& timeList = m_azel->getTimeList();
    if (m_currentEpochIndex < 0 || m_currentEpochIndex >= timeList.size()) {
        return;
    }

    // 获取当前时间戳
    double currentTimestamp = timeList[m_currentEpochIndex];
    
    // 如果当前处于轨迹显示模式，则不显示当前历元的卫星
    if (m_showTrajectory) {
        // 更新时间信息显示为轨迹模式
        if (m_timeInfoLabel) {
            QString timeInfo = QString("轨迹显示模式\n当前历元: %1/%2\n时间: %3")
                .arg(m_currentEpochIndex + 1)
                .arg(timeList.size())
                .arg(QString::fromStdString(tsToStr(currentTimestamp)));
            m_timeInfoLabel->setText(timeInfo);
        }
        return;
    }
    
    // 清除之前的卫星点和轨迹数据（确保显示分离）
    m_polarPlotArea->clearSatellitePoints();
    m_polarPlotArea->clearTrajectoryPoints();
    
    // 关闭轨迹显示状态
    m_polarPlotArea->setShowTrajectory(false);
    
    // 获取当前时间戳的卫星数据
    const auto& satelliteData = m_azel->getSatelliteData(currentTimestamp);
    NOVA_LOG("显示第 " << m_currentEpochIndex + 1 << " 个历元，有 " << satelliteData.size() << " 颗卫星");

    // 绘制当前历元的卫星
    int displayedSatellites = 0;
    for (const auto& [prn, data] : satelliteData) {
        double az = data.first;
        double el = data.second;
        QString prnStr = QString::fromStdString(prn);
        
        // 根据卫星系统过滤
        QString systemPrefix = prnStr.left(1);
        bool shouldDisplay = false;
        
        if (systemPrefix == "G" && m_showGPS) shouldDisplay = true;
        else if (systemPrefix == "C" && m_showBDS) shouldDisplay = true;
        else if (systemPrefix == "E" && m_showGAL) shouldDisplay = true;
        else if (systemPrefix == "R" && m_showGLO) shouldDisplay = true;
        
        // 如果卫星系统被选中，则显示该卫星
        if (shouldDisplay) {
            // 获取国旗图片路径
            QString flagPath = getFlagByPRN(prnStr);
            
            // 添加卫星点（使用国旗图片）
            m_polarPlotArea->addSatellitePoint(az, el, prnStr, flagPath);
            displayedSatellites++;
        }
    }
    
    // 更新时间信息显示
    if (m_timeInfoLabel) {
        QString timeStr = QString::fromStdString(tsToStr(currentTimestamp));
        QString timeInfo = QString("当前历元: %1/%2\n时间: %3\n可见卫星: %4")
            .arg(m_currentEpochIndex + 1)
            .arg(timeList.size())
            .arg(timeStr)
            .arg(displayedSatellites);
        m_timeInfoLabel->setText(timeInfo);
    }
    
    NOVA_LOG("显示第 " << m_currentEpochIndex + 1 << " 个历元，过滤后显示 " << displayedSatellites << " 颗卫星");
}

// 上一历元按钮槽函数
void SatSky::onPrevEpoch()
{
    if (!m_azel) {
        return;
    }

    const VecDouble& timeList = m_azel->getTimeList();
    if (timeList.empty()) {
        return;
    }

    // 停止播放（如果正在播放）
    if (m_isPlaying) {
        m_timer->stop();
        m_isPlaying = false;
        m_playButton->setText("播放");
        m_playButton->setStyleSheet("QPushButton { padding: 8px; font-size: 14px; background-color: #4CAF50; color: white; }");
    }

    // 移动到上一历元
    if (m_currentEpochIndex > 0) {
        m_currentEpochIndex--;
        displayCurrentEpoch();
    }
}

// 下一历元按钮槽函数
void SatSky::onNextEpoch()
{
    if (!m_azel) {
        return;
    }

    const VecDouble& timeList = m_azel->getTimeList();
    if (timeList.empty()) {
        return;
    }

    // 停止播放（如果正在播放）
    if (m_isPlaying) {
        m_timer->stop();
        m_isPlaying = false;
        m_playButton->setText("播放");
        m_playButton->setStyleSheet("QPushButton { padding: 8px; font-size: 14px; background-color: #4CAF50; color: white; }");
    }

    // 移动到下一历元
    if (m_currentEpochIndex < timeList.size() - 1) {
        m_currentEpochIndex++;
        displayCurrentEpoch();
    }
}

// 播放/暂停按钮槽函数
void SatSky::onPlayPause()
{
    if (!m_azel) {
        return;
    }

    const VecDouble& timeList = m_azel->getTimeList();
    if (timeList.empty()) {
        return;
    }

    if (m_isPlaying) {
        // 暂停播放
        m_timer->stop();
        m_isPlaying = false;
        m_playButton->setText("播放");
        m_playButton->setStyleSheet("QPushButton { padding: 8px; font-size: 14px; background-color: #4CAF50; color: white; }");
    } else {
        // 开始播放
        m_timer->start();
        m_isPlaying = true;
        m_playButton->setText("暂停");
        m_playButton->setStyleSheet("QPushButton { padding: 8px; font-size: 14px; background-color: #f44336; color: white; }");
    }
}

// 定时器超时槽函数
void SatSky::onTimerTimeout()
{
    if (!m_azel) {
        return;
    }

    const VecDouble& timeList = m_azel->getTimeList();
    if (timeList.empty()) {
        return;
    }

    // 移动到下一历元
    if (m_currentEpochIndex < timeList.size() - 1) {
        m_currentEpochIndex++;
        displayCurrentEpoch();
    } else {
        // 到达最后一个历元，停止播放
        m_timer->stop();
        m_isPlaying = false;
        m_playButton->setText("播放");
        m_playButton->setStyleSheet("QPushButton { padding: 8px; font-size: 14px; background-color: #4CAF50; color: white; }");
    }
}

// 重置按钮槽函数 - 跳转到第一个历元
void SatSky::onReset()
{
    if (!m_azel) {
        return;
    }

    const VecDouble& timeList = m_azel->getTimeList();
    if (timeList.empty()) {
        return;
    }

    // 停止播放（如果正在播放）
    if (m_isPlaying) {
        m_timer->stop();
        m_isPlaying = false;
        m_playButton->setText("播放");
        m_playButton->setStyleSheet("QPushButton { padding: 8px; font-size: 14px; background-color: #4CAF50; color: white; }");
    }

    // 重置到第一个历元
    if (m_currentEpochIndex != 0) {
        m_currentEpochIndex = 0;
        displayCurrentEpoch();
        NOVA_LOG("重置到第一个历元");
    }
}

// 历元跳转槽函数
void SatSky::onJumpToEpoch()
{
    if (!m_azel) {
        return;
    }

    const VecDouble& timeList = m_azel->getTimeList();
    if (timeList.empty()) {
        return;
    }

    // 查找输入框（通过对象名查找）
    QLineEdit *epochInput = m_controlArea->findChild<QLineEdit*>("epochInput");
    if (!epochInput) {
        NOVA_LOG("错误：未找到历元输入框");
        return;
    }

    QString epochText = epochInput->text();
    if (epochText.isEmpty()) {
        NOVA_LOG("错误：请输入历元编号");
        return;
    }

    bool ok;
    int targetEpoch = epochText.toInt(&ok);
    if (!ok || targetEpoch < 1 || targetEpoch > timeList.size()) {
        NOVA_LOG("错误：请输入有效的历元编号 (1-" << timeList.size() << ")");
        return;
    }

    // 停止播放（如果正在播放）
    if (m_isPlaying) {
        m_timer->stop();
        m_isPlaying = false;
        m_playButton->setText("播放");
        m_playButton->setStyleSheet("QPushButton { padding: 8px; font-size: 14px; background-color: #4CAF50; color: white; }");
    }

    // 跳转到指定历元（转换为0-based索引）
    m_currentEpochIndex = targetEpoch - 1;
    displayCurrentEpoch();
    NOVA_LOG("跳转到第 " << targetEpoch << " 个历元");
}

// 卫星系统选择槽函数
void SatSky::onSatelliteSystemToggled()
{
    // 获取复选框状态
    QCheckBox *gpsCheckBox = m_controlArea->findChild<QCheckBox*>("gpsCheckBox");
    QCheckBox *bdsCheckBox = m_controlArea->findChild<QCheckBox*>("bdsCheckBox");
    QCheckBox *galCheckBox = m_controlArea->findChild<QCheckBox*>("galCheckBox");
    QCheckBox *gloCheckBox = m_controlArea->findChild<QCheckBox*>("gloCheckBox");
    
    if (!gpsCheckBox || !bdsCheckBox || !galCheckBox || !gloCheckBox) {
        qWarning() << "Failed to find satellite system checkboxes";
        return;
    }
    
    // 更新卫星系统显示状态
    m_showGPS = gpsCheckBox->isChecked();
    m_showBDS = bdsCheckBox->isChecked();
    m_showGAL = galCheckBox->isChecked();
    m_showGLO = gloCheckBox->isChecked();
    
    // 如果当前处于轨迹显示模式，重新加载轨迹数据
    if (m_showTrajectory) {
        loadAllSatelliteTrajectories();
    } else {
        // 否则重新显示当前历元
        displayCurrentEpoch();
    }
    
    NOVA_LOG("卫星系统选择更新: GPS=" << m_showGPS << ", BDS=" << m_showBDS 
           << ", GAL=" << m_showGAL << ", GLO=" << m_showGLO);
}

void SatSky::onToggleTrajectory()
{
    // 切换轨迹显示状态
    m_showTrajectory = !m_showTrajectory;
    
    // 更新按钮文本
    if (m_trajectoryButton) {
        if (m_showTrajectory) {
            m_trajectoryButton->setText("隐藏所有卫星轨迹");
            m_trajectoryButton->setStyleSheet("QPushButton { padding: 8px; font-size: 12px; background-color: #f44336; color: white; border: none; border-radius: 4px; } QPushButton:hover { background-color: #d32f2f; }");
        } else {
            m_trajectoryButton->setText("显示所有卫星轨迹");
            m_trajectoryButton->setStyleSheet("QPushButton { padding: 8px; font-size: 12px; background-color: #4CAF50; color: white; border: none; border-radius: 4px; } QPushButton:hover { background-color: #45a049; }");
        }
    }
    
    if (m_showTrajectory) {
        // 显示轨迹：加载所有历元的所有卫星数据
        loadAllSatelliteTrajectories();
    } else {
        // 隐藏轨迹：清除轨迹数据，显示当前历元
        if (m_polarPlotArea) {
            m_polarPlotArea->clearTrajectoryPoints();
        }
        displayCurrentEpoch();
    }
}

void SatSky::loadAllSatelliteTrajectories()
{
    if (!m_polarPlotArea || !m_azel) {
        return;
    }
    
    // 清除之前的轨迹数据和卫星点
    m_polarPlotArea->clearTrajectoryPoints();
    m_polarPlotArea->clearSatellitePoints();
    
    // 设置轨迹显示状态
    m_polarPlotArea->setShowTrajectory(true);
    m_polarPlotArea->setShowTrajectoryLabels(true); // 显示轨迹标签
    
    // 使用已经加载的AzEl数据
    if (!m_azel->isReaded()) {
        qWarning() << "AzEl data not loaded";
        return;
    }
    
    // 获取所有时间戳
    const VecDouble& timeList = m_azel->getTimeList();
    
    // 显示所有卫星的轨迹
    QMap<QString, QColor> satelliteColors;
    QMap<QString, int> satellitePointCounts;
    int totalTrajectoryPoints = 0;
    int visibleSatelliteCount = 0;
    
    // 为每颗卫星生成不同颜色的函数（使用美观的调色板）
    auto generateSatelliteColor = [](const QString& satelliteName) -> QColor {
        // 使用卫星名称的哈希值来生成稳定的颜色索引
        uint hash = qHash(satelliteName);
        
        // 美观的颜色调色板（精心挑选的16种颜色）
        static const QVector<QColor> colorPalette = {
            QColor(230, 25, 75),    // 红色
            QColor(60, 180, 75),    // 绿色
            QColor(255, 225, 25),   // 黄色
            QColor(0, 130, 200),    // 蓝色
            QColor(245, 130, 48),   // 橙色
            QColor(145, 30, 180),   // 紫色
            QColor(70, 240, 240),   // 青色
            QColor(240, 50, 230),   // 粉色
            QColor(210, 245, 60),   // 黄绿色
            QColor(250, 190, 190),  // 浅粉色
            QColor(0, 128, 128),    // 深青色
            QColor(230, 190, 255),  // 淡紫色
            QColor(170, 110, 40),   // 棕色
            QColor(255, 250, 200),  // 米色
            QColor(128, 0, 0),      // 深红色
            QColor(170, 255, 195)   // 薄荷绿
        };
        
        // 从调色板中选择颜色
        int index = hash % colorPalette.size();
        return colorPalette[index];
    };
    
    // 遍历所有时间戳，显示所有卫星的轨迹
    for (double timestamp : timeList) {
        // 获取该时间戳的卫星数据
        const auto& satelliteData = m_azel->getSatelliteData(timestamp);
        
        // 遍历该时间戳的所有卫星
        for (const auto& satellite : satelliteData) {
            QString satelliteName = QString::fromStdString(satellite.first);
            
            // 根据卫星系统过滤
            QString systemPrefix = satelliteName.left(1);
            if ((systemPrefix == "G" && !m_showGPS) ||
                (systemPrefix == "C" && !m_showBDS) ||
                (systemPrefix == "E" && !m_showGAL) ||
                (systemPrefix == "R" && !m_showGLO)) {
                continue; // 跳过不显示的卫星系统
            }
            
            double az = satellite.second.first;
            double el = satellite.second.second;
            
            // 为每颗卫星生成独特的颜色
            QColor color = generateSatelliteColor(satelliteName);
            
            // 添加轨迹点
            m_polarPlotArea->addTrajectoryPoint(az, el, satelliteName, color);
            
            // 统计信息
            if (!satellitePointCounts.contains(satelliteName)) {
                satellitePointCounts[satelliteName] = 0;
                visibleSatelliteCount++;
            }
            satellitePointCounts[satelliteName]++;
            totalTrajectoryPoints++;
            
            // 不限制轨迹点数，显示所有时间的轨迹
            // 轨迹点数量统计用于信息显示
        }
        
        // 继续处理下一个时间戳，显示所有时间的轨迹
    }
    
    // 强制更新显示
    m_polarPlotArea->update();
    
    // 更新时间信息显示为轨迹模式
    if (m_timeInfoLabel) {
        QString timeInfo = QString("轨迹显示模式\n显示卫星数: %1\n总轨迹点数: %2")
            .arg(visibleSatelliteCount)
            .arg(totalTrajectoryPoints);
        m_timeInfoLabel->setText(timeInfo);
    }
    
    NOVA_LOG("显示所有卫星轨迹，共 " << visibleSatelliteCount << " 颗卫星，" << totalTrajectoryPoints << " 个轨迹点");
}

// 设置菜单栏
void SatSky::setupMenuBar()
{
    // 创建菜单栏
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    // 文件菜单
    QMenu *fileMenu = menuBar->addMenu("文件(&F)");
    
    // 导出图像动作
    QAction *exportAction = new QAction("导出当前图像(&E)", this);
    exportAction->setShortcut(QKeySequence("Ctrl+E"));
    exportAction->setStatusTip("将当前极坐标图导出为图像文件");
    connect(exportAction, &QAction::triggered, this, &SatSky::onExportImage);
    fileMenu->addAction(exportAction);
    
    // 分隔符
    fileMenu->addSeparator();
    
    // 退出动作
    QAction *exitAction = new QAction("退出(&Q)", this);
    exitAction->setShortcut(QKeySequence("Ctrl+Q"));
    exitAction->setStatusTip("退出应用程序");
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAction);
    
    // 视图菜单
    QMenu *viewMenu = menuBar->addMenu("视图(&V)");
    
    // 显示/隐藏轨迹动作
    QAction *trajectoryAction = new QAction("显示/隐藏轨迹(&T)", this);
    trajectoryAction->setShortcut(QKeySequence("Ctrl+T"));
    trajectoryAction->setStatusTip("切换显示所有卫星轨迹");
    connect(trajectoryAction, &QAction::triggered, this, &SatSky::onToggleTrajectory);
    viewMenu->addAction(trajectoryAction);
}

// 导出图像槽函数
void SatSky::onExportImage()
{
    if (!m_polarPlotArea) {
        QMessageBox::warning(this, "导出错误", "极坐标区域未初始化");
        return;
    }
    
    // 获取当前图像
    QPixmap pixmap = m_polarPlotArea->grab();
    
    if (pixmap.isNull()) {
        QMessageBox::warning(this, "导出错误", "无法获取当前图像");
        return;
    }
    
    // 生成默认文件名（包含时间戳）
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString defaultFileName = QString("satellite_sky_%1.png").arg(timestamp);
    
    // 获取保存路径
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "导出图像",
        QDir::homePath() + "/" + defaultFileName,
        "PNG 图像 (*.png);;JPEG 图像 (*.jpg *.jpeg);;BMP 图像 (*.bmp)"
    );
    
    if (filePath.isEmpty()) {
        return; // 用户取消
    }
    
    // 根据文件扩展名确定保存格式
    QString format;
    if (filePath.endsWith(".png", Qt::CaseInsensitive)) {
        format = "PNG";
    } else if (filePath.endsWith(".jpg", Qt::CaseInsensitive) || filePath.endsWith(".jpeg", Qt::CaseInsensitive)) {
        format = "JPG";
    } else if (filePath.endsWith(".bmp", Qt::CaseInsensitive)) {
        format = "BMP";
    } else {
        // 默认使用PNG格式
        filePath += ".png";
        format = "PNG";
    }
    
    // 保存图像
    if (pixmap.save(filePath, format.toUtf8().constData())) {
        QMessageBox::information(this, "导出成功", 
            QString("图像已成功导出到:\n%1").arg(filePath));
        NOVA_LOG("图像已导出: " << filePath.toStdString());
    } else {
        QMessageBox::critical(this, "导出失败", 
            QString("无法保存图像到:\n%1\n请检查文件路径和权限").arg(filePath));
    }
}