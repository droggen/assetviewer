#include <QString>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>
#include <QDateTime>
#include "asset.h"
#include "mainwindow.h"




ASSETS assets_load_dir(QString directory)
{
    // Identify all assets based on the .nam extension

    //dprintf("Directory: '%s'\n",directory.toStdString().c_str());


    //QDir dir(directory);

    QDir dir("d:\\wearlab\\admin\\finance\\assetdata");
    //QDir dir("D:\\wearlab\\admin\\finance\\assettest");

    //dprintf("Num entries: %d\n",dir.count());

    QFileInfoList fileinfos = dir.entryInfoList(QStringList("*.nam"),QDir::NoFilter, QDir::NoSort);

    ASSETS assets;

    foreach(QFileInfo fileinfo,fileinfos)
    {

        ASSET asset = asset_load(fileinfo);
        if(asset.ok)
            assets.push_back(asset);


    }



    return assets;

}

/*
 * Load the sedol data
*/

ASSET asset_load(QFileInfo fi)
{
    ASSET asset;
    asset.ok = false;

    QString sedol = fi.baseName();
    QString ext = fi.completeSuffix();
    QString pth = fi.path();

    //dprintf("Loading %s %s %s\n",sedol.toStdString().c_str(),ext.toStdString().c_str(),pth.toStdString().c_str());

    // Load the name
    QString fn = pth+"/"+sedol+".nam";
    QFile file(fn);
    if(!file.open(QIODevice::ReadOnly))
    {
        dprintf("Error, cannot open %s\n",fn.toStdString().c_str());
        return asset;
    }
    QTextStream in(&file);
    QString sedolname = in.readLine();
    file.close();
    //dprintf("name: %s\n",sedolname.toStdString().c_str());



    asset.sedol = sedol;
    asset.name = sedolname;


    // Load the historical data
    fn = pth+"/"+sedol+".hist";
    QFile file2(fn);
    if(!file2.open(QIODevice::ReadOnly))
    {
        dprintf("Error, cannot open %s\n",fn.toStdString().c_str());
        return asset;
    }
    QTextStream in2(&file2);
    while(!in2.atEnd())
    {
        ASSETSAMPLE as;
        QString line = in2.readLine();
        // Simplify line: multiple spaces removed
        line = line.simplified();

        //dprintf("'%s'\n",line.toStdString().c_str());

        // Split the line
        QStringList splt = line.split(" ");
        //dprintf("split: %d\n",splt.size());
        /*for(int i=0;i<splt.size();i++)
            dprintf("%d: '%s'\n",i,splt.at(i).toStdString().c_str());*/
        if(splt.size()!=2)
        {
            dprintf("File format error in %s\n",pth.toStdString().c_str());
            return asset;
        }
        //bool ok;
        unsigned date = (unsigned)splt.at(0).toDouble();       // Time in seconds since epoch
        /*if(!ok)
        {
            dprintf("conv toint error\n");
            return asset;
        }*/
        double val = splt.at(1).toDouble();
        //dprintf("%u %lf\n",date,val);

        as.date = date;
        as.v = val;
        asset.data.push_back(as);

    }
    file2.close();

    // Load the book value
    QString fn3 = pth+"/"+sedol+".val";
    QFile file3(fn3);
    if(!file3.open(QIODevice::ReadOnly))
    {
        dprintf("Error, cannot open %s\n",fn3.toStdString().c_str());
        return asset;
    }
    QTextStream in3(&file3);
    QString value = in3.readLine();
    file3.close();
    //dprintf("val: %s\n",value.toStdString().c_str());

    asset.bookvalue = value.toDouble();

    // Load the amount of units
    QString fn4 = pth+"/"+sedol+".unt";
    QFile file4(fn4);
    if(!file4.open(QIODevice::ReadOnly))
    {
        dprintf("Error, cannot open %s\n",fn4.toStdString().c_str());
        return asset;
    }
    QTextStream in4(&file4);
    value = in4.readLine();
    file4.close();
    //dprintf("units : %s\n",value.toStdString().c_str());

    asset.units = value.toDouble();

    // Calculate most recent value
    asset.curvalue = asset.units*asset.data.back().v;
    // Calculate the gain (loss)
    asset.gain = (asset.curvalue-asset.bookvalue)/asset.bookvalue;




    asset.ok = true;
    return asset;

}
void assets_print(ASSETS assets)
{
    for(unsigned i=0;i<assets.size();i++)
    {
        asset_print(assets[i]);

    }

}
void asset_print(ASSET asset)
{
    dprintf("Asset %s:\n",asset.sedol.toStdString().c_str());
    dprintf("%s\n",asset.name.toStdString().c_str());
    for(unsigned j=0;j<asset.data.size();j++)
    {
        dprintf("\t%u %lf\n",asset.data[j].date,asset.data[j].v);
    }
}

