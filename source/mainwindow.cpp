/*
 QTChars in designer: https://stackoverflow.com/questions/48362864/how-to-insert-qchartview-in-form-with-qt-designer

*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDesktopWidget>
#include <QMap>
#include <QtCharts>
#include <limits>
#include "asset.h"

MainWindow *mainwin = nullptr;

int dprintf(const char *format, ...)
{
    va_list arg;
    int done;
    char buffer[16384];

    va_start(arg, format);
    done = vsnprintf(buffer,16384, format, arg);
    va_end(arg);


    mainwin->print(buffer);



    return done;
}



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    mainwin = this;

    ui->setupUi(this);


    resize(QDesktopWidget().availableGeometry(this).size() * 0.7);

    // Finish the tableWidget setup
    QStringList hdr ={"SEDOL","Tot value","Name"};
    ui->twAssets->setColumnCount(3);
    ui->twAssets->setHorizontalHeaderLabels(hdr);
    QHeaderView *header = ui->twAssets->horizontalHeader();
    connect(header, SIGNAL(sectionClicked(int)), this, SLOT(onHeaderSectionClicked(int)));



    // Setting up chart
    chartxrange=0;
    chartyrangepercent=false;
    connect(ui->graphicsView,SIGNAL(mouseMoveSignal(qreal,qreal)),this,SLOT(on_chartMouseMoveSignal(qreal,qreal)));
    ui->graphicsView->setRubberBand(QChartView::HorizontalRubberBand);


    // Setting up status bar
    statusBar()->addWidget(statusLabel = new QLabel);
    statusLabel->setText("");
    ui->statusBar->showMessage("Starting up",2000);


    // Always create the console, but show only in DEVELMODE
    console = new WidgetTerminal(this);
    console->setTitle("Console");
    ui->verticalLayout->addWidget(console);


#ifdef DEVELMODE
    print("Starting up\n");
#else
    // Hide console
    console->setVisible(false);
    ui->label_console->setVisible(false);
#endif

}


MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::print(QString str)
{
#ifdef DEVELMODE
    console->addString(str);
#endif
}
/*
 * printf equivalent writing to the debug console
*/
int MainWindow::printf(const char *format, ...)
{
    va_list arg;
    int done;
    char buffer[16384];

    va_start(arg, format);
    done = vsnprintf(buffer,16384, format, arg);
    va_end(arg);

    print(buffer);

    return done;
}

