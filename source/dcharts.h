#ifndef DCHARTS_H
#define DCHARTS_H

#include <QtCharts>

class DChartView : public QChartView
{
    Q_OBJECT
public:
    DChartView(QWidget *parent = nullptr);
    DChartView(QChart *chart, QWidget *parent = nullptr);
    virtual ~DChartView();
protected:
    void mouseMoveEvent(QMouseEvent *event);
signals:
    void mouseMoveSignal(qreal x,qreal y);
};


#endif // DCHARTS_H
