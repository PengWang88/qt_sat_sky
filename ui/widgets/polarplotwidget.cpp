#include "polarplotwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <QDebug>
#include <cmath>

PolarPlotWidget::PolarPlotWidget(QWidget *parent)
    : QWidget(parent)
    , m_plotSize(400, 400)
    , m_center(200, 200)
    , m_radius(180)
    , m_gridColor(Qt::darkGray)
    , m_radialColor(Qt::lightGray)
    , m_angularColor(Qt::lightGray)
    , m_labelColor(Qt::black)
    , m_backgroundColor(Qt::white)
    , m_satellitePoints()
    , m_showTrajectory(false)
    , m_showTrajectoryLabels(false)
{
    setMinimumSize(400, 400);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void PolarPlotWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // 绘制背景
    painter.fillRect(rect(), m_backgroundColor);
    
    // 计算绘图区域，增加边距
    int side = qMin(width(), height());
    m_plotSize = QSize(side, side);
    m_center = QPointF(width() / 2.0, height() / 2.0);
    m_radius = side / 2.0 - 50; // 增加边距，从20改为50
    
    // 绘制极坐标网格
    drawPolarGrid(painter);
    
    // 绘制卫星点
    drawSatellitePoints(painter);
}

void PolarPlotWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    update(); // 重绘
}

void PolarPlotWidget::drawPolarGrid(QPainter &painter)
{
    // 绘制外圆
    painter.setPen(QPen(m_gridColor, 2));
    painter.drawEllipse(m_center, m_radius, m_radius);
    
    // 绘制径向线
    drawRadialLines(painter);
    
    // 绘制角度线
    drawAngularLines(painter);
    
    // 绘制标签
    drawLabels(painter);
}

void PolarPlotWidget::drawRadialLines(QPainter &painter)
{
    // 绘制主同心圆（每20度一个）
    painter.setPen(QPen(m_radialColor, 1, Qt::SolidLine));
    for (int i = 1; i <= 5; ++i) {
        double radius = m_radius * i / 5.0;
        painter.drawEllipse(m_center, radius, radius);
    }
    
    // 绘制次同心圆（每10度一个，虚线）
    painter.setPen(QPen(m_radialColor, 0.5, Qt::DotLine));
    for (int i = 1; i <= 9; ++i) {
        if (i % 2 != 0) continue; // 跳过主网格线
        double radius = m_radius * i / 10.0;
        painter.drawEllipse(m_center, radius, radius);
    }
}

void PolarPlotWidget::drawAngularLines(QPainter &painter)
{
    // 绘制主角度线（每30度一条）
    painter.setPen(QPen(m_angularColor, 1, Qt::SolidLine));
    for (int i = 0; i < 12; ++i) {
        double angle = i * 30.0;
        QPointF endPoint = polarToCartesian(m_radius, angle);
        painter.drawLine(m_center, endPoint);
    }
    
    // 绘制次角度线（每10度一条，虚线）
    painter.setPen(QPen(m_angularColor, 0.5, Qt::DotLine));
    for (int i = 0; i < 36; ++i) {
        if (i % 3 == 0) continue; // 跳过主网格线
        double angle = i * 10.0;
        QPointF endPoint = polarToCartesian(m_radius, angle);
        painter.drawLine(m_center, endPoint);
    }
}

void PolarPlotWidget::drawLabels(QPainter &painter)
{
    painter.setPen(QPen(m_labelColor, 1));
    QFont font = painter.font();
    font.setPointSize(9);
    font.setBold(true);
    painter.setFont(font);
    
    // 绘制高度角标签（仰角标签）- 竖着放在方位角0度（N方向）那一列
    QStringList elevationLabels = {"90°", "70°", "50°", "30°", "10°", "0°"};
    
    for (int i = 0; i <= 5; ++i) {
        double radius = m_radius * i / 5.0;
        QString label = elevationLabels[i];
        QFontMetrics metrics(font);
        int textWidth = metrics.horizontalAdvance(label);
        int textHeight = metrics.height();
        
        // 在方位角0度（N方向）那一列显示仰角标签
        // 使用0度方位角，标签位置在垂直方向
        QPointF labelPos = polarToCartesian(radius, 0);
        
        // 调整标签位置使其垂直对齐
        labelPos.setX(labelPos.x() - textWidth / 2);
        labelPos.setY(labelPos.y() + textHeight / 3);
        
        // 只在主网格线上显示标签（除了中心点）
        if (i > 0) {
            painter.drawText(labelPos, label);
        }
    }
    
    // 绘制角度标签（方位角标签）
    // 调整标签顺序：0度在上方（北），90度在右侧（东）
    QStringList azimuthLabels = {"N", "30°", "60°", "E", "120°", "150°", 
                                "S", "210°", "240°", "W", "300°", "330°"};
    
    for (int i = 0; i < 12; ++i) {
        double angle = i * 30.0;
        
        // 计算标签位置（稍微超出外圆）
        double labelRadius = m_radius + 30;
        QPointF labelPos = polarToCartesian(labelRadius, angle);
        
        QFontMetrics metrics(font);
        int textWidth = metrics.horizontalAdvance(azimuthLabels[i]);
        int textHeight = metrics.height();
        
        // 调整标签位置使其居中对齐
        labelPos.setX(labelPos.x() - textWidth / 2);
        labelPos.setY(labelPos.y() + textHeight / 3);
        
        // 为基本方向（N, E, S, W）使用更大的字体
        if (i % 3 == 0) {
            QFont directionFont = font;
            directionFont.setPointSize(11);
            directionFont.setBold(true);
            painter.setFont(directionFont);
            painter.drawText(labelPos, azimuthLabels[i]);
            painter.setFont(font);
        } else {
            painter.drawText(labelPos, azimuthLabels[i]);
        }
    }
    
    // 绘制中心点标签
    font.setPointSize(10);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(m_center.x() - 10, m_center.y() + 5, "天顶");
}

