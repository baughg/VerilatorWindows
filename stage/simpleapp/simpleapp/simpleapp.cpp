// simpleapp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "Vsimple.h"
#include <iostream>

int main(int argc, char** argv)
{    
    Verilated::commandArgs(argc, argv);   // Remember args
    Vsimple* uut = new Vsimple;   // Create instance

    uut->eval();
    uut->eval();

    uint32_t clk = 0;

    while (true)
    {
        uut->clk = ~uut->clk & 0x1;
        uut->eval();
        if (uut->clk)
        {
            uut->a = ~uut->a & 0x1;
        }

        uut->eval();
    }
    delete uut;
    return 0;
}
