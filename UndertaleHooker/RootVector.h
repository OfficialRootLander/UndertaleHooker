#pragma once
#ifndef ROOTVECTOR_H
#define ROOTVECTOR_H

struct RootVector {
    int x;
    int y;

    RootVector(int ex, int ey)
        : x(ex), y(ey)
    {
    }

    int GetX() const { return x; }
    int GetY() const { return y; }
};

#endif