QPointF PolarPlotWidget::polarToCartesian(double radius, double angle)
{
    // 标准方位角：0度在上方（北），顺时针方向增加
    // 数学角度：0度在右侧，逆时针方向增加
    // 所以需要将方位角转换为数学角度：数学角度 = 90 - 方位角
    double mathAngle = 90.0 - angle;
    
    // 角度转换为弧度
    double radian = mathAngle * M_PI / 180.0;
    
    // 极坐标转笛卡尔坐标
    double x = m_center.x() + radius * std::cos(radian);
    double y = m_center.y() - radius * std::sin(radian); // Y轴向下为正，所以取负
    
    return QPointF(x, y);
}

void PolarPlotWidget::addSatellitePoint(double azimuth, double elevation, const QString &name, const QColor &color)
{
    // 验证输入参数
    if (azimuth < 0 || azimuth >= 360) {
        qWarning() << "Invalid azimuth:" << azimuth << "- should be in range [0, 360)";
        return;
    }
    
    if (elevation < 0 || elevation > 90) {
        qWarning() << "Invalid elevation:" << elevation << "- should be in range [0, 90]";
        return;
    }
    
    // 添加卫星点到列表
    m_satellitePoints.append(SatellitePoint(azimuth, elevation, name, color));
    
    // 触发重绘
    update();
}

void PolarPlotWidget::addSatellitePoint(double azimuth, double elevation, const QString &name, const QPixmap &icon)
{
    // 验证输入参数
    if (azimuth < 0 || azimuth >= 360) {
        qWarning() << "Invalid azimuth:" << azimuth << "- should be in range [0, 360)";
        return;
    }
    
    if (elevation < 0 || elevation > 90) {
        qWarning() << "Invalid elevation:" << elevation << "- should be in range [0, 90]";
        return;
    }
    
    // 检查图片是否有效
    if (icon.isNull()) {
        qWarning() << "Invalid icon provided for satellite point";
        return;
    }
    
    // 添加卫星点到列表（使用图片）
    m_satellitePoints.append(SatellitePoint(azimuth, elevation, name, icon));
    
    // 触发重绘
    update();
}

void PolarPlotWidget::addSatellitePoint(double azimuth, double elevation, const QString &name, const QString &iconPath)
{
    // 验证输入参数
    if (azimuth < 0 || azimuth >= 360) {
        qWarning() << "Invalid azimuth:" << azimuth << "- should be in range [0, 360)";
        return;
    }
    
    if (elevation < 0 || elevation > 90) {
        qWarning() << "Invalid elevation:" << elevation << "- should be in range [0, 90]";
        return;
    }
    
    // 检查文件路径是否为空
    if (iconPath.isEmpty()) {
        qWarning() << "Empty icon path provided for satellite point";
        return;
    }
    
    // 加载图片文件
    QPixmap icon(iconPath);
    if (icon.isNull()) {
        qWarning() << "Failed to load icon from path:" << iconPath;
        return;
    }
    
    // 添加卫星点到列表（使用图片路径）
    m_satellitePoints.append(SatellitePoint(azimuth, elevation, name, icon));
    
    // 触发重绘
    update();
}

void PolarPlotWidget::clearSatellitePoints()
{
    m_satellitePoints.clear();
    update();
}

void PolarPlotWidget::setSatellitePointColor(int index, const QColor &color)
{
    if (index >= 0 && index < m_satellitePoints.size()) {
        m_satellitePoints[index].color = color;
        update();
    }
}

