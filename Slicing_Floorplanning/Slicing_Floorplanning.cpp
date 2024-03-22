// PA3 Floorplan ,input:txt output:txt
// Program objective: minimize total floorplan area of soft module
// Date:2023/05/10
// Author:Guan-You Chen
#include <algorithm>
#include <iostream>
#include <string>
#include <queue>
#include <stack>
#include <fstream>
#include <sstream>
#include <cmath>
#include<utility>
#include<unordered_map>
#include<ctime>
#include<cstdlib>
#include <random>

using namespace std;


// class SlicingTree;
struct TreeNode{
    TreeNode *leftchild;
    TreeNode *rightchild;
    TreeNode *parent;
    string id;//1,2,3,...,H,V
    string childType; //Record whether the node is a left child('L') or a right child('R'), and use 'N' to represent the root
    vector<double> w,h; // Record all possible shape of soft module
    double area,final_h,final_w;//Record the final determined height and width
    int index;//Record node's index in polish expression
    unordered_map<double,string>tableAreaChild;//Key: area, value: left child & right child index in vector w and vector h

    TreeNode():leftchild(0),rightchild(0),parent(0),id(""),childType(""),area(0),final_h(0),final_w(0),index(0){};
    TreeNode(string i,string c,double a):leftchild(0),rightchild(0),parent(0),id(i),childType(c),area(a),final_h(0),final_w(0),index(0){};


};
class SlicingTree{
private:
    vector<TreeNode*> operandPos,operatorPos,numSeq;//operandPos: record operand's address,operatorPos: record operator's address
    vector<TreeNode*> best_operandPos,best_operatorPos,best_PE;//Record best operand,operator,polish expression node's address
    string bestPE;//Record minimal area of polish expression
    double total_h,total_w;
    double cost;
    int indexCnt;
public:
    TreeNode *root;
    SlicingTree():root(0),total_h(0),total_w(0),cost(0),indexCnt(0),bestPE(""){};
    SlicingTree(TreeNode *node):root(node),total_h(0),total_w(0),cost(0),indexCnt(0),bestPE(""){};

    vector<TreeNode*> allNodePos;//Record polish expression node's address

    void insertLeftNode(TreeNode *curPtr);
    void insertRightNode(TreeNode *curPtr,string id,double area);
    void buildTree(vector<string> tokens);
    void Postorder(TreeNode *current);
    void invert(TreeNode *targerNode);
    vector<TreeNode*> get_operatorPos();

    void M1(int,int,int); //Swap two adjacent operand
    void M2(TreeNode *targetPtr); //Invert chain
    void M3(TreeNode *curPtr,TreeNode *sucPtr,int index); //Swap two adjacent operator and operand
    bool M3_Check(TreeNode *curPtr,TreeNode *sucPtr,int index);//Check balloting property

    void Initial_H_W();//Decide module's shape curve
    void ShapeCombine(TreeNode *targerPtr);//Shape curve combine
    void BottomUp();
    void DecideModule(TreeNode *target);//Decide module's final width and height
    double TotalArea();//Calculate total area
    void Topdown();

    void WongLiu(int,clock_t);//Wong-Liu Algorithm
    void SelectMove(int,int);//Select one of three move (M1,M2,M3)
    void genBestSlicingTree();//Base on best polish expression to generate new slicing tree


    friend class Myfile;
};
void SlicingTree::insertLeftNode(TreeNode *curPtr){
    TreeNode *newNode = new TreeNode("V","L",0);//id:"V",childType:"L",area:0
    if(root == 0){
        root=newNode;
        newNode->childType = "N";
        operatorPos.push_back(newNode); //Store operator's address in vector
        bestPE=bestPE+newNode->id;
    }
    else{
        newNode->parent = curPtr;
        curPtr->leftchild = newNode;
        operatorPos.push_back(newNode);//Store operator's address in vector
        bestPE=bestPE+" "+newNode->id;
    }
}
void SlicingTree::insertRightNode(TreeNode *curPtr,string id,double area){
    TreeNode *newNode = new TreeNode(id,"R",area);//id:module's id,childType:"R",area:module's area

    newNode->parent = curPtr;
    curPtr->rightchild = newNode;
    operandPos.push_back(newNode);//Store operand's address in vector
}

