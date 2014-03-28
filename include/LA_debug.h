/*
 * LA_debug.h
 *
 *  Created on: 30 Apr 2013
 *      Author: erickel
 */
#pragma once

#include <list>
#include <iostream>
#include <stdint.h>

using namespace std;

class csvWriter : public ofstream
{
public:

    csvWriter(string filename);
    ~csvWriter();

    void writeLine(list<string> &l_items);

private:

};

class tsReader : public ifstream
{
public:
    int getPacket(uint8_t *pPacket);

private:
};