void MainWindow::onHeaderSectionClicked(int h)
{
    dprintf("Clicked header: %d\n",h);

    // Change the sorting of the assets
    if(h==0)
    {
        // Sort by SEDOL
        assets = assets_sort_sedol(assets);
    }
    if(h==2)
    {
        // Sort by Name
        assets = assets_sort_name(assets);
    }

    fillAssetsTable();

}
void MainWindow::fillAssetsTable()
{
    // Add assets to TabelWidget
    ui->twAssets->clearContents();
    ui->twAssets->setRowCount(assets.size());
    for(unsigned i=0;i<assets.size();i++)
    {
        QTableWidgetItem *newItem;
        newItem = new QTableWidgetItem(assets[i].sedol);
        ui->twAssets->setItem(i, 0, newItem);
        newItem = new QTableWidgetItem(assets[i].name);
        ui->twAssets->setItem(i, 2, newItem);
        newItem = new QTableWidgetItem("N/A");
        ui->twAssets->setItem(i, 1, newItem);
    }
    ui->twAssets->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->twAssets->horizontalHeader()->setStretchLastSection(true);
}
void MainWindow::on_pbLoad_clicked()
{
    // Select a folder which contains the asset data
    QString directory = QFileDialog::getExistingDirectory(this, tr("Open asset folder"), "",QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    // Return if nothing selected
    if(directory.isNull())
        return;

    assets_org = assets_load_dir(directory);
    assets = assets_org;

    ui->statusBar->showMessage(QString("Loaded %1 assets").arg(assets_org.size()),2000);

    //assets_print(assets);
    assets_print_basic(assets);
    fillAssetsTable();

    reinitui();
}



void MainWindow::on_pushButton_2_clicked()
{
    getSelectedAssets();
}

QList<int> MainWindow::getSelectedAssets()
{
    // Get all the selection ranges and put each row in a list
    QList<QTableWidgetSelectionRange> sr = ui->twAssets->selectedRanges();
    QList<int> rows;
    QList<QString> ports;
    foreach(auto i,sr)
    {
        //printf("selection range: %d %d - %d %d\n",i.leftColumn(),i.topRow(),i.rightColumn(),i.bottomRow());
        for(int j=i.topRow();j<=i.bottomRow();j++)
            rows.append(j);
    }

    qSort(rows);

    /*dprintf("Current selection:\n");
    foreach(auto i,rows)
    {
        dprintf("%d\n",i);
    }*/


    return rows;
}

void MainWindow::on_pbDeselect_clicked()
{
    ui->twAssets->clearSelection();
}

void MainWindow::on_twAssets_itemSelectionChanged()
{
    dprintf("Selection changed\n");
    QList<int> rows = getSelectedAssets();
    /*if(rows.size()==0)
        return;

    plotasset(rows[0]);*/
    plotasset(rows);
}



void MainWindow::test()
{
    QLineSeries *series0 = new QLineSeries();
    QLineSeries *series1 = new QLineSeries();

    *series0 << QPointF(1, 5) << QPointF(3, 7) << QPointF(7, 6) << QPointF(9, 7)
          << QPointF(12, 6) << QPointF(16, 7) << QPointF(18, 5);
    *series1 << QPointF(1, 3) << QPointF(3, 4) << QPointF(7, 3) << QPointF(8, 2)
          << QPointF(12, 3) << QPointF(16, 4) << QPointF(18, 3);

    QAreaSeries *series = new QAreaSeries(series0, series1);
    series->setName("Batman");
    QPen pen(0x059605);
    pen.setWidth(3);
    series->setPen(pen);

    QLinearGradient gradient(QPointF(0, 0), QPointF(0, 1));
    gradient.setColorAt(0.0, 0x3cc63c);
    gradient.setColorAt(1.0, 0x26f626);
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    series->setBrush(gradient);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Simple areachart example");
    chart->createDefaultAxes();
    chart->axisX()->setRange(0, 20);
    chart->axisY()->setRange(0, 10);

    //ui->chartview->setChart(chart);
    ui->graphicsView->setChart(chart);
}
void MainWindow::test2()
{
    if(assets.size()==0)
        return;

    QLineSeries *series0 = new QLineSeries();
    for(unsigned i=0;i<assets[4].data.size();i++)
    {
        series0->append(assets[4].data[i].date,assets[4].data[i].v);
    }
    QChart *chart = new QChart();
    chart->addSeries(series0);
    ui->graphicsView->setChart(chart);


    /**series0 << QPointF(1, 5) << QPointF(3, 7) << QPointF(7, 6) << QPointF(9, 7)
          << QPointF(12, 6) << QPointF(16, 7) << QPointF(18, 5);    */
}
void MainWindow::on_pbPlot_clicked()
{
    test2();
}
void MainWindow::plotselectedasset()
{
    QList<int> ids = getSelectedAssets();
    plotasset(ids);
}
void MainWindow::plotasset(int id)
{
    if(assets.size()==0)
        return;

    QLineSeries *series0 = new QLineSeries();
    for(unsigned i=0;i<assets[id].data.size();i++)
    {
        series0->append(assets[id].data[i].date,assets[id].data[i].v);
    }
    QChart *chart = new QChart();
    chart->addSeries(series0);
    chart->createDefaultAxes();
    chart->axisY()->setRange(-3, 13);
    ui->graphicsView->setChart(chart);


    /**series0 << QPointF(1, 5) << QPointF(3, 7) << QPointF(7, 6) << QPointF(9, 7)
          << QPointF(12, 6) << QPointF(16, 7) << QPointF(18, 5);    */
}
void MainWindow::plotasset(QList<int> ids)
{
    dprintf("plotasset qlist. ids: %d\n",ids.size());
    //return;
    //if(assets.size()==0)
        //return;

    // Manually create the X, Y axis.
    QChart *chart = new QChart();
    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setTickCount(10);
    axisX->setFormat("MMM yyyy");
    axisX->setTitleText("Date");
    chart->addAxis(axisX, Qt::AlignBottom);

    QValueAxis *axisY = new QValueAxis;
    axisY->setLabelFormat("%f");
    if(chartyrangepercent)
        axisY->setTitleText("NAV [%]");
    else
        axisY->setTitleText("NAV [abs]");
    chart->addAxis(axisY, Qt::AlignLeft);


    double vmin=DBL_MAX,vmax=DBL_MIN;

    for(int id = 0;id<ids.size();id++)
    {
        QLineSeries *series0 = new QLineSeries();
        for(unsigned i=0;i<assets[ids[id]].data.size();i++)
        {
            // Convert time in milliseconds
            series0->append(1000.0*assets[ids[id]].data[i].date,assets[ids[id]].data[i].v);
            vmin=std::min(vmin,assets[ids[id]].data[i].v);
            vmax=std::max(vmax,assets[ids[id]].data[i].v);
        }
        chart->addSeries(series0);
        QString nt=assets[ids[id]].name;
        nt.truncate(30);
        series0->setName(assets[ids[id]].sedol+"<BR>"+nt);
        series0->attachAxis(axisX);
        series0->attachAxis(axisY);


    }
    dprintf("min-max: %lf %lf\n",vmin,vmax);

    // Adjust the margins

    if(vmin==0)
        dprintf("vmin is zero\n");
    else
        dprintf("vmin is not zero\n");
    if(vmax==0)
        dprintf("vmax is zero\n");
    else
        dprintf("vmax is not zero\n");
    if(vmin==0 && vmax==0)
    {
        vmin=0;
        vmax=1;
    }
    dprintf("min-max after adj: %lf %lf\n",vmin,vmax);
    vmin=vmin*0.9;
    vmax=vmax*1.1;

    axisY->setRange(vmin,vmax);
    //chart->legend()->hide();


    /*chart->createDefaultAxes();
    // Rescale only if the axis exists (if no series is addred, no axis is created)
    if(chart->axisY())
        chart->axisY()->setRange(-3, 13);*/
    ui->graphicsView->setChart(chart);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);

}