void SlicingTree::buildTree(vector<string> tokens){
    int max_node_num = stoi(tokens[0])-1; //
    int times = max_node_num-1;
    insertLeftNode(root);//Insert first node
    TreeNode *curPtr=root;
    for(int i=1;i<=times;i++){
        insertLeftNode(curPtr);
        insertRightNode(curPtr,tokens[1+2*max_node_num],stod(tokens[2+2*max_node_num]));
        curPtr=curPtr->leftchild;
        max_node_num=max_node_num-1;
    }
    TreeNode *Node0 = new TreeNode( "0","L",stod(tokens[2]) );
    TreeNode *Node1 = new TreeNode( "1","R",stod(tokens[4]) );
    curPtr->leftchild = Node0;
    Node0->parent = curPtr;
    curPtr->rightchild = Node1;
    Node1->parent = curPtr;
    operandPos.push_back(Node1);
    operandPos.push_back(Node0);
    reverse(operandPos.begin(),operandPos.end());
    numSeq=operandPos;

    Postorder(root);
    best_operandPos=operandPos;
    best_operatorPos=operatorPos;
    best_PE=allNodePos;
    Initial_H_W();
    BottomUp();
    Topdown();
}

void SlicingTree::Postorder(TreeNode *current){
    if (current) {                          // if current != NULL
        Postorder(current->leftchild);      // L
        Postorder(current->rightchild);     // R
        allNodePos.push_back(current);
        current->index = indexCnt;
        indexCnt = indexCnt +1;
    }

}

void SlicingTree::invert(TreeNode *targerNode){
    if(targerNode->id =="H") targerNode->id = "V";
    else targerNode->id = "H";
}
vector<TreeNode*> SlicingTree::get_operatorPos(){
    return operatorPos;
}


void SlicingTree::M1(int node1,int node2,int idx){
    int temp;
    int index1=operandPos[node1]->index;
    int index2=operandPos[node2]->index;

    TreeNode *tempPtr;
    TreeNode *node1_parent=operandPos[node1]->parent;
    TreeNode *node2_parent=operandPos[node2]->parent;
    string node1_type=operandPos[node1]->childType;
    string node2_type=operandPos[node2]->childType;

    double old_Area,new_Area;
    old_Area=TotalArea();

    //Swap two operand
    if(node1_type == "R"){//node1
        node1_parent->rightchild = operandPos[node2];
        operandPos[node2]->childType = "R";
    }
    else{
        node1_parent->leftchild = operandPos[node2];
        operandPos[node2]->childType = "L";
    }

    if(node2_type == "R"){//node2
        node2_parent->rightchild = operandPos[node1];
        operandPos[node1]->childType = "R";
    }
    else{
        node2_parent->leftchild = operandPos[node1];
        operandPos[node1]->childType = "L";
    }

    //Change the parent of the swapped node
    operandPos[node1]->parent=node2_parent;
    operandPos[node2]->parent=node1_parent;

    //Change the index of the swapped node
    temp=operandPos[node1]->index;
    operandPos[node1]->index=operandPos[node2]->index;
    operandPos[node2]->index=temp;

    //Change the address of the swapped node
    tempPtr=allNodePos[index1];
    allNodePos[index1]=allNodePos[index2];
    allNodePos[index2]=tempPtr;

    //Change the operand sequence of the swapped node
    tempPtr=numSeq[idx];
    numSeq[idx]=numSeq[idx+1];
    numSeq[idx+1]=tempPtr;

    //M1 shape updating
    while(node1_parent){
        ShapeCombine(node1_parent);
        node1_parent=node1_parent->parent;
    }
    while(node2_parent){
        ShapeCombine(node2_parent);
        node2_parent=node2_parent->parent;
    }
    //Cost calculate
    new_Area=TotalArea();
    cost=new_Area-old_Area;

}

