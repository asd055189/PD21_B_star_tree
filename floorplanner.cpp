#include "bits/stdc++.h"
#include "floorplanner.h"
using namespace std;


Floorplanner::Floorplanner(fstream& input_blk,fstream& input_net,double alpha){

    Alpha = alpha;
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
    initial_B_star_tree();//init

    DFS(tree_array);//init pack

   SA();
}

void Floorplanner::initial_B_star_tree() {
    for (int i = 0; i < block_list.size();i++) {
        if (block_list[i].getHeight() > block_list[i].getWidth())
            block_list[i]._rotate();
        sorted_list.push_back(&block_list[i]);
    }
    //sort width & height
    sort(sorted_list.begin(), sorted_list.end(),[](Block *a , Block *b){
        if(a->getWidth()!=b->getWidth())
            return a->getWidth() > b->getWidth();
        return a->getHeight()>b->getHeight();});
    tree_array = new Node;
    node_in_tree.push_back(tree_array);
    Node* root = tree_array;
    bool *put=new bool[sorted_list.size()]();
    int i = 0;
    int h = 0;
    bool tag=true;
   while (1) {
       tag = true;
        Node* p = root;
        while (1) {
            int width;
            for (int i=0;i< sorted_list.size();i++){
                if (put[i] == false) {
                    put[i] = true;
                    p->block = sorted_list[i];
                    h += sorted_list[i]->getHeight();
                    width = sorted_list[i]->getWidth();
                    break;
                }
            }
            for (int i = 0; i < sorted_list.size(); i++){
                if (put[i] == false) {
                    if (sorted_list[i]->getWidth()<=width&&h + sorted_list[i]->getHeight() < getbound_height()) {
                        h += sorted_list[i]->getHeight();
                        p->right = new Node;
                        node_in_tree.push_back(p->right);
                        p->right->block = sorted_list[i];
                        p->right->parent = p;
                        p = p->right;
                        put[i] = true;
                    }
                }
            }  
            break;
         }
        for (int j = 0; j < sorted_list.size(); j++){
            if (put[j] == false)
                tag = false;
        }
        if (tag)
            break;
        h = 0;
        root->left = new Node;
        node_in_tree.push_back(root->left);
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
   // if (node != nullptr)
       //cout << node->block->getName() << "   "<<node <<" "<<node->parent <<" " << node->left << " " << node->right << endl;
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

void Floorplanner::output(fstream& out,double runtime) {
    plot();
    int Wire = calcW();
    int W,H;
    int A = calcA(W,H);

    cout << "cost  factor : " << Alpha<<endl;
    cout << "cost : " << int(Alpha * A + (1 - Alpha) * Wire) <<endl;
    out << int(Alpha * A + (1 - Alpha) * Wire) << endl;
    cout << "Wire : " << Wire << endl;
    out << Wire << endl;
    cout << "area  : " << A << endl;
    out  << A << endl;
    cout << "width : " << W << " height : " << H<<endl;
    out << W << " " << H << endl;
    cout << "runtime : " << runtime << endl;
    out << runtime << endl;
    for (auto node : block_list) {
        cout << node.getName() << " " << node.getX1() << " " << node.getY1() << " " << node.getX2() << " " << node.getY2() << " " << endl;
        out << node.getName() << " " << node.getX1() << " " << node.getY1() << " " << node.getX2() << " " << node.getY2() << " " << endl;
    }
}

void Floorplanner::plot() {
 //gnuplot preset
    fstream outgraph("output.gp", ios::out);
    outgraph << "reset\n";
    outgraph << "set tics\n";
    outgraph << "unset key\n";
    Line* p = line;
    int X, Y=-1;
    while (p->next != nullptr) {
        if (p->Y > Y)
            Y = p->Y; 
        p = p->next;
    }
    X = p->x2;
    int boundWidth = X;// user-define value (boundary info)
    int boundHeight = Y;// same above
    outgraph << "set title \"The result of Floorplan"<<"[x:"<<X << "," << getbound_width() <<"][y:"<< Y << "," << getbound_height()<<"]\"\n";
    int index = 1;
    // wirte block info into output.gp
    for (auto b : sorted_list)// for block
    {
        string NodeName = b->getName();
        int x0 = b->getX1();
        int y0 =  b->getY1();
        int x1 = b->getX2();
        int y1 = b->getY2();
        int midX = (x0 + x1) / 2;
        int midY = (y0 + y1) / 2;
        Line* P=line->next;
        while (P) {
            outgraph << "set arrow from " << P->x1 << "," << P->Y << " to " << P->x2 << "," << P->Y;
            outgraph << " nohead  lc rgb \"red\" front\n";
            P = P->next;
        }
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
    system("gnuplot output.gp");
    outgraph.close();
}

void Floorplanner::SA() {
    double T = 0;//init temperature
    int r = 0;//# of iterations
    double avgA, avgW;
    int w, h;

    srand(0); //set seed

    Node *oldtree = copytree(tree_array,nullptr);
    Node* bestans = copytree(tree_array, nullptr);
    long long int A = 0, W = 0;
    for (int i = 0; i < 100; i++) {
        A += calcA(w,h);
        W += calcW();
        randomop();
    }
    tree_array = copytree(oldtree, nullptr);
    node_in_tree.clear();
    clonearray(tree_array);
    DFS(tree_array); 
    avgA = A / 100;
    avgW = W / 100;

    A = calcA(w,h);
    W = calcW();
    double cost = (1 - Alpha) * (A / avgA) + Alpha * (W / avgW);
    while (r < 1000) {
        vector<bool> rotatearr;
        for (auto node : node_in_tree) {
            rotatearr.push_back(node->block->getrotate());
        }
        int i = 0;
        randomop();
        DFS(tree_array);
        W = calcW();
        A = calcA(w, h);
        double newcost = (1 - Alpha) *(A/ avgA) + Alpha *(W/avgW);
        double diff = newcost - cost;
        if (diff > 0) {//up move
            cout << "===============up move\n";
            tree_array = copytree(oldtree, nullptr);
            for (int i = 0; i < node_in_tree.size(); i++) {
                if (node_in_tree[i]->block->getrotate() != rotatearr[i]) {
                    node_in_tree[i]->block->_rotate();
                }
            }
            node_in_tree.clear();
            clonearray(tree_array);
            DFS(tree_array);
        }
        if (diff <= 0) {//down move
            cout << "===============================down move\n";
            oldtree = copytree(tree_array, nullptr);
            node_in_tree.clear();
            clonearray(tree_array);
            DFS(tree_array);
            if (w <= getbound_width() && h <= getbound_height()) {
                cout << "better sol found!\n";
                DFS(tree_array);
                calcA(w,h);
                break;
            }
        }
        r++;
    }

    /*
    tree_array = copytree(bestans, nullptr);
    for (int i = 0; i < node_in_tree.size(); i++) {
        if (node_in_tree[i]->block->getrotate()!=rotatearr[i]) {
            node_in_tree[i]->block->_rotate();
        }
    }
    node_in_tree.clear();
    clonearray(tree_array);
    DFS(tree_array);
*/


}

int Floorplanner::calcA(int &W,int &H) {
    Line* p = line->next;
     W= H = -1;
    while (p) {
        if (p->Y > H)
            H = p->Y;
        if (p->x2 > W)
            W = p->x2;
        p = p->next;
    }
    return W*H;
}

int Floorplanner::calcW() {
    int W = 0;
    for (auto node : net) 
        W += node.calcHPWL();
    return W;
}

void Floorplanner::randomop() {
    
    int op =rand()%3+1;//op from 1 to 3

    int r;
    int d, i;
    int b;
    Node* _d, *_i;
 
    switch (op) {
        case 1:
            r= rand() % block_list.size();
            node_in_tree[r]->block->_rotate();
            //cout << "rotate : " << node_in_tree[r]->block->getName()<<endl;
            break;
        case 2:
            while (1) {
                d = rand() % block_list.size() ;
                i = rand() % block_list.size() ;
                if (i != d&&node_in_tree[d]->left == nullptr && node_in_tree[d]->right == nullptr )
                    break;
            }
            _d = node_in_tree[d];
            _i = node_in_tree[i];
            //cout << "delete from : " << _d->block->getName() << "   insert to  : " << _i->block->getName()<<endl;
            if (_d->parent != nullptr) {
                if (_d->parent->left == _d)
                    _d->parent->left = _d->left;
                if (_d->parent->right == _d)
                    _d->parent->right = _d->right;
            }
            _d->right=_d->left = nullptr;
            _d->parent = _i;
            b = rand() % 2;

            if (b == 0) {//i->left
                _d->left = _i->left;
                if(_i->left!=nullptr)
                     _i->left->parent = _d;
                _i->left = _d;
            }
            if (b == 1) {//i->right
                _d->right = _i->right;               
                if (_i->right!= nullptr)
                    _i->right->parent = _d;
                _i->right = _d;
            }
            break;
        case 3:
            while (1) {
                d = rand() % block_list.size();
                i = rand() % block_list.size();
                if (i != d )
                    break;
            }
            Node* sw1 = node_in_tree[d];
            Node* sw2 = node_in_tree[i];
            //cout << "swap : " << sw1->block->getName() << " ,  " << sw2->block->getName()<<endl;
            if (sw1->block->getName() == "cc_12" && sw2->block->getName() == "clk")
                int a = 0;
            if (sw1->parent == sw2->parent) {
                if (sw2->parent->left == sw2) {
                    sw2->parent->right = sw2;
                    sw2->parent->left = sw1;
                }
                if (sw2->parent->right == sw2) {
                    sw2->parent->right = sw1;
                    sw2->parent->left = sw2;
                }
            }
            else {
                if (sw2->parent != nullptr) {
                    if (sw2->parent->left == sw2)
                        sw2->parent->left = sw1;
                    if (sw2->parent->right == sw2)
                        sw2->parent->right = sw1;
                }
                else {
                    tree_array = sw1;
                }
                if (sw1->parent != nullptr) {
                    if (sw1->parent->left == sw1)
                        sw1->parent->left = sw2;
                    if (sw1->parent->right == sw1)
                        sw1->parent->right = sw2;
                }
                else {
                    tree_array = sw2;
                }
            }
            Node* par = sw1->parent;
            sw1->parent = sw2->parent;
            sw2->parent = par;
            Node* l=sw1->left, * r=sw1->right;
            sw1->left = sw2->left;
            sw1->right = sw2->right;
            if(sw1->left!=nullptr)
                sw1->left->parent = sw1;
            if (sw1->right != nullptr)
                sw1->right->parent = sw1;
            sw2->left = l;
            sw2->right = r;
            if (sw2->left != nullptr)
                sw2->left->parent = sw2;
            if (sw2->right != nullptr)
                sw2->right->parent = sw2;
            break;
     }
}

Node* Floorplanner::copytree(Node* head,Node * parent)
{
    if (head == nullptr)
        return nullptr;
    Node* tmp = new Node;
    tmp->block = head->block;
    tmp->parent =parent;
    tmp->left = copytree(head->left,tmp);
    tmp->right = copytree(head->right,tmp);
    return tmp;
}


void  Floorplanner::clonearray(Node* head) {
    if (head == nullptr)
        return;
    node_in_tree.push_back(head);
      clonearray(head->left);
      clonearray(head->right);
}