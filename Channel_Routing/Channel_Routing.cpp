//left-edge algorithm
// Program objective: detailed channel routing: minimize total tracks
// Date:2023/06/08
// Author:Guan-You Chen
#include <iostream>
#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
using namespace std;

struct Node{
    Node *nextPtr;//Point to next Node
    int id;//Represent terminal's number

    Node():nextPtr(0),id(0){};
    Node(int i):nextPtr(0),id(i){};
};

//begin class Myfile
class Myfile{

public:
    vector<int> UpperPin,LowerPin,OrderPin; //OrderPin: Record each pin based on pin's left-end from smallest to largest
    map<int,int>pinVistOrNot;//Record of pin visitation
    int maxPin,totalPin;//maxPin: Record of the largest pin number, totalPin:Record of the total pin count

    void readFile();

friend class Routing;
};


void Myfile::readFile(){
    unordered_map<int,int>orderTable;
    ifstream inFile("case11.txt");
    int tmp;
    string line;
    bool endCheck = false;
    maxPin=0,totalPin=0;


    while (getline(inFile, line)) {
        if(line[line.size()-1]==' ') //Remove blank from input file
            line.erase(line.size()-1,1);
        stringstream ss(line);
        string token;
        while (ss >> token) {

            tmp=stoi(token);
            if(!endCheck){
                if(pinVistOrNot[tmp]==0 ){
                    pinVistOrNot[tmp]=1;
                    if(tmp!=0){ //Calculate totalPin
                        totalPin++;
                    }
                    if(tmp > maxPin)
                        maxPin=tmp;//Calculate maxPin
                }
                UpperPin.push_back(stoi(token));
                if (ss.eof() ) { // To determine if you have reached the last token of the first line
                    endCheck = true;
                }
            }
            else{
                if(!pinVistOrNot[tmp]){
                    pinVistOrNot[tmp]=1;
                    if(tmp!=0){
                        totalPin++;
                    }
                    if(tmp > maxPin)
                        maxPin=tmp;
                }
                LowerPin.push_back(stoi(token));
            }

        }//end inner while
    }//end outer while




    //Build orderPin based on pin's left-end coordinate
    for(int i=0;i<UpperPin.size();i++){
        if(orderTable[UpperPin[i]]==0){
            orderTable[UpperPin[i]]=1;
            if(UpperPin[i]!=0)
                OrderPin.push_back(UpperPin[i]);
        }
        if(orderTable[LowerPin[i]]==0){
            orderTable[LowerPin[i]]=1;
            if(LowerPin[i]!=0)
                OrderPin.push_back(LowerPin[i]);
        }




    }

    //Print some basic infomations
    cout<<"***********Readfile***********"<<endl;
    cout<<"UpperPin:";
    for(int i=0;i<UpperPin.size();i++){
        cout<<UpperPin[i]<<" ";
    }
    cout<<endl;
    cout<<"LowerPin:";
    for(int i=0;i<LowerPin.size();i++){
        cout<<LowerPin[i]<<" ";
    }
    cout<<endl;
    cout<<"maxPin: "<<maxPin<<", totalPin: "<<totalPin<<endl;
    cout<<"OrderPin:"<<endl;
    for(int i=0;i<OrderPin.size();i++){
        cout<<OrderPin[i]<<" ";
    }
    cout<<endl;
    cout<<"*****************************"<<endl;
    inFile.close();
}



// end class Myfile

//begin Routing
class Routing{
private:
    vector<Node*> sucList,preList;//sucList: VCG_Graph , preList: Record node's parent
    vector<int> topLevel,track,OrderPin,LimitOrNot; //LimitedOrNot: 1 indicates that this node is not ready to be executed
    vector<vector<int>> U,pinPosition,pinDirection;//U:Record the left and right coordinates of each Net
    //pinPosition : Record each pin's coordinate, pinDirection: 1 indicates that this pin is on the TOP
    vector<bool> pinFirst;//pinFirst=1 indicates that this pin has not been encountered yet
    int maxTrack;//maxTrack: The maximum track number
    map<int,int> pinExist; //When pin existthen value =1
    int maxPin;//maxPin: The number of the largest pin
public:
    Routing(Myfile f);
    void VCG_build(Myfile f);//Build VCG Graph
    void insertNode(int,int);//Insert node
    void deleteNode(int,int);//Delete Node
    void clearList(int);// Clear a specific row of a linked list
    void LeftEdge();//Left edge algorithm
    void writeFile();
};

Routing::Routing(Myfile f){ //Initialize variable

    for(int i=0;i<=f.maxPin;i++){
        sucList.push_back(nullptr);
        preList.push_back(nullptr);
        U.push_back({1,1});
        pinFirst.push_back(1);
        track.push_back(0);
        LimitOrNot.push_back(1);
        pinPosition.push_back(std::vector<int>());
        pinDirection.push_back(std::vector<int>());
    }
    maxTrack=1;
    maxPin=f.maxPin;
    pinExist=f.pinVistOrNot;
    OrderPin=f.OrderPin;
}

