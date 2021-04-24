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
        /*for (auto a : net) {
            for (auto b : a.getTermList()) {
                cout << b->getName()<<endl;
            }
            cout << endl;
        }*/
    }
}
void Floorplanner::floorplan(){
    initial_B_star_tree();
    DFS(tree_array);
    output();
}

void Floorplanner::initial_B_star_tree() {

    sorted_list=block_list;

    //sort width & height
    sort(sorted_list.begin(), sorted_list.end(),[](Block a , Block b){
        if(a.getWidth()!=b.getWidth())
            return a.getWidth() > b.getWidth();
        return a.getHeight()>b.getHeight();});
    tree_array = new Node;
    Node* root = tree_array;

    int i = 0;
    int h = 0;
    while (i != sorted_list.size()) {
        Node* p = root;
        while (i!=sorted_list.size()) {
            

            p->block = &sorted_list[i];
            h += sorted_list[i].getHeight();
            i++;
            if (i == sorted_list.size() )
                break;
            if (h+ sorted_list[i].getHeight() < getbound_height()) {
                p->right = new Node;
                p->right->parent = p;
                p = p->right;
            }
            else {
                break;
            }

        }
        if (i == sorted_list.size())
            break;
        h = 0;
        root->left = new Node;
        root = root->left;
    }
}

void Floorplanner :: DFS(Node* node) {
    if (node->left != nullptr) {
        DFS(node->left);
        cout << "\n=====\n";
    }
    if (node->right != nullptr) {
        DFS(node->right);
    }
    cout << node->block->getName() << " " << node->block->getWidth() <<" "<<node->block->getHeight()<<endl;
}
void Floorplanner::output() {

}