void SlicingTree::M2(TreeNode *targetPtr){
    double old_Area,new_Area;
    TreeNode *parentPtr = targetPtr->parent;
    TreeNode *childPtr = targetPtr->rightchild;
    TreeNode *tmpPtr;
    old_Area=TotalArea();

    //Invert chain
    if(targetPtr->childType == "N"){
        invert(targetPtr);
        while(childPtr->id =="H" ||childPtr->id =="V"){
            invert(childPtr);
            childPtr = childPtr->rightchild;
        }
        tmpPtr=childPtr->parent;
    }
    else if(targetPtr->childType == "L"){
        invert(targetPtr);
        while(childPtr->id =="H" ||childPtr->id =="V"){
            invert(childPtr);
            childPtr = childPtr->rightchild;
        }
        tmpPtr=childPtr->parent;
    }
    else{
        invert(targetPtr);
        while(parentPtr->childType == "R"){
            invert(parentPtr);
            parentPtr = parentPtr->parent;
        }
        invert(parentPtr);
        while(childPtr->id =="H" ||childPtr->id =="V"){
            invert(childPtr);
            childPtr = childPtr->rightchild;
        }
        tmpPtr=childPtr->parent;
    }



    //M2 shape updating
    while(tmpPtr){
        ShapeCombine(tmpPtr);
        tmpPtr=tmpPtr->parent;
    }
    //Cost calculate
    new_Area=TotalArea();
    cost=new_Area-old_Area;



}

void SlicingTree::M3(TreeNode *curPtr,TreeNode *sucPtr,int index){

    TreeNode *tempPtr=sucPtr->parent;
    int temp;
    //Check balloting property
    if( !M3_Check(curPtr,sucPtr,index) ) return;

    double old_Area,new_Area;
    old_Area=TotalArea();

    //Swap two adjacent operand and operator
    if(curPtr->childType == "L" && sucPtr->childType == "L"){;
        (curPtr->parent)->leftchild = curPtr->leftchild;
        (curPtr->leftchild)->parent = curPtr->parent;
        curPtr->leftchild = curPtr->rightchild;
        (sucPtr->parent)->leftchild = curPtr;
        curPtr->rightchild = sucPtr;
        curPtr->parent = sucPtr->parent;
        sucPtr->parent = curPtr;

        (curPtr->leftchild)->childType = "L";
        (curPtr->rightchild)->childType = "R";
        //shape updating
        while(curPtr){
            ShapeCombine(curPtr);
            curPtr=curPtr->parent;
        }

    }
    else if(curPtr->childType == "L" && sucPtr->childType == "R"){
        (curPtr->parent)->leftchild = curPtr->leftchild;
        (curPtr->leftchild)->parent = curPtr->parent;
        curPtr->leftchild = curPtr->rightchild;
        (sucPtr->parent)->rightchild = curPtr;
        curPtr->rightchild = sucPtr;
        curPtr->parent = sucPtr->parent;
        sucPtr->parent = curPtr;

        curPtr->childType = "R";
        (curPtr->leftchild)->childType = "L";
        //shape updating
        while(curPtr){
            ShapeCombine(curPtr);
            curPtr=curPtr->parent;
        }
    }
    else if(curPtr->childType == "R" && sucPtr->childType == "R"){
        (sucPtr->parent)->rightchild = sucPtr->rightchild;
        (sucPtr->rightchild)->parent = sucPtr->parent;
        sucPtr->rightchild = sucPtr->leftchild;
        ((sucPtr->parent)->leftchild)->parent = sucPtr;
        sucPtr->leftchild = (sucPtr->parent)->leftchild;
        (sucPtr->parent)->leftchild = sucPtr;
        curPtr->parent = sucPtr->parent;

        sucPtr->childType = "L";
        (sucPtr->rightchild)->childType ="R";
        while(sucPtr){
            ShapeCombine(sucPtr);
            sucPtr=sucPtr->parent;
        }
    }
    else if(curPtr->childType == "R" && sucPtr->childType == "L"){
        (sucPtr->parent)->leftchild=sucPtr->rightchild;
        (sucPtr->rightchild)->parent=sucPtr->parent;
        sucPtr->rightchild=sucPtr->leftchild;

        while(tempPtr->childType=="L"){
            tempPtr=tempPtr->parent;
        }
        sucPtr->leftchild=(tempPtr->parent)->leftchild;
        ((tempPtr->parent)->leftchild)->parent=sucPtr;
        (tempPtr->parent)->leftchild=sucPtr;
        sucPtr->parent=tempPtr->parent;

        (sucPtr->rightchild)->childType="R";
        curPtr->childType="L";

        while(sucPtr){
            ShapeCombine(sucPtr);
            sucPtr=sucPtr->parent;
        }

    }
    else{}

    //Change the address of the swapped node
    tempPtr=allNodePos[index];
    allNodePos[index]=allNodePos[index+1];
    allNodePos[index+1]=tempPtr;
    //Change the index of the swapped node
    temp=allNodePos[index]->index;
    allNodePos[index]->index=allNodePos[index+1]->index;
    allNodePos[index+1]->index=temp;
    //Cost calculate
    new_Area=TotalArea();
    cost=new_Area-old_Area;

}