void assets_print_basic(ASSETS assets)
{
    for(unsigned i=0;i<assets.size();i++)
    {
        dprintf("Asset %d: %s\n",i,assets[i].sedol.toStdString().c_str());
        dprintf("%s\n",assets[i].name.toStdString().c_str());
        if(assets[i].data.size())
            dprintf("Data points: %d from %u to %u\n",assets[i].data.size(),assets[i].data[0].date,assets[i].data[assets[i].data.size()-1].date);

    }
}

ASSETS assets_sort_name(ASSETS assets)
{
    ASSETS newassets;

    // Use a map with name as key - maps always sorted by key
    QMap<QString,int> map;
    // Populate
    for(unsigned i=0;i<assets.size();i++)
        map.insert(assets[i].name,i);
    QMapIterator<QString, int> i(map);
    while(i.hasNext())
    {
        i.next();
        //dprintf("key: %s val: %d\n",i.key().toStdString().c_str(),i.value());
        newassets.push_back(assets[i.value()]);
    }
    return newassets;
}
ASSETS assets_sort_sedol(ASSETS assets)
{
    ASSETS newassets;

    // Use a map with sedol as key - maps always sorted by key
    QMap<QString,int> map;
    // Populate
    for(unsigned i=0;i<assets.size();i++)
        map.insert(assets[i].sedol,i);
    QMapIterator<QString, int> i(map);
    while(i.hasNext())
    {
        i.next();
        //dprintf("key: %s val: %d\n",i.key().toStdString().c_str(),i.value());
        newassets.push_back(assets[i.value()]);
    }
    return newassets;
}
ASSETS assets_sort_bookvalue(ASSETS assets)
{
    ASSETS newassets;

    // Use a map with value as key - maps always sorted by key
    // (must use multimap)
    QMap<double,int> map;
    // Populate
    for(unsigned i=0;i<assets.size();i++)
        map.insertMulti(assets[i].bookvalue,i);
    QMapIterator<double, int> i(map);
    i.toBack();
    while(i.hasPrevious())
    {
        i.previous();
        //dprintf("key: %lf val: %d\n",i.key(),i.value());
        newassets.push_back(assets[i.value()]);
    }
    return newassets;
}
ASSETS assets_sort_curvalue(ASSETS assets)
{
    ASSETS newassets;

    // Use a map with value as key - maps always sorted by key
    // (must use multimap)
    QMap<double,int> map;
    // Populate
    for(unsigned i=0;i<assets.size();i++)
        map.insertMulti(assets[i].curvalue,i);
    QMapIterator<double, int> i(map);
    i.toBack();
    while(i.hasPrevious())
    {
        i.previous();
        //dprintf("key: %lf val: %d\n",i.key(),i.value());
        newassets.push_back(assets[i.value()]);
    }
    return newassets;
}
ASSETS assets_sort_gain(ASSETS assets)
{
    ASSETS newassets;

    // Use a map with value as key - maps always sorted by key
    // (must use multimap)
    QMap<double,int> map;
    // Populate
    for(unsigned i=0;i<assets.size();i++)
        map.insertMulti(assets[i].gain,i);
    QMapIterator<double, int> i(map);
    i.toBack();
    while(i.hasPrevious())
    {
        i.previous();
        //dprintf("key: %lf val: %d\n",i.key(),i.value());
        newassets.push_back(assets[i.value()]);
    }
    return newassets;
}
/*
 * Return subset of data starting at date
*/
ASSETS assets_after(ASSETS assets,qint64 date)
{





    ASSETS an = assets;
    // Iterate all assets
    for(unsigned i=0;i<an.size();i++)
    {
        // Find earliest entry which is after date
        for(unsigned j=0;j<an[i].data.size();j++)
        {
            if(an[i].data[j].date>=date)
            {
                // If anything to remove
                if(j>0)
                {
                    // Remove all from zero inclusive to j exclusive
                    an[i].data.erase(an[i].data.begin(),an[i].data.begin()+j);
                }
                break;
            }
        }
    }
    return an;
}


