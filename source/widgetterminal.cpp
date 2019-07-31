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


#include <QScrollBar>
#include "widgetterminal.h"
#include "ui_widgetterminal.h"

WidgetTerminal::WidgetTerminal(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetTerminal),
    terminal(new DTerminal(1000))
{
    ui->setupUi(this);
}

WidgetTerminal::~WidgetTerminal()
{
    delete ui;
}
/*
 * Sets the string in the label about the textedit field - displays command line parameter
*/
void WidgetTerminal::setTitle(const QString &str)
{
    // Set the title
    ui->label->setText(str);
}
/*void WidgetTerminal::setTitleColor(QColor color)
{
    // Must do something with color.
    printf("Set title color\n");
}*/
void WidgetTerminal::addBytes(const QByteArray &ba)
{
    terminal->addBytes(ba);
    updateTerminal();
}
void WidgetTerminal::addString(const QString &str)
{
    print(str);
}
void WidgetTerminal::print(QString str)
{
    terminal->print(str);
    updateTerminal();
}
void WidgetTerminal::clear()
{
    terminal->clear();
    updateTerminal();
}
void WidgetTerminal::updateTerminal()
{
    QString str=terminal->getText();
    ui->uiptTerminal->setPlainText(str);
    ui->uiptTerminal->verticalScrollBar()->setValue(ui->uiptTerminal->verticalScrollBar()->maximum());
}