bool SlicingTree::M3_Check(TreeNode *curPtr,TreeNode *sucPtr,int index){
    int operatorCnt=0;

    if( (curPtr->id!="H" && curPtr->id!="V") && (sucPtr->id!="H" && sucPtr->id!="V") ) //both node are number
    {
        /*cout<<"Both targer node are number."<<endl;*/
        return false;
    }
    else if( (curPtr->id=="H" || curPtr->id=="V") && (sucPtr->id=="H" || sucPtr->id=="V") ) //both node are H or V
    {
        /*cout<<"Both target node are H or V."<<endl;*/
        return false;
    }
    else{}

    // only 1 of 2 node is H or V
    if(curPtr->id == "H" && allNodePos[index+2]->id =="H") {//curPtr->id = H or V
        /*cout<<"Current node is H, and continuous H appear."<<endl;*/
        return false;
    }
    else if(curPtr->id =="V" && allNodePos[index+2]->id =="V") {//curPtr->id = H or V
        /*cout<<"Current node is V, and continuous V appear."<<endl;*/
        return false;
    }
    else if(curPtr->id !="H" && curPtr->id !="V" && sucPtr->id =="V" && allNodePos[index-1]->id =="V"){//curPtr->id = number
        /*cout<<"Current node is number, and continuous V appear."<<endl;*/
        return false;
    }
    else if(curPtr->id !="H" && curPtr->id !="V" && sucPtr->id =="H" && allNodePos[index-1]->id =="H"){//curPtr->id = number
        /*cout<<"Cueent Node is number, and continuous H appear."<<endl;*/
        return false;
    }
    else{}

    for(int i=1;i<=index+2;i++){
        if(allNodePos[i-1]->id == "H" || allNodePos[i-1]->id == "V") operatorCnt++;
    }

    if(2*operatorCnt < index+1) return true;
    else {
        /*cout<<"Equation 2Ni+1 < i doesn't hold. "<<endl;*/
        return false;
    }
}

