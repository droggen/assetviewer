/*
 QTChars in designer: https://stackoverflow.com/questions/48362864/how-to-insert-qchartview-in-form-with-qt-designer

*/
/*
 * TODO:
 * v Hovering over sum gives name of 1st sedol instead of nothing or sum
 * v Pie chart: by book value or current value
 * - Keyboard shortcut for load
 * - Keyboard shortcut for quit
 * - Prettify axis range
 * - Value at mouse location
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
    QStringList hdr ={"SEDOL","Book value","Units","Cur value", "%", "Name"};
    ui->twAssets->setColumnCount(6);
    ui->twAssets->setHorizontalHeaderLabels(hdr);
    QHeaderView *header = ui->twAssets->horizontalHeader();
    connect(header, SIGNAL(sectionClicked(int)), this, SLOT(onHeaderSectionClicked(int)));



    // Setting up chart
    chartxrange=chartxrange_all;
    chartyrangepercent=false;
    chartsort=chartsort_sedol;
    chartformat=chartformat_unitprice;
    piechartformat=piechartformat_bookval;
    reinitui();
    connect(ui->graphicsView,SIGNAL(mouseMoveSignal(qreal,qreal)),this,SLOT(on_chartMouseMoveSignal(qreal,qreal)));
    ui->graphicsView->setRubberBand(QChartView::HorizontalRubberBand);


    pieseries = new QPieSeries();
    piechart = new QChart();
    piechart->addSeries(pieseries);
    piechart->setTitle("Asset allocation");
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
        chartsort=chartsort_sedol;
    }
    if(h==headeridx_bookval)
    {
        // Sort by Value
        chartsort=chartsort_bookval;
    }
    if(h==headeridx_name)
    {
        // Sort by Name
        chartsort=chartsort_name;
    }
    if(h==headeridx_curval)
    {
        // Sort by current value
        chartsort=chartsort_curval;
    }
    if(h==headeridx_gain)
    {
        // Sort by gain
        chartsort=chartsort_gain;
    }
    updateassetspresentation();
    fillAssetsTable();
}
void MainWindow::fillAssetsTable()
{
    // Add assets to TabelWidget
    ui->twAssets->clearContents();
    // Skip last item which is sum asset
    ui->twAssets->setRowCount(assets.size()-1);
    for(unsigned i=0;i<assets.size()-1;i++)
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
    ASSETS tmpassets;
    // Sort QPieSeries by book or current value
    if(piechartformat==piechartformat_bookval)
        tmpassets = assets_sort_bookvalue(assets_org);
    else
        tmpassets = assets_sort_curvalue(assets_org);

    pieseries->clear();
    for(unsigned i=0;i<tmpassets.size();i++)
    {
        if(piechartformat==piechartformat_bookval)
            pieseries->append(tmpassets[i].sedol,tmpassets[i].bookvalue);
        else
            pieseries->append(tmpassets[i].sedol,tmpassets[i].curvalue);
    }
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
    //QList<QString> ports;
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
    assets_print(assets);
}

void MainWindow::on_twAssets_itemSelectionChanged()
{
    //dprintf("Selection changed\n");
    QList<int> rows = getSelectedAssets();
    /*if(rows.size()==0)
        return;*/


    selectedAssets = rows;

    updateassetspresentation();         // Recomputs sum based on selected
    plotassetchart(rows);


    updatepieselection(rows);



}
void MainWindow::updatepieselection(QList<int> rows)
{
    // Must: unexplode all
    for(int i=0;i<pieseries->slices().size();i++)
    {
        pieseries->slices().at(i)->setExploded(false);
        pieseries->slices().at(i)->setLabelVisible(false);
    }

    ASSETS tmpassets;
    if(piechartformat==piechartformat_bookval)
        tmpassets = assets_sort_bookvalue(assets_org);
    else
        tmpassets = assets_sort_curvalue(assets_org);
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
    plotassetchart(ids);
}
void MainWindow::plotassetchart(QList<int> ids)
{
    //dprintf("plotasset qlist. ids: %d\n",ids.size());
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

    // Plot individual lines
    if(chartformat==chartformat_sumvalue || chartformat==chartformat_sumvaluepercent || chartformat==chartformat_sumvaluedelta)
    {
        // Select the sum asset
        ids.clear();
        ids.push_back(assets.size()-1);
    }

    for(int id = 0;id<ids.size();id++)
    {
        QLineSeries *series0 = new QLineSeries();
        for(unsigned i=0;i<assets[ids[id]].data.size();i++)
        {
            // Convert time in milliseconds
            double v;
            v = assets[ids[id]].data[i].v;
            series0->append(1000.0*assets[ids[id]].data[i].date,v);
            vmin=std::min(vmin,v);
            vmax=std::max(vmax,v);
        }

        // Connect click and hover, but only for non-sum chart
        if(!(chartformat==chartformat_sumvalue || chartformat==chartformat_sumvaluepercent || chartformat==chartformat_sumvaluedelta))
        {
            // Connect series clicked to onSeriesClickedID via a lamba to pass the id of the series
            connect(series0,&QLineSeries::clicked,this,[id,this](const QPointF &p){this->on_seriesClickedID(id,p);});
            connect(series0,&QLineSeries::hovered,this,[id,this](const QPointF &p,bool s){this->on_seriesHovered(id,p,s);});
        }

        chart->addSeries(series0);
        QString nt=assets[ids[id]].name;
        nt.truncate(30);
        series0->setName(assets[ids[id]].sedol+"<BR>"+nt);
        series0->attachAxis(axisX);
        series0->attachAxis(axisY);


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

void MainWindow::on_rbDateRange7d_clicked()
{
    chartxrange=chartxrange_7d;
    updateassetspresentation();
}
void MainWindow::on_rbDateRange1m_clicked()
{
    chartxrange=chartxrange_1m;
    updateassetspresentation();
}

void MainWindow::on_rbDateRange3m_clicked()
{
    chartxrange=chartxrange_3m;
    updateassetspresentation();
}

void MainWindow::on_rbDateRange6m_clicked()
{
    chartxrange=chartxrange_6m;
    updateassetspresentation();
}

void MainWindow::on_rbDateRange12m_clicked()
{
    chartxrange=chartxrange_1y;
    updateassetspresentation();
}

void MainWindow::on_rbDateRange3y_clicked()
{
    chartxrange=chartxrange_3y;
    updateassetspresentation();
}

void MainWindow::on_rbDateRange5y_clicked()
{
    chartxrange=chartxrange_5y;
    updateassetspresentation();
}

void MainWindow::on_rbDateRangeAll_clicked()
{
    chartxrange=chartxrange_all;
    updateassetspresentation();
}

void MainWindow::reinitui()
{
    // Makes the UI radios match the model state
    switch(chartxrange)
    {
        case chartxrange_1m:
            ui->rbDateRange1m->setChecked(true);
            break;
        case chartxrange_3m:
            ui->rbDateRange3m->setChecked(true);
            break;
        case chartxrange_6m:
            ui->rbDateRange6m->setChecked(true);
            break;
        case chartxrange_1y:
            ui->rbDateRange12m->setChecked(true);
            break;
        case chartxrange_3y:
            ui->rbDateRange3y->setChecked(true);
            break;
        case chartxrange_5y:
            ui->rbDateRange5y->setChecked(true);
            break;
        case chartxrange_7d:
            ui->rbDateRange7d->setChecked(true);
            break;
        case chartxrange_all:
        default:
            ui->rbDateRangeAll->setChecked(true);
            break;
    }
    /*switch(chartsort)
    {
        case chartsort_sedol:
            ui->rb
            break;
        case chartsort_gain:
        case chartsort_name:
        case chartsort_curval:
        case chartsort_bookval:
    }*/
    switch(chartformat)
    {
        case chartformat_value:
            ui->rbPlotV->setChecked(true);
            break;
        case chartformat_sumvalue:
            ui->rbPlotSV->setChecked(true);
            break;
        case chartformat_unitprice:
            ui->rbPlotUP->setChecked(true);
            break;
        case chartformat_valuepercent:
            ui->rbPlotVP->setChecked(true);
            break;
        case chartformat_sumvaluepercent:
            ui->rbPlotSVP->setChecked(true);
            break;
        case chartformat_unitpricepercent:
        default:
            ui->rbPlotUPP->setChecked(true);
            break;
    }
}

void MainWindow::on_rbYAbsolute_clicked()
{
    chartyrangepercent = false;
    updateassetspresentation();
}

void MainWindow::on_rbYPercentage_clicked()
{
    chartyrangepercent = true;
    updateassetspresentation();
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
    updateassetspresentation();
    plotselectedasset();
}

void MainWindow::on_rbPlotUPP_clicked()
{
    chartformat = chartformat_unitpricepercent;
    updateassetspresentation();
    plotselectedasset();
}

void MainWindow::on_rbPlotV_clicked()
{
    chartformat = chartformat_value;
    updateassetspresentation();
    plotselectedasset();
}
void MainWindow::on_rbPlotVD_clicked()
{
    chartformat = chartformat_valuedelta;
    updateassetspresentation();
    plotselectedasset();
}

void MainWindow::on_rbPlotVP_clicked()
{
    chartformat = chartformat_valuepercent;
    updateassetspresentation();
    plotselectedasset();
}

void MainWindow::on_rbPlotSV_clicked()
{
    chartformat = chartformat_sumvalue;
    updateassetspresentation();
    plotselectedasset();
}
void MainWindow::on_rbPlotSVP_clicked()
{
    chartformat = chartformat_sumvaluepercent;
    updateassetspresentation();
    plotselectedasset();
}
void MainWindow::on_rbPlotSVD_clicked()
{
    chartformat = chartformat_sumvaluedelta;
    updateassetspresentation();
    plotselectedasset();
}



void MainWindow::updateassetspresentation()
{
    // Perform sorting order
    switch(chartsort)                                               // Sort
    {
        case chartsort_sedol:
            assets = assets_sort_sedol(assets_org);
            break;
        case chartsort_name:
            assets = assets_sort_name(assets_org);
            break;
        case chartsort_gain:
            assets = assets_sort_gain(assets_org);
            break;
        case chartsort_curval:
            assets = assets_sort_curvalue(assets_org);
            break;
        case chartsort_bookval:
            assets = assets_sort_bookvalue(assets_org);
            break;
        default:
            dprintf("Unknown sort order\n");
    }

    // Perform time limit
    switch(chartxrange)
    {
        case chartxrange_7d:
        {
            QDateTime fromdate = QDateTime::currentDateTime();
            fromdate = fromdate.addDays(-7);
            qint64 unixtime = fromdate.toSecsSinceEpoch();
            assets = assets_after(assets,unixtime);
        }
        break;
        case chartxrange_1m:
        {
            QDateTime fromdate = QDateTime::currentDateTime();
            fromdate = fromdate.addMonths(-1);
            qint64 unixtime = fromdate.toSecsSinceEpoch();
            assets = assets_after(assets,unixtime);
        }
        break;
        case chartxrange_3m:
        {
            QDateTime fromdate = QDateTime::currentDateTime();
            fromdate = fromdate.addMonths(-3);
            qint64 unixtime = fromdate.toSecsSinceEpoch();
            assets = assets_after(assets,unixtime);
        }
        break;
        case chartxrange_6m:
        {
            QDateTime fromdate = QDateTime::currentDateTime();
            fromdate = fromdate.addMonths(-6);
            qint64 unixtime = fromdate.toSecsSinceEpoch();
            assets = assets_after(assets,unixtime);
        }
        break;
        case chartxrange_1y:
        {
            QDateTime fromdate = QDateTime::currentDateTime();
            fromdate = fromdate.addMonths(-12);
            qint64 unixtime = fromdate.toSecsSinceEpoch();
            assets = assets_after(assets,unixtime);
        }
        break;
        case chartxrange_3y:
        {
            QDateTime fromdate = QDateTime::currentDateTime();
            fromdate = fromdate.addMonths(-12*3);
            qint64 unixtime = fromdate.toSecsSinceEpoch();
            assets = assets_after(assets,unixtime);
        }
        break;
        case chartxrange_5y:
        {
            QDateTime fromdate = QDateTime::currentDateTime();
            fromdate = fromdate.addMonths(-12*5);
            qint64 unixtime = fromdate.toSecsSinceEpoch();
            assets = assets_after(assets,unixtime);
        }
        break;
    }

    // Add sum asset
    ASSET as = generatesumtimeseries(assets,selectedAssets);
    assets.push_back(as);
    // Print
    //asset_print(as);





    switch(chartformat)
    {
        case chartformat_unitprice:
            // By default assets.data[].v is unit price
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
        case chartformat_valuedelta:
        {
            // Convert to value % change, i.e. change in value (unit * unitprice) compared to book cost
            for(unsigned i=0;i<assets.size();i++)
            {
                double ref = assets[i].bookvalue;
                for(unsigned j=0;j<assets[i].data.size();j++)
                {
                    assets[i].data[j].v = assets[i].data[j].v*assets[i].units-assets[i].bookvalue;
                }
            }

        }
        break;
        case chartformat_sumvalue:
            // Nothing to do for sum
            break;
        case chartformat_sumvaluepercent:           // This is same as valuepercent, only for last asset
        {
            // Convert to value % change, i.e. change in value (unit * unitprice) compared to book cost
            double ref = assets.back().bookvalue;

            for(unsigned j=0;j<assets.back().data.size();j++)
            {
                assets.back().data[j].v = (assets.back().data[j].v/ref-1.0)*100;
            }
        }
        break;
        case chartformat_sumvaluedelta:
        {
            double ref = assets.back().bookvalue;
            for(unsigned j=0;j<assets.back().data.size();j++)
            {
                assets.back().data[j].v = assets.back().data[j].v-ref;
            }
        }
        break;
        default:
            break;
    }
    //assets_print_basic(assets);
}





void MainWindow::on_rbPieBook_clicked()
{
    piechartformat=piechartformat_bookval;
    fillPieChart();
    updatepieselection(getSelectedAssets());
}

void MainWindow::on_rbPieCurrent_clicked()
{
    piechartformat=piechartformat_curval;
    fillPieChart();
    updatepieselection(getSelectedAssets());
}

void MainWindow::on_action_Load_triggered()
{
    // Select a folder which contains the asset data
    QString directory = QFileDialog::getExistingDirectory(this, tr("Open asset folder"), "",QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    // Return if nothing selected
    if(directory.isNull())
        return;

    assets_org = assets_load_dir(directory);

    assets_print_basic(assets_org);

    ui->statusBar->showMessage(QString("Loaded %1 assets").arg(assets_org.size()),2000);


    updateassetspresentation();




    chartxrange=chartxrange_all;
    chartyrangepercent=false;
    chartsort=chartsort_sedol;
    chartformat=chartformat_unitprice;
    reinitui();



    fillAssetsTable();
    fillPieChart();
}
