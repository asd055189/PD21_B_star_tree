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
        root->left->parent = root;
        root = root->left;
    }
}

void Floorplanner::packing()
{

}

void Floorplanner::contourline(Node* node, int X1)
{
    int maxY=0;
    Line *pre = nullptr;
    Line* cur=line;
    while (cur !=nullptr) {
        if (cur->x1 < X1 &&X1< cur->x2 ) {//case : x1-X1 survive
            if (cur->Y > maxY)
                maxY = cur->Y;
            Line* tmp=new Line;
            tmp->x1= cur->x1;
            tmp->x2 = X1;
            tmp->Y = cur->Y;
            tmp->next = cur->next;
            if (pre != nullptr)
                pre->next = tmp;
            else
                line = tmp;
            Line* t = cur;
            delete t;
            cur = tmp;
        }
        if (cur->x1 < X1 + node->block->getWidth() && X1 + node->block->getWidth() < cur->x2) {//case : X2-x2 survive
            if (cur->Y > maxY)
                maxY = cur->Y;
            Line* tmp = new Line;
            tmp->x1 = X1 + node->block->getWidth();
            tmp->x2 = cur->x2;
            tmp->Y = cur->Y;
            tmp->next = cur->next;
            pre->next = tmp;
            Line* t = cur;
            delete t;
            cur = tmp;
        }
        if (X1 <= cur->x1 && cur->x1 <= X1 + node->block->getWidth() &&
            X1 <= cur->x2 && cur->x2 <= X1 + node->block->getWidth()) {//case: all x1-x2 remove 
            if (cur->Y > maxY)
                maxY = cur->Y;
            Line* t = cur;
            if(pre!=nullptr)
                pre->next = cur->next;
            cur = pre;
            delete t;
        }
        if (X1 + node->block->getWidth() <= cur->x1)
            break;
        pre = cur;
        cur = cur->next;
     }
    Line* tmp=new Line;
    tmp->Y = maxY+node->block->getHeight();
    tmp->x1 = X1;
    tmp->x2 = X1 + node->block->getWidth();
    node->block->setPos(tmp->x1, tmp->Y- node->block->getHeight(), tmp->x2, tmp->Y);

    cur = line;
    while (cur != nullptr) {
        if (cur->x2==tmp->x1){
            tmp->next = cur->next;
            cur->next = tmp;
        }
        pre = cur;
        cur = cur->next;
    }
}

void Floorplanner :: DFS(Node* node) {
    if (node->parent == nullptr) {
        line = new Line;
        line->x1 = -1;
        line->x2 = 0;
        line->Y = 0;

        Line *root = new Line;
        root->x1 = 0;
        root->x2 = node->block->getWidth();
        root->Y = node->block->getHeight();
        line->next = root;
        node->block->setPos(0, 0, node->block->getWidth(), node->block->getHeight());
    }
    //else
        //node->block->setPos(node->parent->block.)
    if (node->left != nullptr) {
        int X = node->block->getX2();
        contourline(node->left,X);
        DFS(node->left);
        
    }
    if (node->right != nullptr) {
        int X = node->block->getX1();
        contourline(node->right,X);
        DFS(node->right);
    }
}
void Floorplanner::output() {
    plot();
}
void Floorplanner::plot() {
    /////////////info. to show for gnu/////////////
    int boundWidth =2*getbound_width();// user-define value (boundary info)
    int boundHeight = 2*getbound_height();// same above
 /////////////////////////////////////////////
 //gnuplot preset
    fstream outgraph("output.gp", ios::out);
    outgraph << "reset\n";
    outgraph << "set tics\n";
    outgraph << "unset key\n";
    outgraph << "set title \"The result of Floorplan\"\n";
    int index = 1;
    // wirte block info into output.gp
    for (auto b : sorted_list)// for block
    {
        string NodeName = b.getName();
        int x0 = b.getX1();
        int y0 =  b.getY1();
        int x1 = b.getX2();
        int y1 = b.getY2();
        int midX = (x0 + x1) / 2;
        int midY = (y0 + y1) / 2;

        outgraph << "set object " << index << " rect from "
            << x0 << "," << y0 << " to " << x1 << "," << y1 << " fs empty\n"
            << "set label " << "\"" << NodeName << "\"" << " at " << midX << "," << midY << " center\n";
        index++;
    }
    // write script to output.gp and execute by system call
    outgraph << "set object " << index << " rect from "
        << "0" << "," << "0" << " to " << boundWidth << "," << boundHeight << " fs empty border 3 \n";
    //<< "set label " << "\"" << "Outline" << "\"" << " at " << boundWidth/2+50 << "," << boundHeight+100 << "right\n";

    outgraph << "set style line 1 lc rgb \"red\" lw 3\n";
    outgraph << "set border ls 1\n";
    outgraph << "set terminal png\n";
    outgraph << "set output \"graph.png\"\n";
    outgraph << "plot [0:" << boundWidth << "][0:" << boundHeight << "] 0\n";
    outgraph << "set terminal x11 persist\n";
    outgraph << "replot\n";
    outgraph << "exit";
    outgraph.close();
}