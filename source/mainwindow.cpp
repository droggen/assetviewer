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
#include <cfloat>
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
    QStringList hdr ={"SEDOL","Book value","Units","Cur value", "%",    "Name"};
    ui->twAssets->setColumnCount(6);
    ui->twAssets->setHorizontalHeaderLabels(hdr);
    QHeaderView *header = ui->twAssets->horizontalHeader();
    connect(header, SIGNAL(sectionClicked(int)), this, SLOT(onHeaderSectionClicked(int)));



    // Setting up chart
    chartxrange=0;
    chartyrangepercent=false;
    connect(ui->graphicsView,SIGNAL(mouseMoveSignal(qreal,qreal)),this,SLOT(on_chartMouseMoveSignal(qreal,qreal)));
    ui->graphicsView->setRubberBand(QChartView::HorizontalRubberBand);

    // Play with pie chart
    //test();

    pieseries = new QPieSeries();
    piechart = new QChart();
    piechart->addSeries(pieseries);
    piechart->setTitle("Simple piechart example");
    piechart->legend()->hide();
    ui->graphicsView_2->setChart(piechart);
    ui->graphicsView_2->setRenderHint(QPainter::Antialiasing);


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
    if(h==headeridx_sedol)
    {
        // Sort by SEDOL
        assets = assets_sort_sedol(assets_org);
    }
    if(h==headeridx_bookval)
    {
        // Sort by Value
        assets = assets_sort_value(assets_org);
    }
    if(h==headeridx_name)
    {
        // Sort by Name
        assets = assets_sort_name(assets_org);
    }
    if(h==headeridx_curval)
    {
        // Sort by Name
        assets = assets_sort_curvalue(assets_org);
    }
    if(h==headeridx_gain)
    {
        // Sort by Name
        assets = assets_sort_gain(assets_org);
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
        ui->twAssets->setItem(i, headeridx_sedol, newItem);
        newItem = new QTableWidgetItem(assets[i].name);
        ui->twAssets->setItem(i, headeridx_name, newItem);
        newItem = new QTableWidgetItem(QString("%1").arg(assets[i].bookvalue));
        ui->twAssets->setItem(i, headeridx_bookval, newItem);
        newItem = new QTableWidgetItem(QString("%1").arg(assets[i].curvalue));
        ui->twAssets->setItem(i, headeridx_curval, newItem);
        newItem = new QTableWidgetItem(QString("%1").arg(assets[i].units));
        ui->twAssets->setItem(i, headeridx_units, newItem);
        newItem = new QTableWidgetItem(QString("%1").arg(gain_rnd(assets[i].gain)));
        if(assets[i].gain>0)
            newItem->setTextColor(Qt::darkGreen);
        if(assets[i].gain<0)
            newItem->setTextColor(Qt::darkRed);
        ui->twAssets->setItem(i, headeridx_gain, newItem);
    }
    ui->twAssets->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->twAssets->horizontalHeader()->setStretchLastSection(true);
}
void MainWindow::fillPieChart()
{
    // QPieSeries must get sorted data by value
    ASSETS tmpassets = assets_sort_value(assets_org);

    /*QPieSeries *series = new QPieSeries();

    series->append("Jane", 1);
    series->append("Joe", 2);
    series->append("Andy", 3);
    series->append("Barbara", 4);
    series->append("Axel", 5);
    series->append("Axel2", 1);*/

    pieseries->clear();
    for(unsigned i=0;i<tmpassets.size();i++)
    {
        pieseries->append(tmpassets[i].sedol,tmpassets[i].bookvalue);
    }

    /*QPieSlice *slice = series->slices().at(1);
    slice->setExploded();
    slice->setLabelVisible();
    slice->setPen(QPen(Qt::darkGreen, 2));
    slice->setBrush(Qt::green);*/

    /*QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Simple piechart example");
    chart->legend()->hide();*/

    // Should remove previous chart (memory leak)
    //dprintf("previous chart: %p\n",ui->graphicsView_2->chart());
    //delete ui->graphicsView_2->chart();
    //ui->graphicsView_2->setChart(chart);
    //ui->graphicsView_2->setRenderHint(QPainter::Antialiasing);
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
    fillPieChart();

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
        return;*/


    selectedAssets = rows;

    plotasset(rows);


    // Must: unexplode all
    for(int i=0;i<pieseries->slices().size();i++)
    {
        pieseries->slices().at(i)->setExploded(false);
        pieseries->slices().at(i)->setLabelVisible(false);
    }

    ASSETS tmpassets = assets_sort_value(assets_org);
    if(rows.size())
    {
        // Quick hack: need better data structure. Pie chart is sorted by value; asset can be sorted by value, sedol or name.
        for(int r=0;r<rows.size();r++)
        {
            for(unsigned i=0;i<tmpassets.size();i++)
            {
                if(tmpassets[i].sedol==assets[rows[r]].sedol)
                {

                    QPieSlice *slice = pieseries->slices().at(i);
                    slice->setExploded();
                    slice->setLabelVisible();
                    break;
                    //slice->setPen(QPen(Qt::darkGreen, 2));
                    //slice->setBrush(Qt::green);
                }
            }
        }
    }


}