void MainWindow::on_rbDateRange1m_clicked()
{
    chartxrange=1;
    updateassetsrange();
}

void MainWindow::on_rbDateRange3m_clicked()
{
    chartxrange=3;
    updateassetsrange();
}

void MainWindow::on_rbDateRange6m_clicked()
{
    chartxrange=6;
    updateassetsrange();
}

void MainWindow::on_rbDateRange12m_clicked()
{
    chartxrange=12;
    updateassetsrange();
}

void MainWindow::on_rbDateRange3y_clicked()
{
    chartxrange=3*12;
    updateassetsrange();
}

void MainWindow::on_rbDateRange5y_clicked()
{
    chartxrange=5*12;
    updateassetsrange();
}

void MainWindow::on_rbDateRangeAll_clicked()
{
    chartxrange=0;
    updateassetsrange();
}
void MainWindow::updateassetsrange()
{
    if(chartxrange)
    {
        QDateTime fromdate = QDateTime::currentDateTime();
        fromdate = fromdate.addMonths(-chartxrange);
        unsigned unixtime = fromdate.toSecsSinceEpoch();
        assets = assets_after(assets_org,unixtime);
    }
    else
        assets = assets_org;
    if(chartyrangepercent)
    {
        // Convert from absolute to percent
        for(unsigned i=0;i<assets.size();i++)
        {
            if(assets[i].data.size()!=0 && assets[i].data[0].v!=0)
            {
                double ref = assets[i].data[0].v;
                for(unsigned j=0;j<assets[i].data.size();j++)
                {
                    assets[i].data[j].v = (assets[i].data[j].v/ref-1.0)*100;
                }
            }
        }
    }
    plotselectedasset();
}

void MainWindow::reinitui()
{
    ui->rbDateRangeAll->setChecked(true);
}

void MainWindow::on_rbYAbsolute_clicked()
{
    chartyrangepercent = false;
    updateassetsrange();
}

void MainWindow::on_rbYPercentage_clicked()
{
    chartyrangepercent = true;
    updateassetsrange();
}

void MainWindow::on_chartMouseMoveSignal(qreal x, qreal y)
{
    // x is date in milliseconds
    //dprintf("on_chartMouseMoveSignal %f %f\n",(float)x,(float)y);

    // Convert x to date
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(x);

    //statusLabel->setText(QString("%1;%2 %3").arg(x).arg(y).arg(dt.toString("dd MMM yyyy")));
    statusLabel->setText(dt.toString("dd MMM yyyy"));

}



