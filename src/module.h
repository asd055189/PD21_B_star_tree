#ifndef MODULE_H
#define MODULE_H

#include <vector>
#include <string>
#include <bits/stdc++.h>
using namespace std;

class Terminal
{
public:
    // constructor and destructor
    Terminal(string& name, size_t x, size_t y) :
        _name(name), _x1(x), _y1(y), _x2(x), _y2(y) { }
    ~Terminal()  { }

    // basic access methods
    const string getName()  { return _name; }
    const size_t getX1()    { return _x1; }
    const size_t getX2()    { return _x2; }
    const size_t getY1()    { return _y1; }
    const size_t getY2()    { return _y2; }

    // set functions
    void setName(string& name) { _name = name; }
    void setPos(size_t x1, size_t y1, size_t x2, size_t y2) {
        _x1 = x1;   _y1 = y1;
        _x2 = x2;   _y2 = y2;
    }

protected:
    string      _name;      // module name
    size_t      _x1;        // min x coordinate of the terminal
    size_t      _y1;        // min y coordinate of the terminal
    size_t      _x2;        // max x coordinate of the terminal
    size_t      _y2;        // max y coordinate of the terminal
};


class Block : public Terminal
{
public:
    // constructor and destructor
    Block(string& name, int w, int h) :
        Terminal(name, 0, 0), _w(w), _h(h) { }
    ~Block() { }

    // basic access methods
    const int getWidth()  { return rotate? _h: _w; }
    const int getHeight() { return rotate? _w: _h; }
    const int getArea()  { return _h * _w; }
    static int getMaxX() { return _maxX; }
    static int getMaxY() { return _maxY; }
    bool getrotate() { return rotate; }
    // set functions
    void setWidth(int w)         { _w = w; }
    void setHeight(int h)        { _h = h; }
    static void setMaxX(int x)   { _maxX = x; }
    static void setMaxY(int y)   { _maxY = y; }
    void _rotate() { rotate = !rotate; }
private:
    bool rotate=false;
    int          _w;         // width of the block
    int          _h;         // height of the block
    static int   _maxX;      // maximum x coordinate for all blocks
    static int   _maxY;      // maximum y coordinate for all blocks
};


class Net
{
public:
    // constructor and destructor
    Net()   { }
    ~Net()  { }

    // basic access methods
    const vector<Terminal*> getTermList()   { return _termList; }

    // modify methods
    void addTerm(Terminal* term) { _termList.push_back(term); }

    // other member functions
    double calcHPWL() {
        double minx = INT_MAX, miny = INT_MAX, maxx = 0, maxy = 0;
        for (auto i : _termList) {
            double Y = (i->getY2() + i->getY1()) / 2.0, X = (i->getX2() + i->getX1()) / 2.0;
            if (X > maxx)
                maxx = X;
            if (Y > maxy)
                maxy = Y;
            if (Y < miny)
                miny = Y;
            if (X < minx)
                minx = X;
        }
        return (maxx - minx) + (maxy - miny);
    }

private:
    vector<Terminal*>   _termList;  // list of terminals the net is connected to
};

class Node
{
public:
    Block *block;
    Node* left=nullptr;
    Node* right= nullptr;
    Node* parent= nullptr;
};

class Line {
public:
    Line* next=nullptr;
    int x1;
    int x2;
    int Y;
};
#endif  // MODULE_H