//Determine the width and height of the soft module according to the aspect ratio
void SlicingTree::Initial_H_W(){

    if(operandPos.size()<=500){
        for(int i=0;i<operandPos.size();i++){
            for(double j=1.99;j>=0.51;j=j-0.001){
                (operandPos[i]->w).push_back( sqrt(operandPos[i]->area/j) );
                (operandPos[i]->h).push_back( sqrt(operandPos[i]->area*j) );
            }
        }
    }
    else if(operandPos.size()>500 &&operandPos.size()<=5000){
        for(int i=0;i<operandPos.size();i++){
            for(double j=1.99;j>=0.51;j=j-0.05){
                (operandPos[i]->w).push_back( sqrt(operandPos[i]->area/j) );
                (operandPos[i]->h).push_back( sqrt(operandPos[i]->area*j) );
            }
        }
    }
    else if(operandPos.size()>5000 &&operandPos.size()<10000){
        for(int i=0;i<operandPos.size();i++){
            for(double j=1.99;j>=0.51;j=j-0.1){
                (operandPos[i]->w).push_back( sqrt(operandPos[i]->area/j) );
                (operandPos[i]->h).push_back( sqrt(operandPos[i]->area*j) );
            }
        }
    }
    else if(operandPos.size()>=10000 &&operandPos.size()<=20000){
        for(int i=0;i<operandPos.size();i++){
            for(double j=1.99;j>=0.51;j=j-0.3){
                (operandPos[i]->w).push_back( sqrt(operandPos[i]->area/j) );
                (operandPos[i]->h).push_back( sqrt(operandPos[i]->area*j) );
            }
        }
    }
    else if(operandPos.size()>20000){
        for(int i=0;i<operandPos.size();i++){
                (operandPos[i]->w).push_back( sqrt(operandPos[i]->area/1.0) );
                (operandPos[i]->h).push_back( sqrt(operandPos[i]->area*1.0) );
           
        }
    }
 


}

//Combine two node's shape curve
void SlicingTree::ShapeCombine(TreeNode *targetPtr){
    vector<double> leftchild_h=(targetPtr->leftchild)->h;
    vector<double> leftchild_w=(targetPtr->leftchild)->w;
    vector<double> rightchild_h=(targetPtr->rightchild)->h;
    vector<double> rightchild_w=(targetPtr->rightchild)->w;
    vector<double> w,h;
    unordered_map<double,string> tableAreaChild;
    double newWidth,newHeight,newArea;
    string str;


    int i=1,j=1,k=1;
    if( targetPtr->id == "V" ){
        while(i<=leftchild_h.size()&&j<=rightchild_h.size()){
            newWidth=leftchild_w[i-1]+rightchild_w[j-1];
            newHeight=max(leftchild_h[i-1],rightchild_h[j-1]);
            newArea=newHeight*newWidth;
            str=to_string(i-1)+" "+to_string(j-1);
            tableAreaChild.insert(make_pair(newArea,str));
            w.push_back(newWidth);
            h.push_back(newHeight);
            k=k+1;
            if(newHeight==leftchild_h[i-1]){
                i++;
            }

            if(newHeight==rightchild_h[j-1]){
                j++;
            }
        }
    }
    else if( targetPtr->id== "H" ){
        i=leftchild_h.size(),j=rightchild_h.size();
        while(i>=1&&j>=1){
            newHeight=leftchild_h[i-1]+rightchild_h[j-1];
            newWidth=max(leftchild_w[i-1],rightchild_w[j-1]);
            newArea=newHeight*newWidth;
            str=to_string(i-1)+" "+to_string(j-1);
            tableAreaChild.insert(make_pair(newArea,str));
            w.push_back(newWidth);
            h.push_back(newHeight);
            k=k+1;
            if(newWidth==leftchild_w[i-1])
                i--;
            if(newWidth==rightchild_w[j-1])
                j--;
        }
        reverse(w.begin(),w.end());
        reverse(h.begin(),h.end());

    }
    else{}

    targetPtr->w=w;
    targetPtr->h=h;
    targetPtr->tableAreaChild=tableAreaChild;
}

void SlicingTree::BottomUp(){
    for(int i=operatorPos.size()-1;i>=0;i--){
        ShapeCombine(operatorPos[i]);
    }
}

//Return the minimal floorplanning area
double SlicingTree::TotalArea(){
    double min_area;
    vector<double> area,w,h;
    w=root->w;
    h=root->h;
    for(int i=0;i<w.size();i++){
        area.push_back(w[i]*h[i]);
    }
    auto it=min_element(area.begin(),area.end());
    min_area=*it;
    return min_area;
}

