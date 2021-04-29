#include "bits/stdc++.h"
#include "floorplanner.h"
using namespace std;


Floorplanner::Floorplanner(fstream& input_blk, fstream& input_net, double alpha, time_t s) {
	start = s;
	Alpha = alpha;

	string str;

	input_blk >> str;
	if (str == "Outline:") {
		string a, b;
		input_blk >> a;
		input_blk >> b;
		bound_width = atoi(a.c_str());
		bound_height = atoi(b.c_str());
	}

	input_blk >> str;
	if (str == "NumBlocks:") {
		input_blk >> str;
		num_block = atoi(str.c_str());
	}

	input_blk >> str;
	if (str == "NumTerminals:") {
		input_blk >> str;
		num_terminal = atoi(str.c_str());
	}
	for (int i = 0; i < getnum_block(); i++) {
		input_blk >> str;
		string name = str;

		int w, h;
		input_blk >> str;
		w = atoi(str.c_str());
		input_blk >> str;
		h = atoi(str.c_str());

		Block tmp(name, w, h);
		block_list.push_back(tmp);

	}

	for (int i = 0; i < getnum_terminal(); i++) {
		input_blk >> str;
		string name = str;
		input_blk >> str;
		int w, h;
		input_blk >> str;
		w = atoi(str.c_str());
		input_blk >> str;
		h = atoi(str.c_str());

		Terminal tmp(name, w, h);
		terminal_list.push_back(tmp);

	}

	input_net >> str;
	if (str == "NumNets:")
	{
		input_net >> str;
		num_net = atoi(str.c_str());
	}
	for (int i = 0; i < num_net; i++) {
		input_net >> str;
		if (str == "NetDegree:") {
			input_net >> str;
			int N = atoi(str.c_str());
			Net tmp;
			input_net >> str;
			net.push_back(tmp);
		}
	}
}

void Floorplanner::floorplan() {
	initial_B_star_tree();//init tree
	DFS(tree_array);//packing
	SA();
}

void Floorplanner::initial_B_star_tree() {
	if (getbound_height() * 1.4 < getbound_width()) {
		for (int i = 0; i < block_list.size(); i++) {
			if (block_list[i].getHeight() > block_list[i].getWidth())
				block_list[i]._rotate();
			sorted_list.push_back(&block_list[i]);
		}
	}
	else if (getbound_width() * 1.4 < getbound_height()) {
		for (int i = 0; i < block_list.size(); i++) {
			if (block_list[i].getHeight() < block_list[i].getWidth())
				block_list[i]._rotate();
			sorted_list.push_back(&block_list[i]);
		}
	}
	else {
		for (int i = 0; i < block_list.size(); i++) {
			if (block_list[i].getHeight() > block_list[i].getWidth())
				block_list[i]._rotate();
			sorted_list.push_back(&block_list[i]);
		}
	}

	//sort width & height
	sort(sorted_list.begin(), sorted_list.end(), [](Block* a, Block* b) {
		if (a->getWidth() != b->getWidth())
			return a->getWidth() > b->getWidth();
		return a->getHeight() > b->getHeight(); });

	tree_array = new Node;
	Node* root = tree_array;

	bool* put = new bool[sorted_list.size()]();

	int i = 0;		//index of block already inserted in the tree
	int h = 0;	//height for each column

	bool tag = true;
	while (1) {
		tag = true;
		Node* p = root;
		while (1) {
			int width;//the most large width in the column (the base block)
			for (int i = 0; i < sorted_list.size(); i++) {//put the base block
				if (put[i] == false) {
					put[i] = true;
					p->block = sorted_list[i];
					h += sorted_list[i]->getHeight();
					width = sorted_list[i]->getWidth();
					break;
				}
			}

			for (int i = 0; i < sorted_list.size(); i++) {//put block at the column untill Y > boundary_high
				if (put[i] == false) {
					if (sorted_list[i]->getWidth() <= width && h + sorted_list[i]->getHeight() < getbound_height()) {
						h += sorted_list[i]->getHeight();
						p->right = new Node;
						p->right->block = sorted_list[i];
						p->right->parent = p;
						p = p->right;
						put[i] = true;
					}
				}
			}
			break;
		}

		for (int j = 0; j < sorted_list.size(); j++) {
			if (put[j] == false)
				tag = false;
		}
		if (tag)//whether the block all inserted to the b* tree
			break;
		h = 0;
		root->left = new Node;
		root->left->parent = root;
		root = root->left;
	}

	//check boundary
	DFS(tree_array);//init pack

	int tmpw, tmph;
	calcA(tmpw, tmph);
	while (tmpw > getbound_width()) {
		Node* p = tree_array;
		while (p->left) { //According to the way I put, Y will be fine, so check the most left child (X) in the tree
			p = p->left;
		}
		Line* l = line;
		int x1, x2;
		while (line) {//find the area in order to insert  block p via contour line
			if (line->x2 - line->x1 >= p->block->getWidth() && line->Y + p->block->getHeight() < getbound_height()) {
				x1 = line->x1;
				x2 = line->x2;
				break;
			}
			line = line->next;
		}
		if (line) {
			Node* base = tree_array;
			p->parent->left = p->right;
			if(p->right!=nullptr)
				p->right->parent = p->parent;
			p->left = p->right = p->parent = nullptr;
			while (!(base->block->getX1() <= x1 && x2 <= base->block->getX2())) {
				base = base->left;
			}
			Node* pos = base;
			while (pos) {
				if (pos->block->getWidth() + p->block->getWidth() < base->block->getX2()) {//insert to left child
					while (pos->left) {
						pos = pos->left;
					}
					pos->left = p;
					p->parent = pos;
					DFS(tree_array);
					break;
				}
				pos = pos->right;
			}
			if(!pos)
				break;
		}
		else {
			break;
		}
	}
	clonearray(tree_array);//update array
}