void MainWindow::on_pbPlot_clicked()
{

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

    // Plot individual lines if plot AV or AV*units
    if(ui->rbPlotAV->isChecked() || ui->rbPlotAVxUnits->isChecked())
    {
        for(int id = 0;id<ids.size();id++)
        {
            QLineSeries *series0 = new QLineSeries();
            for(unsigned i=0;i<assets[ids[id]].data.size();i++)
            {
                // Convert time in milliseconds
                double v;
                if(ui->rbPlotAV->isChecked())
                    v = assets[ids[id]].data[i].v;                          // NAV
                if(ui->rbPlotAVxUnits->isChecked())
                    v = assets[ids[id]].data[i].v*assets[ids[id]].units;    // Value at date
                series0->append(1000.0*assets[ids[id]].data[i].date,v);
                vmin=std::min(vmin,v);
                vmax=std::max(vmax,v);
            }

            // Connect series clicked to onSeriesClickedID via a lamba to pass the id of the series.
            // Connect with new syntax
            //connect(series0,&QLineSeries::clicked,this,&MainWindow::on_seriesClicked);
            connect(series0,&QLineSeries::clicked,this,[id,this](const QPointF &p){this->on_seriesClickedID(id,p);});
            connect(series0,&QLineSeries::hovered,this,[id,this](const QPointF &p,bool s){this->on_seriesHovered(id,p,s);});

            // Size of line
            /*QPen p = series0->pen();
            p.setWidth(p.width()+5);
            series0->setPen(p);*/



            chart->addSeries(series0);
            QString nt=assets[ids[id]].name;
            nt.truncate(30);
            series0->setName(assets[ids[id]].sedol+"<BR>"+nt);
            series0->attachAxis(axisX);
            series0->attachAxis(axisY);


        }
    }
    if(ui->rbPlotTotal->isChecked())
    {

    }
    //dprintf("min-max: %lf %lf\n",vmin,vmax);

    // Adjust the margins
    if(vmin==0 && vmax==0)
    {
        vmin=0;
        vmax=1;
    }
    //dprintf("min-max after adj: %lf %lf\n",vmin,vmax);
    double vmean = (vmax+vmin)/2.0;
    double vrange = (vmax-vmin);
    vmin=vmean-vrange*1.1/2.0;
    vmax=vmean+vrange*1.1/2.0;

    axisY->setRange(vmin,vmax);

    //chart->legend()->hide();
    /*chart->createDefaultAxes();
    // Rescale only if the axis exists (if no series is addred, no axis is created)
    if(chart->axisY())
        chart->axisY()->setRange(-3, 13);*/

    ui->graphicsView->setChart(chart);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);

    for(int i=0;i<ui->graphicsView->chart()->series().size();i++)
    {
        // Size of line
        QPen p = ((QLineSeries*)ui->graphicsView->chart()->series().at(i))->pen();
        p.setWidthF(p.widthF()+.75);
        ((QLineSeries*)ui->graphicsView->chart()->series().at(i))->setPen(p);

    }

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