void SlicingTree::Topdown(){

    vector<double>area;
    vector<double> w;
    vector<double> h;
    int min_inx;
    w=root->w;
    h=root->h;


    for(int i=0;i<w.size();i++){
        area.push_back(w[i]*h[i]);
    }

    auto it = min_element(area.begin(),area.end());
    min_inx =it - area.begin();
    root->area = *it;
    root->final_h=h[min_inx];
    root->final_w=w[min_inx];
    cout<<"----------------------------------"<<endl;
    cout<<"Floorplan total area: "<<root->area<<endl;
    cout<<"total_width: "<<root->final_w<<endl;
    cout<<"total height: "<<root->final_h<<endl;
    cout<<"----------------------------------"<<endl;
    for(int i=0;i<operatorPos.size();i++){
        DecideModule(operatorPos[i]);
    }
}

void SlicingTree::DecideModule(TreeNode *target){
    TreeNode *leftPtr=target->leftchild;
    TreeNode *rightPtr=target->rightchild;
    unordered_map<double,string> tableAreaChild;
    vector<double> w;
    vector<double> h;
    string v,str1,str2;//table's value
    double a;//temp area
    int leftchild_index,rightchild_index;
    tableAreaChild=target->tableAreaChild;

    v=tableAreaChild[target->area];
    istringstream iss(v);
    iss >> str1 >> str2;

    leftchild_index=stoi(str1);
    w=leftPtr->w;
    h=leftPtr->h;
    leftPtr->final_w=w[leftchild_index];
    leftPtr->final_h=h[leftchild_index];
    leftPtr->area=w[leftchild_index]*h[leftchild_index];

    rightchild_index=stoi(str2);
    w=rightPtr->w;
    h=rightPtr->h;
    rightPtr->final_w=w[rightchild_index];
    rightPtr->final_h=h[rightchild_index];
    rightPtr->area=w[rightchild_index]*h[rightchild_index];

}
void SlicingTree::SelectMove(int M,int r){

    int node1,node2;

    if(M==0){ //M1

        node1=stoi(numSeq[r]->id);
        node2=stoi(numSeq[r+1]->id);
        M1(node1,node2,r);
    }
    else if(M==1){ //M2
        M2(operatorPos[r]);
    }
    else{ //M3
        M3(allNodePos[r],allNodePos[r+1],r);
    }
}

void SlicingTree::WongLiu(int area,clock_t ACC_begin){
    double T=-(area)/log(0.9999),freezeT=5,Random,duration;//T:Temperature
    int M=0,MT=0,uphill=0,N=5*operandPos.size();
    int reject,r;
    int node1,node2;
    int operandSize=operandPos.size()-1;
    int operatorSize=operatorPos.size()-1;
    int allNodePosSize=allNodePos.size()-2;
    double best_area=TotalArea();
    double new_area;
    clock_t Begin,End,ACC_end;


    if(operandPos.size()>=150) return;

    while(true){

        ACC_end=clock();
        duration=double(ACC_end-ACC_begin)/CLOCKS_PER_SEC;
        // If duration > 6600 seconds then break
        if(duration>6600) {
            break;
        }

        MT=0;
        uphill=0;
        reject=0;
        Begin=clock();
        while(true){
            ACC_end=clock();
            duration=double(ACC_end-ACC_begin)/CLOCKS_PER_SEC;
            // If duration > 6600 seconds then break
            if(duration>6600) {
                break;
            }
            M=rand()%3;
            if(M==0)
                r=rand()%operandSize;
            else if(M==1)
                r=rand()&operatorSize;
            else
                r=rand()&allNodePosSize;
            SelectMove(M,r);
            new_area=TotalArea();
            MT=MT+1;
            Random = (double) rand() / (RAND_MAX + 1.0);
            if( cost<=0 || Random<exp(-cost/T) ){
                if(cost>0)
                    uphill=uphill+1;
                if(new_area<best_area){
                    cout<<"new_area: "<<new_area<<", best_area: "<<best_area<<endl;
                    bestPE="";
                    best_area=new_area;
                    best_operandPos=operandPos;
                    best_operatorPos=operatorPos;
                    best_PE=allNodePos;
                    for(int i=0;i<best_operatorPos.size();i++){
                        bestPE=bestPE+best_operatorPos[i]->id;
                        if(i!=best_operatorPos.size()-1){
                            bestPE=bestPE+" ";
                        }
                    }
                }
            }
            else{ //Swap to original
                if(M==0){
                    node1=stoi(numSeq[r]->id);
                    node2=stoi(numSeq[r+1]->id);
                    M1(node1,node2,r);
                    MT=MT-1;
                }
                else if(M==1){
                    M2(operatorPos[r]);
                    MT=MT-1;
                }
                else{
                    M3(allNodePos[r],allNodePos[r+1],r);
                    MT=MT-1;
                }
                reject=reject+1;
            }
            if( (uphill>N) || (MT>N) ){
                break;
            }

        }
        End=clock();
        ACC_end=clock();
        duration=double(End-Begin)/CLOCKS_PER_SEC;

        T=0.85*T;


        if(reject/MT >0.95 || T<freezeT ||duration>6000){
            break;
        }
    }


    stringstream sstream(bestPE);
    string substr;
    int k=0;
    while(sstream >> substr){
        operatorPos[k]->id=substr;
        k++;
        if(k==operatorPos.size())
            break;
    }


}

