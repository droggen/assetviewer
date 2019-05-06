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
#include <algorithm>
#include "dlinereader.h"

DLineReader::DLineReader()
{
    buffer.clear();
}

DLineReader::~DLineReader()
{

}

void DLineReader::clear()
{
    buffer.clear();
}
void DLineReader::add(const std::string& str)
{
    buffer.append(str);
}
int DLineReader::size()
{
    return buffer.size();
}

bool DLineReader::getLine(std::string &line)
{
    int p1,p2,p;

    //printf("<%s>\n",buffer.c_str());

    while(1)
    {
        // Search for the first newline characters (10 or 13)
        p1 = buffer.find(10);
        p2 = buffer.find(13);



        // Return if no newline character found
        if(p1==std::string::npos && p2==std::string::npos)
            return false;

        // First newline delimiter
        if(p1==std::string::npos || p2==std::string::npos)
        {
            //printf("max\n");
            p = std::max(p1,p2);
        }
        else
        {
            //printf("min\n");
            p=std::min(p1,p2);
        }

        //printf("%d %d %d\n",p1,p2,p);

        // If position is zero for first delimiter remove character and search for the next entry
        if(p==0)
        {
            buffer=buffer.erase(p,1);
            continue;
        }

        // Position is nonzero: return the string

        line = buffer.substr(0,p);
        buffer.erase(0,p);
        return true;
    }


}