void MainWindow::on_seriesClicked(const QPointF &)
{
    dprintf("hi: no id\n");
}
void MainWindow::on_seriesClickedID(int id, const QPointF &)
{
    dprintf("hi: %d\n",id);
    //ui->statusBar->showMessage(QString("SEDOL: %1").arg(assets[id].sedol),5000);
}
void MainWindow::on_seriesHovered(int id, const QPointF &,bool s)
{
    //dprintf("hovered: %d corresponding to id %d %s\n",id, selectedAssets[id],assets[selectedAssets[id]].sedol.toStdString().c_str());
    ui->statusBar->showMessage(QString("SEDOL: %1: %2").arg(assets[selectedAssets[id]].sedol).arg(assets[selectedAssets[id]].name),1000);

    QAbstractSeries *as = ui->graphicsView->chart()->series().at(id);
    QLineSeries *ls=(QLineSeries*)as;

    QPen p = ls->pen();
    if(s)
    {
        //p.setWidthF(p.widthF()+2);
        p.setStyle(Qt::DashLine);
    }
    else
    {
        //p.setWidthF(p.widthF()-2);
        p.setStyle(Qt::SolidLine);
    }
    ls->setPen(p);
}




void MainWindow::on_rbPlotAV_clicked()
{
    plotselectedasset();
}

void MainWindow::on_rbPlotAVxUnits_clicked()
{
    plotselectedasset();
}


void MainWindow::on_rbPlotUP_clicked()
{
    chartformat = chartformat_unitprice;
    updateassetsrange();
}

void MainWindow::on_rbPlotUPP_clicked()
{
    chartformat = chartformat_unitpricepercent;
    updateassetsrange();
}

void MainWindow::on_rbPlotV_clicked()
{
    chartformat = chartformat_value;
    updateassetsrange();
}

void MainWindow::on_rbPlotVP_clicked()
{
    chartformat = chartformat_valuepercent;
    updateassetsrange();
}

void MainWindow::on_rbPlotSV_clicked()
{
    chartformat = chartformat_sumvalue;
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
    /*if(chartyrangepercent)
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
    }*/
    switch(chartformat)
    {
        case chartformat_unitprice:
            // By defauilt assets.data[].v is unit price
            break;
        case chartformat_unitpricepercent:
        {
            // Convert unit price to percent change since start of display area (i.e. first sample)
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
        break;
        case chartformat_value:
        {
            // Convert to value, i.e. unit * unitprice
            for(unsigned i=0;i<assets.size();i++)
            {
                for(unsigned j=0;j<assets[i].data.size();j++)
                {
                    assets[i].data[j].v = assets[i].data[j].v*assets[i].units;
                }
            }

        }
        break;
        case chartformat_valuepercent:
        {
            // Convert to value % change, i.e. change in value (unit * unitprice) compared to book cost
            for(unsigned i=0;i<assets.size();i++)
            {
                double ref = assets[i].bookvalue;
                for(unsigned j=0;j<assets[i].data.size();j++)
                {
                    assets[i].data[j].v = (assets[i].data[j].v*assets[i].units/ref-1.0)*100;
                }
            }

        }
        break;
        case chartformat_sumvalue:
            // TBI
            /*
             * Either: convert to value, and leave the sum to be computed depending on the selected assets
             * Or: create a new vector with the sum of selected assets here
             * Must also: sumvalue percent compared to sum of bookcosts
             * Data series must be interpolated so that addition is successful? or create new series with daily points and pick
             * nearest from asset series.
            */

            break;
        //case chartformat_sumvaluepercent:
            // TBI
            //break;
        default:
            break;
    }
    plotselectedasset();
}