void Floorplanner::contourline(Node* node, int X1)
{
	int maxY = 0;
	Line* pre = nullptr;
	Line* cur = line;
	while (cur != nullptr) {
		if (cur->x1 < X1 && X1 < cur->x2) {//case : x1-X1 survive
			if (cur->Y > maxY)
				maxY = cur->Y;
			Line* tmp = new Line;
			tmp->x1 = cur->x1;
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
			if (pre != nullptr)
				pre->next = cur->next;
			cur = pre;
			delete t;
		}
		if (X1 + node->block->getWidth() <= cur->x1)
			break;
		pre = cur;
		cur = cur->next;
	}
	Line* tmp = new Line;
	tmp->Y = maxY + node->block->getHeight();
	tmp->x1 = X1;
	tmp->x2 = X1 + node->block->getWidth();
	node->block->setPos(tmp->x1, tmp->Y - node->block->getHeight(), tmp->x2, tmp->Y);

	cur = line;
	while (cur != nullptr) {
		if (cur->x2 == tmp->x1) {
			tmp->next = cur->next;
			cur->next = tmp;
		}
		pre = cur;
		cur = cur->next;
	}
}

void Floorplanner::DFS(Node* node) {
	if (node->parent == nullptr) {
		line = new Line;
		line->x1 = -1;
		line->x2 = 0;
		line->Y = 0;

		Line* root = new Line;
		root->x1 = 0;
		root->x2 = node->block->getWidth();
		root->Y = node->block->getHeight();
		line->next = root;
		node->block->setPos(0, 0, node->block->getWidth(), node->block->getHeight());
	}

	if (node->left != nullptr) {
		int X = node->block->getX2();
		contourline(node->left, X);
		DFS(node->left);

	}

	if (node->right != nullptr) {
		int X = node->block->getX1();
		contourline(node->right, X);
		DFS(node->right);
	}
}

void Floorplanner::output(fstream& out, double runtime) {
	plot();
	int Wire = calcW();
	int W, H;
	int A = calcA(W, H);

	//cout << "cost  factor : " << Alpha<<endl;
	//cout << "cost : " << int(Alpha * A + (1 - Alpha) * Wire) <<endl;
	//cout << "Wire : " << Wire << endl;
	//cout << "area  : " << A << endl;
	//cout << "width : " << W << " height : " << H<<endl;
	//cout << "runtime : " << runtime << endl;
	// cout << node.getName() << " " << node.getX1() << " " << node.getY1() << " " << node.getX2() << " " << node.getY2() << " " << endl;

	out << int(Alpha * A + (1 - Alpha) * Wire) << endl;
	out << Wire << endl;
	out << A << endl;
	out << W << " " << H << endl;
	out << runtime << endl;
	for (auto node : block_list) 
		out << node.getName() << " " << node.getX1() << " " << node.getY1() << " " << node.getX2() << " " << node.getY2() << " " << endl;
}

