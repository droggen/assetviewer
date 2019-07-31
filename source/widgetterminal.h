/*
   Copyright (C) 2017:
         Daniel Roggen, droggen@gmail.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#ifndef WIDGETTERMINAL_H
#define WIDGETTERMINAL_H

#include <QWidget>

#include "dterminal.h"

namespace Ui {
class WidgetTerminal;
}

class WidgetTerminal : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetTerminal(QWidget *parent = 0);
    ~WidgetTerminal();


    void setTitle(const QString &str);
    //void setTitleColor(QColor color);
    void updateTerminal();
    void addBytes(const QByteArray &ba);
    void addString(const QString &str);
    void print(QString str);
    void clear();

private:
    Ui::WidgetTerminal *ui;

    DTerminal *terminal;
};

#endif // WIDGETTERMINAL_H
