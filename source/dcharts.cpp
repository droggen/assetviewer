#include "dcharts.h"
#include "mainwindow.h"

DChartView::DChartView(QWidget *parent) : QChartView (parent)
{
}
DChartView::DChartView(QChart *chart, QWidget *parent) : QChartView (chart,parent)
{
}
DChartView::~DChartView()
{

}

void DChartView::mouseMoveEvent(QMouseEvent *event)
{
    QChartView::mouseMoveEvent(event);
    //dprintf("mouse move event: %d %d\n",event->x(),event->y());

    // Convert widget coordinates to series coordinates
    auto const widgetPos = event->localPos();
    auto const scenePos = mapToScene(QPoint(static_cast<int>(widgetPos.x()), static_cast<int>(widgetPos.y())));
    auto const chartItemPos = chart()->mapFromScene(scenePos);
    auto const valueGivenSeries = chart()->mapToValue(chartItemPos);
    /*qDebug() << "widgetPos:" << widgetPos;
    qDebug() << "scenePos:" << scenePos;
    qDebug() << "chartItemPos:" << chartItemPos;
    qDebug() << "valSeries:" << valueGivenSeries;*/

    //emit mouseMoveSignal(widgetPos.x(),widgetPos.y());
    emit mouseMoveSignal(valueGivenSeries.x(),valueGivenSeries.y());
}