void Floorplanner::SA() {

    double avgA, avgW,avgU;
    int w, h;
    int seed = 1619644695;
    cout <<"seed : "<< seed <<endl;
    srand(seed); //set seed

    Node *oldtree = copytree(tree_array,nullptr);
    Node* bestans = copytree(tree_array, nullptr);
    long long int A = 0, W = 0,U=0;
    vector<bool> initrotatearr;
    vector<bool> bestrotatearr;
    for (auto node : node_in_tree) {
        initrotatearr.push_back(node->block->getrotate());
        bestrotatearr.push_back(node->block->getrotate());
    }
    double c= (1 - Alpha) * (A)+Alpha * (W);

    for (int i = 0; i < 100; i++) {
        A += calcA(w,h);
        W += calcW();
        randomop();
        double ncost = (1 - Alpha) * (A ) + Alpha * (W );
        if (ncost > c) {
            U += ncost;
        }
    }

    //back to init
    tree_array = copytree(oldtree, nullptr);
    node_in_tree.clear();
    clonearray(tree_array);
    for (int i = 0; i < node_in_tree.size(); i++) {
        if (node_in_tree[i]->block->getrotate() != initrotatearr[i]) {
            node_in_tree[i]->block->_rotate();
        }
    }
   //node_in_tree.clear();
    clonearray(tree_array);
    DFS(tree_array);
    //back to init
	plot();
    cout << "\n\nstart here\n\n";
    avgA = A / 100;
    avgW = W / 100;
    avgU = -U / 100;
    A = calcA(w,h);
    W = calcW();
    double p = 0.9;//init p
    double cost = (1 - Alpha) * (A / avgA) + Alpha * (W / avgW);
    double bestcost = cost;
    double temperature = avgU / log(p);

    int _c = 100, _k = 7;
    double avgtemp=0;
    int r = 1;//# of iterations
    double T = temperature;
    time_t end = time(NULL);
    while (difftime(end, start) <= 8) {
        Node* oldtree = copytree(tree_array, nullptr);
        vector<bool> rotatearr;
        for (auto node : node_in_tree) {
            rotatearr.push_back(node->block->getrotate());
        }
        int i = 0;
        randomop();

        DFS(tree_array);
        W = calcW();
        A = calcA(w, h);
        double newcost = (1 - Alpha) * (A / avgA) + Alpha * (W / avgW);
        double diff = newcost - cost;
        avgtemp = ((r - 1) * avgtemp + diff) / r;
        if (diff > 0) {//up move

            //cout << "up\n";
            double dice = (double)rand() / (RAND_MAX + 1.0);

            if (dice > p) {
                //reject change
                tree_array = copytree(oldtree, nullptr);
                node_in_tree.clear();
                clonearray(tree_array);
                for (int i = 0; i < node_in_tree.size(); i++) {
                    if (node_in_tree[i]->block->getrotate() != rotatearr[i]) {
                        node_in_tree[i]->block->_rotate();
                    }
                }

                DFS(tree_array);
            }
            else {//accept
                cost = newcost;
            }

        }
        if (diff <= 0) {//down move
            //cout << "down\n";
            cost = newcost;
            if (w < getbound_width() && h < getbound_height() && cost < bestcost) {
               //plot();
                bestcost = cost;
                bestans = copytree(tree_array, nullptr);

                //printtree(bestans);
                //cout << "\n==========================\n";
                //printtree(tree_array);

                //cout << "\n\n===best solution changed!===" << w << "  " << h << " " << bestcost << "\n\n";
                bestrotatearr.clear();
                node_in_tree.clear();
                clonearray(tree_array);
                for (auto node : node_in_tree) {
                    bestrotatearr.push_back(node->block->getrotate());
                    //cout << node->block->getrotate() << endl;
                }
                // bestans = copytree(tree_array, nullptr);
                 //break;
            }
            rotatearr.clear();
            for (auto node : node_in_tree) {
                rotatearr.push_back(node->block->getrotate());
            }

        }
        if (2 <= r && r <= _k)
            T = (temperature * avgtemp) / (r * _c);
        else if (_k < r)
            T = (temperature * avgtemp) / r;
        r++;
        end = time(NULL);

    }
    tree_array = copytree(bestans, nullptr);
    node_in_tree.clear();
    clonearray(tree_array);
    for (int i = 0; i < bestrotatearr.size(); i++) {
        if (node_in_tree[i]->block->getrotate() != bestrotatearr[i]) {
            node_in_tree[i]->block->_rotate();
        }
    }
    node_in_tree.clear();
    clonearray(tree_array);
    DFS(tree_array);
    plot();
}

