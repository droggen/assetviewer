#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
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
    void on_pbLoad_clicked();

    void on_pushButton_2_clicked();

    void on_pbDeselect_clicked();

    void on_twAssets_itemSelectionChanged();
    void onHeaderSectionClicked(int);

    void on_pbPlot_clicked();

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

private:
    Ui::MainWindow *ui;
    WidgetTerminal *console;
    QLabel *statusLabel;

    ASSETS assets,assets_org;
    int chartxrange;
    bool chartyrangepercent;

    QList<int> getSelectedAssets();
    void fillAssetsTable();
    int printf(const char *format, ...);
    void test();
    void test2();
    void plotselectedasset();
    void plotasset(int id);
    void plotasset(QList<int> ids);
    void reinitui();
    void updateassetsrange();

};

int dprintf(const char *format, ...);

#endif // MAINWINDOW_H