void PolarPlotWidget::drawSatellitePoints(QPainter &painter)
{
    // 设置点的大小和样式
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // 绘制卫星点（如果有）
    if (!m_satellitePoints.isEmpty()) {
        for (const SatellitePoint &satPoint : m_satellitePoints) {
            // 计算卫星点在极坐标图中的位置
            // 高度角转换为半径：90度对应中心点（半径=0），0度对应外圆（半径=m_radius）
            double radius = m_radius * (1.0 - satPoint.elevation / 90.0);
            
            // 转换为笛卡尔坐标
            QPointF pointPos = polarToCartesian(radius, satPoint.azimuth);
            
            // 检查是否使用图片资源
            if (!satPoint.icon.isNull()) {
                // 绘制图片
                QSize iconSize(24, 24); // 设置图标大小
                QPixmap scaledIcon = satPoint.icon.scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                
                // 计算图片绘制位置（居中）
                QPointF iconPos = pointPos - QPointF(iconSize.width() / 2.0, iconSize.height() / 2.0);
                painter.drawPixmap(iconPos, scaledIcon);
            } else {
                // 绘制颜色圆点
                painter.setPen(QPen(satPoint.color, 2));
                painter.setBrush(QBrush(satPoint.color));
                painter.drawEllipse(pointPos, 4, 4); // 绘制一个4像素半径的圆点
            }
            
            // 绘制卫星名称标签
            if (!satPoint.name.isEmpty()) {
                QFont font = painter.font();
                font.setPointSize(8);
                font.setBold(true);
                painter.setFont(font);
                
                QFontMetrics metrics(font);
                int textWidth = metrics.horizontalAdvance(satPoint.name);
                int textHeight = metrics.height();
                
                // 标签位置在点的右侧稍微偏移
                QPointF labelPos = pointPos + QPointF(8, -textHeight / 2);
                
                // 绘制文本（透明背景）
                painter.setPen(QPen(Qt::black, 1));
                painter.drawText(labelPos, satPoint.name);
            }
        }
    }
    
    // 绘制轨迹点（在卫星点之后绘制，确保轨迹在下方）
    if (m_showTrajectory && !m_trajectoryPoints.isEmpty()) {
        drawTrajectoryPoints(painter);
    }
}

// 轨迹显示相关方法实现
void PolarPlotWidget::addTrajectoryPoint(double azimuth, double elevation, const QString &name, const QColor &color)
{
    // 验证输入参数
    if (azimuth < 0 || azimuth >= 360) {
        qWarning() << "Invalid azimuth for trajectory:" << azimuth << "- should be in range [0, 360)";
        return;
    }
    
    if (elevation < 0 || elevation > 90) {
        qWarning() << "Invalid elevation for trajectory:" << elevation << "- should be in range [0, 90]";
        return;
    }
    
    // 添加轨迹点到列表
    m_trajectoryPoints.append(TrajectoryPoint(azimuth, elevation, name, color));
    
    // 触发重绘
    update();
}

void PolarPlotWidget::clearTrajectoryPoints()
{
    m_trajectoryPoints.clear();
    update();
}

void PolarPlotWidget::setShowTrajectory(bool show)
{
    m_showTrajectory = show;
    update();
}

void PolarPlotWidget::setShowTrajectoryLabels(bool show)
{
    m_showTrajectoryLabels = show;
    update();
}

void PolarPlotWidget::drawTrajectoryPoints(QPainter &painter)
{
    if (m_trajectoryPoints.isEmpty()) {
        return;
    }
    
    // 设置轨迹点的样式
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // 按卫星名称分组轨迹点
    QMap<QString, QList<QPointF>> satelliteTrajectories;
    QMap<QString, QColor> satelliteColors;
    
    for (const TrajectoryPoint &trajPoint : m_trajectoryPoints) {
        // 计算轨迹点在极坐标图中的位置
        double radius = m_radius * (1.0 - trajPoint.elevation / 90.0);
        QPointF pointPos = polarToCartesian(radius, trajPoint.azimuth);
        
        satelliteTrajectories[trajPoint.name].append(pointPos);
        satelliteColors[trajPoint.name] = trajPoint.color;
    }
    
    // 绘制每个卫星的轨迹散点
    for (auto it = satelliteTrajectories.begin(); it != satelliteTrajectories.end(); ++it) {
        const QString &satName = it.key();
        const QList<QPointF> &points = it.value();
        QColor color = satelliteColors.value(satName, Qt::gray);
        
        // 设置散点样式
        painter.setPen(QPen(color, 1));
        painter.setBrush(QBrush(color));
        
        // 绘制轨迹散点（稍大的圆点，便于观察）
        for (const QPointF &point : points) {
            painter.drawEllipse(point, 2, 2); // 绘制2像素半径的圆点
        }
        
        // 只在最后一个点显示标签（如果启用）
        if (m_showTrajectoryLabels && !satName.isEmpty()) {
            QFont font = painter.font();
            font.setPointSize(6);
            font.setBold(true);
            painter.setFont(font);
            
            QFontMetrics metrics(font);
            int textWidth = metrics.horizontalAdvance(satName);
            int textHeight = metrics.height();
            
            // 标签位置在最后一个点的右侧稍微偏移
            QPointF labelPos = points.last() + QPointF(6, -textHeight / 2);
            
            // 绘制文本（透明背景）
            painter.setPen(QPen(Qt::black, 1));
            painter.drawText(labelPos, satName);
        }
    }
}