int assets_find_id_by_sedol(ASSETS assets,QString sedol)
{
    return 0;
}

// Express gain in % with 3 digits
double gain_rnd(double gain)
{
    return round(gain*100000.0)/1000.0;
}

double asset_getvaluenearesttodate(ASSET asset,qint64 date)
{
    return rand();
}

unsigned assets_findyoungest(ASSETS assets)
{
    // Find oldsest timestamp in assets - always positive
    unsigned lv=UINT_MAX;
    for(unsigned i=0;i<assets.size();i++)
    {
        lv=std::min(lv,assets[i].data.front().date);
    }
    return lv;
}
unsigned assets_findoldest(ASSETS assets)
{
    unsigned lv=0;
    for(unsigned i=0;i<assets.size();i++)
    {
        lv=std::max(lv,assets[i].data.back().date);
    }
    return lv;
}

double asset_getvalat(ASSET asset,unsigned at)
{
    // Find the asset value at time "at".
    // Relies on assumption that data is sorted
    // use a "zero hold", i.e. nearest data in past, except for the first sample which is nearest in future
    int t_at = at;
    if(asset.data.size()==0)
        return 0.0;
    for(unsigned i=1;i<asset.data.size();i++)
    {
        int t_data=asset.data[i].date;
        if(t_data-t_at>=0)
            return asset.data[i-1].v;
    }
    //dprintf("after for return value[0] %lf at %d\n",asset.data[0].v,asset.data[0].date);
    // If at is later than all data, return most recent value
    return asset.data.back().v;
}

ASSET generatesumtimeseries(ASSETS assets,QList<int> ids)
{
    /*
     * Either: convert to value, and leave the sum to be computed depending on the selected assets
     * Or: create a new vector with the sum of selected assets here
     * Must also: sumvalue percent compared to sum of bookcosts
     * Data series must be interpolated so that addition is successful? or create new series with daily points and pick
     * nearest from asset series.
    */

    //dprintf("In generatesumtimeseries\n");

    ASSET assetsum;

    // Find oldest date and most recent date
    unsigned ts_1 = assets_findoldest(assets);
    unsigned ts_0 = assets_findyoungest(assets);
    //dprintf("ts: %u %u\n",ts_0,ts_1);
    int delta = ts_1-ts_0;
    //dprintf("delta: %d\n",delta);
    //dprintf("dur: %d days\n",(ts_1-ts_0)/3600/24);

    assetsum.sedol="Sum";
    assetsum.units=0;
    assetsum.name="Sum value";
    assetsum.data.clear();

    // Generate sums only for selected assets
    double curv=0,bookv=0;
    for(int id = 0;id<ids.size();id++)
    {
        curv+=assets[ids[id]].curvalue;
        bookv+=assets[ids[id]].bookvalue;
    }
    assetsum.curvalue=curv;
    assetsum.bookvalue=bookv;
    // Calculate the gain (loss)
    assetsum.gain = (assetsum.curvalue-assetsum.bookvalue)/assetsum.bookvalue;

    //dprintf("curvalue: %lf bookvalue: %lf gain: %lf\n",assetsum.curvalue,assetsum.bookvalue,assetsum.gain);

    // Generate one data point per day - round to one day in past
    unsigned tstart = ts_1-((ts_1-ts_0)/3600/24+1)*3600*24;

    for(unsigned t=tstart;t<=ts_1;t+=3600*24)
    {
        ASSETSAMPLE as;
        as.date=t;
        // Must find nearest data point to t
        //as.v=rand();
        as.v = 0;
        for(int id = 0;id<ids.size();id++)
        {
            double v = asset_getvalat(assets[ids[id]],t) * assets[ids[id]].units;

            as.v += v;

            //dprintf("at %u asset %s v %lf\n",t,assets[selectedAssets[id]].name.toStdString().c_str(), v);
        }

        //dprintf("at %u v %lf\n",t,as.v);

        assetsum.data.push_back(as);
    }
    //asset_print(assetsum);
    return assetsum;

}

int assets_find_id_by_sedol(const ASSETS &assets,const QString &sedol)
{
    for(unsigned i=0;i<assets.size();i++)
    {
        if(assets[i].sedol==sedol)
        {
            return i;
        }
    }
    return -1;
}
