#ifndef FLOORPLANNER_H
#define FLOORPLANNER_H
#include "bits/stdc++.h"
#include <unordered_map>
#include "module.h"
using namespace std;

class Floorplanner{
    public:
        Floorplanner(fstream& input_blk,fstream& input_net);
        ~Floorplanner() { }
        void floorplan();
        void initial_B_star_tree();
        int getbound_width(){return bound_width;}
        int getbound_height(){return bound_height;}
        int getnum_block(){return num_block;}
        int getnum_terminal(){return num_terminal;}
        void packing();
        void contourline(Node* node,int X1);
        void DFS(Node *node);
        void output();
        void plot();
    private:
        int bound_width;
        int bound_height;
        int num_block;
        int num_terminal;
        int num_net;
        vector<Net> net;
        unordered_map<string,int>block_list_map;
        unordered_map<string,int>terminal_list_map;
        vector<Block> block_list;
        vector<Terminal> terminal_list;
        Node* tree_array;
        vector<Block> sorted_list;
        Line* line;
};


#endif