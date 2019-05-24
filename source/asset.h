#ifndef ASSET_H
#define ASSET_H

#include <QString>
#include <QFileInfo>
#include <vector>
#include <QList>



typedef struct
{
    unsigned date;
    double v;

} ASSETSAMPLE;


typedef struct
{
    QString name;
    QString sedol;
    double bookvalue,curvalue;
    double units;
    double gain;
    bool ok;
    std::vector<ASSETSAMPLE> data;
} ASSET;

typedef std::vector<ASSET> ASSETS;

ASSETS assets_load_dir(QString directory);
ASSET asset_load(QFileInfo finfo);
void assets_print(ASSETS assets);
void asset_print(ASSET asset);
void assets_print_basic(ASSETS assets);
ASSETS assets_sort_name(ASSETS assets);
ASSETS assets_sort_sedol(ASSETS assets);
ASSETS assets_sort_bookvalue(ASSETS assets);
ASSETS assets_sort_curvalue(ASSETS assets);
ASSETS assets_sort_gain(ASSETS assets);
ASSETS assets_after(ASSETS assets,qint64 date);
double gain_rnd(double gain);
double asset_getvaluenearesttodate(ASSET asset,qint64 date);
unsigned assets_findoldest(ASSETS assets);
unsigned assets_findyoungest(ASSETS assets);
double asset_getvalat(ASSET asset,unsigned at);
ASSET generatesumtimeseries(ASSETS assets,QList<int> ids);
int assets_find_id_by_sedol(const ASSETS &assets,const QString &sedol);

#endif // ASSET_H