int Floorplanner::calcA(int& W, int& H) {
	Line* p = line->next;
	W = H = -1;
	while (p) {
		if (p->Y > H)
			H = p->Y;
		if (p->x2 > W)
			W = p->x2;
		p = p->next;
	}
	return W * H;
}

int Floorplanner::calcW() {
	int W = 0;
	for (auto node : net)
		W += node.calcHPWL();
	return W;
}

void Floorplanner::randomop() {

	int op = rand() % 3 + 1;//op from 1 to 3

	int r;
	int d, i;
	int b;
	Node* _d, * _i;

	switch (op) {
	case 1://rotate
		r = rand() % block_list.size();
		node_in_tree[r]->block->_rotate();
		break;
	case 2://delete and insert
		while (1) {
			d = rand() % block_list.size();
			i = rand() % block_list.size();
			if (i != d && node_in_tree[d]->left == nullptr && node_in_tree[d]->right == nullptr)//only choose the leaf node to make the process easy to implement
				break;
			
		}
		_d = node_in_tree[d];
		_i = node_in_tree[i];
		if (_d->parent != nullptr) {
			if (_d->parent->left == _d)
				_d->parent->left = _d->left;
			if (_d->parent->right == _d)
				_d->parent->right = _d->right;
		}
		_d->right = _d->left = nullptr;
		_d->parent = _i;
		b = rand() % 2;

		if (b == 0) {//i->left
			_d->left = _i->left;
			if (_i->left != nullptr)
				_i->left->parent = _d;
			_i->left = _d;
		}
		if (b == 1) {//i->right
			_d->right = _i->right;
			if (_i->right != nullptr)
				_i->right->parent = _d;
			_i->right = _d;
		}
		break;
	case 3://swap
		while (1) {
			d = rand() % block_list.size();
			i = rand() % block_list.size();
			if (i != d)
				break;
		}
		Node* sw1 = node_in_tree[d];
		Node* sw2 = node_in_tree[i];
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
		Node* l = sw1->left, * r = sw1->right;
		sw1->left = sw2->left;
		sw1->right = sw2->right;
		if (sw1->left != nullptr)
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

Node* Floorplanner::copytree(Node* head, Node* parent)
{
	if (head == nullptr)
		return nullptr;
	Node* tmp = new Node;
	tmp->block = head->block;
	tmp->parent = parent;
	tmp->left = copytree(head->left, tmp);
	tmp->right = copytree(head->right, tmp);
	return tmp;
}

void  Floorplanner::clonearray(Node* head) {
	if (head == nullptr)
		return;
	node_in_tree.push_back(head);

	clonearray(head->right);
	clonearray(head->left);
}

void Floorplanner::printtree(Node* head) {//for debug reason
	if (head == nullptr)
		return;
	cout << head->block->getName() << " ";


	printtree(head->right);
	cout << "\n====\n";
	printtree(head->left);
}

void Floorplanner::plot() {//draw the placement via gnuplot, code from NTUST LAB
	fstream outgraph("output.gp", ios::out);
	outgraph << "reset\n";
	outgraph << "set tics\n";
	outgraph << "unset key\n";
	Line* p = line;
	int X = -1, Y = -1;
	while (p) {
		if (p->Y > Y)
			Y = p->Y;
		if (p->x2 > X)
			X = p->x2;
		p = p->next;
	}
	outgraph << "set title \"The result of Floorplan" << "[x:" << X << "," << getbound_width() << "][y:" << Y << "," << getbound_height() << "]\"\n";
	int index = 1;
	// wirte block info into output.gp
	for (auto b : sorted_list)// for block
	{
		string NodeName = b->getName();
		int x0 = b->getX1();
		int y0 = b->getY1();
		int x1 = b->getX2();
		int y1 = b->getY2();
		int midX = (x0 + x1) / 2;
		int midY = (y0 + y1) / 2;
		Line* P = line->next;
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
		<< "0" << "," << "0" << " to " << X << "," << Y << " fs empty border 3 \n";
	//<< "set label " << "\"" << "Outline" << "\"" << " at " << boundWidth/2+50 << "," << boundHeight+100 << "right\n";

	outgraph << "set style line 1 lc rgb \"red\" lw 3\n";
	outgraph << "set border ls 1\n";
	outgraph << "set terminal png\n";
	outgraph << "set output \"graph.png\"\n";
	outgraph << "plot [0:" << X << "][0:" << Y << "] 0\n";
	outgraph << "set terminal x11 persist\n";
	outgraph << "replot\n";
	outgraph << "exit";
	//system("gnuplot output.gp");
	outgraph.close();
}