void SlicingTree::genBestSlicingTree(){
    TreeNode *left,*right;
    stack<TreeNode*> s;

    if(operandPos.size()>=150) return;


    for (int i =0;i< best_PE.size();i++) {
        if(best_PE[i]->id == "H" || best_PE[i]->id == "V"){
            right=s.top();
            s.pop();
            left=s.top();
            s.pop();
            best_PE[i]->leftchild=left;
            (best_PE[i]->leftchild)->childType="L";
            best_PE[i]->rightchild=right;
            (best_PE[i]->rightchild)->childType="R";
            left->parent=best_PE[i];
            right->parent=best_PE[i];
            s.push(best_PE[i]);
        }
        else{
            s.push(best_PE[i]);
        }

    }

    root=s.top();
    root->childType="N";
    allNodePos=best_PE;
    BottomUp();
    Topdown();


}
/////////////end of slicing tree class ///////////////

///////////class Myfile////////////////////////
class Myfile{
private:
    vector<string> tokens;
public:
    Myfile();
    int Readfile(char *argv[]);
    void Writefile(SlicingTree T,char *argv[]);
    vector<string> getTokens();


};
Myfile::Myfile(){

}

int Myfile::Readfile(char *argv[]) {

    ifstream inFile(argv[1]);
    string line;
    int i=0,area=0;
    while (getline(inFile, line)) {
        stringstream ss(line);
        string token;
        while (ss >> token) {
            tokens.push_back(token);
            if(i!=0 && i%2==0){
                area=area+stoi(tokens[i]);
            }
            i++;
        }
    }
    area=area/stoi(tokens[0]);
    inFile.close();

    return area;

}

void Myfile::Writefile(SlicingTree T,char *argv[]) {

    ofstream outFile (argv[2]); // write file
    if( !outFile ){
        cerr << "File could not be opened" << endl;
        exit(1);
    }
    outFile<<T.root->final_w<<" "<<T.root->final_h<<" "<<T.root->area<<endl;
    for(int i=0;i<T.operandPos.size();i++){
        outFile<<T.operandPos[i]->final_w<<" "<<T.operandPos[i]->final_h<<endl;
    }
    for(int i=0;i<T.allNodePos.size();i++){
        if(i!=T.allNodePos.size()-1)
            outFile<<T.allNodePos[i]->id<<" ";
        else
            outFile<<T.allNodePos[i]->id;
    }
    outFile.close();

}

vector<string> Myfile::getTokens(){
    return tokens;
}
///////////end of class Myfile////////////////////////

int main(int argc,char *argv[]) {
    clock_t Begin,End;
    int area;
    double duration;
    vector<string> tokens;
    vector<TreeNode*> operatorPos;
    Myfile f;
    SlicingTree T;
    area=f.Readfile(argv);
    tokens=f.getTokens();
    Begin=clock();
    T.buildTree(tokens);
    operatorPos=T.get_operatorPos();
    srand(time(0));
    T.WongLiu(area,Begin);
    T.genBestSlicingTree();
    f.Writefile(T,argv);
    return 0;
}


