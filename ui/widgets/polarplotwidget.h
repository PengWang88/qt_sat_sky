#pragma once

#include <QWidget>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QPointF>
#include <QRadialGradient>
#include <QString>
#include <QVector>
#include <QPixmap>

// 卫星点数据结构
struct SatellitePoint
{
    double azimuth;    // 方位角（度）
    double elevation;  // 高度角（度）
    QString name;      // 卫星名称
    QColor color;      // 点的颜色
    QPixmap icon;      // 点的图标（可选）
    
    SatellitePoint(double az = 0, double el = 0, const QString &n = "", const QColor &c = Qt::red)
        : azimuth(az), elevation(el), name(n), color(c) {}
    
    SatellitePoint(double az, double el, const QString &n, const QPixmap &icon)
        : azimuth(az), elevation(el), name(n), color(Qt::transparent), icon(icon) {}
};

class PolarPlotWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PolarPlotWidget(QWidget *parent = nullptr);
    ~PolarPlotWidget() = default;
    
    // 添加卫星点（使用颜色）
    void addSatellitePoint(double azimuth, double elevation, const QString &name, const QColor &color = Qt::red);
    
    // 添加卫星点（使用图片资源）
    void addSatellitePoint(double azimuth, double elevation, const QString &name, const QPixmap &icon);
    
    // 添加卫星点（使用图片文件路径）
    void addSatellitePoint(double azimuth, double elevation, const QString &name, const QString &iconPath);
    
    // 清除所有卫星点
    void clearSatellitePoints();
    
    // 设置卫星点颜色
    void setSatellitePointColor(int index, const QColor &color);
    
    // 轨迹显示功能
    void addTrajectoryPoint(double azimuth, double elevation, const QString &name, const QColor &color = Qt::gray);
    void clearTrajectoryPoints();
    void setShowTrajectory(bool show);
    void setShowTrajectoryLabels(bool show);
    void drawTrajectoryPoints(QPainter &painter);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void drawPolarGrid(QPainter &painter);
    void drawRadialLines(QPainter &painter);
    void drawAngularLines(QPainter &painter);
    void drawLabels(QPainter &painter);
    void drawSatellitePoints(QPainter &painter);
    
    QPointF polarToCartesian(double radius, double angle);
    
    QSize m_plotSize;
private:
    QPointF m_center;
    double m_radius;
    
    // 颜色配置
    QColor m_gridColor;
    QColor m_radialColor;
    QColor m_angularColor;
    QColor m_labelColor;
    QColor m_backgroundColor;
    
    // 卫星点数据
    QVector<SatellitePoint> m_satellitePoints;
    
    // 轨迹相关
    struct TrajectoryPoint {
        double azimuth;
        double elevation;
        QString name;
        QColor color;
        
        TrajectoryPoint(double az, double el, const QString &n, const QColor &c = Qt::gray)
            : azimuth(az), elevation(el), name(n), color(c) {}
    };
    QVector<TrajectoryPoint> m_trajectoryPoints; // 轨迹点列表
    bool m_showTrajectory;               // 是否显示轨迹
    bool m_showTrajectoryLabels;         // 是否显示轨迹标签
};