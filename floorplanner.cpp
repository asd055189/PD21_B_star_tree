#include "bits/stdc++.h"
#include "floorplanner.h"
using namespace std;


Floorplanner::Floorplanner(fstream& input_blk,fstream& input_net){
    string str;
    input_blk>>str;
    if(str=="Outline:"){
        string a,b;
        input_blk>>a;
        input_blk>>b;
        bound_width=atoi(a.c_str());
        bound_height=atoi(b.c_str());
    }
    input_blk>>str;
    if(str=="NumBlocks:"){
        input_blk>>str;
        num_block=atoi(str.c_str());
    }
    input_blk>>str;
    if(str=="NumTerminals:"){
        input_blk>>str;
        num_terminal=atoi(str.c_str());
    }
    for (int i=0;i<getnum_block();i++){
        input_blk>>str;
        string name=str;

        int w,h;
        input_blk>>str;
        w=atoi(str.c_str());
        input_blk>>str;
        h=atoi(str.c_str());

        Block tmp(name,w,h);
        block_list.push_back(tmp);

        block_list_map.insert(pair<string,int>(name,i));
        //cout <<tmp.getName() <<endl<<tmp.getWidth() <<endl<<tmp.getHeight() <<endl;
    }
    for(int i=0;i<getnum_terminal();i++){
        input_blk>>str;
        string name=str;
        input_blk>>str;
        int w,h;
        input_blk>>str;
        w=atoi(str.c_str());
        input_blk>>str;
        h=atoi(str.c_str());

        Terminal tmp(name,w,h);
        terminal_list.push_back(tmp);

        terminal_list_map.insert(pair<string,int>(name,i));
        //cout <<name<<endl;
    }
    input_net >> str;
    if (str=="NumNets:")
    {
        input_net >> str;
        num_net=atoi(str.c_str());
    }
    for (int i=0;i<num_net;i++){
        input_net>>str;
        if(str=="NetDegree:"){
            input_net>>str;
            int N=atoi(str.c_str());
            Net tmp;
            for (int j=0;j<N;j++){
                input_net>>str;
                //cout <<str;
                unordered_map<string,int>::iterator it1=terminal_list_map.find(str);
                unordered_map<string,int>::iterator it2=block_list_map.find(str);
                if(it1!=terminal_list_map.end())
                    tmp.addTerm(&terminal_list[it1->second]);
                else if(it2!=block_list_map.end())
                {
                    tmp.addTerm(&block_list[it2->second]);
                }
            }
            net.push_back(tmp);
        }
    }
    for (int i=0;i<net[0].getTermList().size();i++)
    {
        //cout <<((Block*)net[0].getTermList()[i])->getName()<<endl;
    }
}
void Floorplanner::floorplan(){

}