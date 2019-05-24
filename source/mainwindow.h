#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QtCharts>
#include <QChart>
#include <QPieSeries>
#include "widgetterminal.h"
#include "asset.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void print(QString str);

private slots:

    void on_pushButton_2_clicked();

    void on_pbDeselect_clicked();

    void on_twAssets_itemSelectionChanged();
    void onHeaderSectionClicked(int);

    void on_pbPlot_clicked();

    void on_rbDateRange7d_clicked();

    void on_rbDateRange1m_clicked();

    void on_rbDateRange3m_clicked();

    void on_rbDateRange6m_clicked();

    void on_rbDateRange12m_clicked();

    void on_rbDateRange3y_clicked();

    void on_rbDateRange5y_clicked();

    void on_rbDateRangeAll_clicked();

    void on_rbYAbsolute_clicked();

    void on_rbYPercentage_clicked();
    void on_chartMouseMoveSignal(qreal,qreal);
    void on_seriesClicked(const QPointF &);
    void on_seriesClickedID(int id, const QPointF &);
    void on_seriesHovered(int id, const QPointF &,bool s);

    void on_rbPlotAV_clicked();

    void on_rbPlotAVxUnits_clicked();



    void on_rbPlotUP_clicked();

    void on_rbPlotUPP_clicked();

    void on_rbPlotV_clicked();

    void on_rbPlotVP_clicked();

    void on_rbPlotSV_clicked();

    void on_rbPlotVD_clicked();

    void on_rbPlotSVP_clicked();

    void on_rbPlotSVD_clicked();

    void on_rbPieBook_clicked();

    void on_rbPieCurrent_clicked();

    void on_action_Load_triggered();


    void on_action_Quit_triggered();

private:
    Ui::MainWindow *ui;
    WidgetTerminal *console;
    QLabel *statusLabel;

    ASSETS assets,assets_org;
    int chartxrange;
    bool chartyrangepercent;
    QPieSeries *pieseries;
    QChart *piechart;

    const int headeridx_sedol=0,headeridx_bookval=1,headeridx_name=5,headeridx_curval=3,headeridx_units=2,headeridx_gain=4;
    static constexpr int chartformat_unitprice=0,chartformat_value=1,chartformat_unitpricepercent=2,chartformat_valuepercent=3,chartformat_sumvalue=4,chartformat_sumvaluepercent=5,chartformat_valuedelta=6,chartformat_sumvaluedelta=7;
    static constexpr int chartsort_sedol=0,chartsort_bookval=1,chartsort_name=2,chartsort_curval=3,chartsort_gain=4;
    static constexpr int chartxrange_7d=0,chartxrange_1m=1,chartxrange_3m=2,chartxrange_6m=3,chartxrange_1y=4,chartxrange_3y=5,chartxrange_5y=6,chartxrange_all=99;
    static constexpr int piechartformat_bookval=0,piechartformat_curval=1;

    int chartformat;
    int chartsort;
    int piechartformat;
    QList<int> selectedAssets;  // Caching of selected assets when on_twAssets_itemSelectionChanged called

    //ASSET assetsum;

    QList<int> getSelectedAssets();
    void fillAssetsTable();
    void fillPieChart();
    int printf(const char *format, ...);
    void plotselectedasset();
    void plotassetchart(QList<int> ids);
    void reinitui();
    void updateassetspresentation();
    void updatepieselection(QList<int> rows);

};

int dprintf(const char *format, ...);

#endif // MAINWINDOW_H