void Routing::VCG_build(Myfile f){
    vector<int> UpperPin=f.UpperPin;
    vector<int> LowerPin=f.LowerPin;
    Node* tmp,sucPtr,prePtr;

    for(int i=0;i<UpperPin.size();i++){
        if(UpperPin[i]!=0 && LowerPin[i]!=0){
            insertNode(UpperPin[i],LowerPin[i]);


        }//end if
        if(pinFirst[UpperPin[i]]==true){//UpperPin
            pinFirst[UpperPin[i]]=false;
            U[UpperPin[i]][0]=i+1,U[UpperPin[i]][1]=i+1;
        }
        else{
            U[UpperPin[i]][1]=i+1;
        }//end else
        if(pinFirst[LowerPin[i]]==true){//LowerPin
            pinFirst[LowerPin[i]]=false;
            U[LowerPin[i]][0]=i+1,U[LowerPin[i]][1]=i+1;
        }
        else{
            U[LowerPin[i]][1]=i+1;
        }//end else


        pinPosition[UpperPin[i]].push_back(i+1);//Record pin's coordinate
        pinDirection[UpperPin[i]].push_back(1); //1: TOP
        pinPosition[LowerPin[i]].push_back(i+1);//Record pin's coordinate
        pinDirection[LowerPin[i]].push_back(0); //0: BOT


    }//end for

    for(int i=1;i<=f.maxPin;i++){
        if(preList[i] == NULL){
            LimitOrNot[i]=0;
        }
    }//end for


}
void Routing::insertNode(int TOP,int BOT){
    Node *newNode1 = new Node(TOP);
    Node *newNode2 = new Node(BOT);
    Node *curPtr;
    if(sucList[TOP]==0){//create sucList(VCG)
        sucList[TOP]=newNode2;
    }
    else{
        curPtr=sucList[TOP];
        while(curPtr->nextPtr){
            curPtr=curPtr->nextPtr;
        }
        curPtr->nextPtr=newNode2;
    }

    if(preList[BOT]==0){//create preList
        preList[BOT]=newNode1;
    }
    else{
        curPtr=preList[BOT];
        while(curPtr->nextPtr){
            curPtr=curPtr->nextPtr;
        }
        curPtr->nextPtr=newNode1;
    }
}

void Routing::deleteNode(int x,int idx){
     Node *current = preList[idx],*previous = 0;
    while (current != 0 && current->id != x) {
        previous = current;
        current = current->nextPtr;
    }

    if (current == 0) {
        std::cout << "There is no " << x << " in list.\n";
    }
    else if (current == preList[idx]) {
    	preList[idx] = current->nextPtr;
        delete current;
        current = 0;
    }
    else {
        previous->nextPtr = current->nextPtr;
        delete current;
        current = 0;
    }

}

void Routing::clearList(int x){
    vector<int> nullVec;
    Node* tmp;
    while(sucList[x]!=0){
        Node *curPtr=sucList[x];
        deleteNode(x,curPtr->id);
        sucList[x]=sucList[x]->nextPtr;
        delete curPtr;
        curPtr=0;
    }

    OrderPin.erase(remove(OrderPin.begin(),OrderPin.end(),x),OrderPin.end() );



    pinExist[x]=0;


    for(auto it = pinExist.begin(); it != pinExist.end(); ++it){
        if(it->second != 0 && it->first !=0){
            if(preList[it->first] == NULL)
                LimitOrNot[it->first] = 0;
        }
    }


}

void Routing::LeftEdge(){

    int t=0,watermark=0,j=0;

    while(OrderPin.size() != 0){
        t=t+1;
        watermark=0;

        for(int i=0;i<OrderPin.size();i++){

            if(LimitOrNot[OrderPin[i]] == 0 && U[OrderPin[i]][0] > watermark){
                track[OrderPin[i]]=t;
                watermark=U[OrderPin[i]][1];
                clearList(OrderPin[i]);
                i--;
            }
        }
    }

    auto maxNum = max_element( track.begin(),track.end() );
    maxTrack=*maxNum;

    for(int i=1;i<track.size();i++){
    	track[i]=maxTrack+1-track[i];
    }

}

void Routing::writeFile(){
    ofstream outFile("out11.txt",ios::out);
    if( !outFile ){
        cerr << "File could not be opened" << endl;
        exit(1);
    }
    for(auto it = pinExist.begin(); it != pinExist.end(); ++it){

        if(it->first == 0){
            continue;
        }

        if(LimitOrNot[it->first]==0){
            outFile<<".begin "<<it->first<<endl;
            outFile<<".H "<<U[it->first][0]-1<<" "<<track[it->first]<<" "<<U[it->first][1]-1<<endl;
            for(int j=0;j<pinDirection[it->first].size();j++){
                if(pinDirection[it->first][j] == 1){
                    outFile<<".V "<<pinPosition[it->first][j]-1<<" "<<track[it->first]<<" "<<maxTrack+1<<endl;
                }
                else{
                    outFile<<".V "<<pinPosition[it->first][j]-1<<" "<<0<<" "<<track[it->first]<<endl;
                }
            }//end inner for
            outFile<<".end"<<endl;
        }//end if
    }//end outer for
    outFile.close();
}

// end Routing

int main(){
    Myfile f;
    f.readFile();
    Routing r(f);
    r.VCG_build(f);
    r.LeftEdge();
    r.writeFile();
    return 0;
}
