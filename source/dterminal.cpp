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

#include <dterminal.h>

DTerminal::DTerminal(unsigned maxlines)
{
    DTerminal::maxlines=maxlines;
}

DTerminal::~DTerminal()
{

}
void DTerminal::clear()
{
    buffer.clear();
    linereader.clear();
}
void DTerminal::addBytes(const QByteArray &ba)
{
    // Add the data to the line reader
    linereader.add(std::string(ba.constData()));
    // Read out the lines from the line reader and put them in the list of strings structure
    while(1)
    {
        std::string str;
        bool rv = linereader.getLine(str);
        if(rv==false)
            break;
        QString qstr(str.c_str());
        buffer.append(qstr);
    }
    trimOld();
}

QString DTerminal::getText()
{
    // Assemble the strings together
    QString str="";


    for(auto i=buffer.begin();i!=buffer.end();i++)
    {
        str+=*i+"\n";
    }
    return str;
}
void DTerminal::print(QString str)
{
    str+="\n";
    addBytes(str.toUtf8());
}

void DTerminal::trimOld()
{
    // Remove the lines beyond maxlines

    if(buffer.size()>maxlines)
    {
        for(int j=0;j<buffer.size()-maxlines;j++)
        {
            buffer.removeFirst();
        }
    }
}



