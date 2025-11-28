#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QFrame>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QGroupBox>
#include <QIntValidator>
#include "utils/azel.h"

class PolarPlotWidget;

class SatSky : public QMainWindow
{
    Q_OBJECT

public:
    explicit SatSky(QWidget *parent = nullptr);
    ~SatSky() = default;

private slots:
    void onPrevEpoch();
    void onNextEpoch();
    void onPlayPause();
    void onTimerTimeout();
    void onReset();
    void onJumpToEpoch();
    void onSatelliteSystemToggled();
    void onToggleTrajectory();
    void onExportImage();

private:
    void setupUI();
    void setupMenuBar();
    void addTestSatellitePoints();
    void test();
    QString getFlagByPRN(const QString &prn);
    void loadAzElData();
    void displayCurrentEpoch();
    void loadAllSatelliteTrajectories();
    
    PolarPlotWidget *m_polarPlotArea;    // 左部分：极坐标区域
    QWidget *m_controlArea;      // 右部分：控制区域
    
    // 时间控制相关
    QLabel *m_timeInfoLabel;     // 显示当前时间信息
    QPushButton *m_prevButton;   // 上一历元按钮
    QPushButton *m_nextButton;   // 下一历元按钮
    QPushButton *m_playButton;   // 播放/暂停按钮
    QTimer *m_timer;             // 定时器用于自动播放
    
    // AzEl数据相关
    nova::AzEl *m_azel;          // AzEl数据对象
    int m_currentEpochIndex;     // 当前历元索引
    bool m_isPlaying;            // 是否正在播放
    
    // 卫星系统选择相关
    bool m_showGPS;              // 是否显示GPS卫星
    bool m_showBDS;              // 是否显示BDS卫星
    bool m_showGAL;              // 是否显示Galileo卫星
    bool m_showGLO;              // 是否显示GLONASS卫星
    
    // 轨迹显示相关
    bool m_showTrajectory;       // 是否显示所有卫星轨迹
    QPushButton *m_trajectoryButton; // 轨迹显示按钮
};