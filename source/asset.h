#ifndef ASSET_H
#define ASSET_H

#include <QString>
#include <QFileInfo>
#include <vector>




typedef struct
{
    unsigned date;
    double v;

} ASSETSAMPLE;


typedef struct
{
    QString name;
    QString sedol;
    bool ok;
    std::vector<ASSETSAMPLE> data;
} ASSET;

typedef std::vector<ASSET> ASSETS;

ASSETS assets_load_dir(QString directory);
ASSET asset_load(QFileInfo finfo);
void assets_print(ASSETS assets);
void assets_print_basic(ASSETS assets);
ASSETS assets_sort_name(ASSETS assets);
ASSETS assets_sort_sedol(ASSETS assets);
ASSETS assets_after(ASSETS assets,unsigned date);

#endif // ASSET_H
