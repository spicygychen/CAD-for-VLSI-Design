//PA2: Scheduling
//This program reads an input file describing a sequencing graph and solves the MR-LCS problem using the list scheduling algorithm.
//Finally, it writes the scheduling result to an output file.
//2023/04/03 Version1
//Author: Guan-You Chen

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// class Graph begin
class Graph{
public:
    Graph(); // Initial private member
    void genList(vector<string>); // Create preList & sucList
    void setNodeInfo(vector<string>);// Set nodeInfo such as nodeType,nodeLatency,etc..
    void ALAP (); // Implement ALAP algorithm
    void LIST_R(); // iImplement list scheduling algorithm
    vector<string> getNodeType(); // Get NodeType vector
    vector<int> getNodeLevel(); // Get NodeLevel vector
    int* getArray(); // Get a[]
    int getNodeSize();// Get NodeSize (total node plus 1 nop node at the end)
private:
    vector<vector<int>> sucList; //Record each node's successor
    vector<vector<int>> preList; //Record each node's precessor
    vector<string> nodeType;// Record type of a node
    vector<int> nodeLatency,nodeIndex;// Record the latency of a node and the index of a node in the token vector
    vector<int> nodeReady,vReady;// Used for ALAP/LIST_R , record the finish status of a node
    vector<int> node_maxReadyTime; // Record the latest execution time of a node
    vector<int> sucCount,preCount; // Record the number of predecessors/succussor of a node
    vector<int>nodeSlack,nodeLevel;// Record the slack and level of a node at the time of scheduling
    vector<int> nodeCmpt; // Record whether a node is currently executing
    int lambda; // Latency constraint
    int nodeSize; // Total node plus 1 nop node at the end
    int a[2]; // Index 0 : "+" , index 1: "*"
};
// class Graph end

class Myfile{

public:
    Myfile();
    void readFile(char* []); //Readfile
    void writeFile(Graph,char* []); // Writefile
    void deleteSpace(char* []); //Remove whitespace and newline characters from the file string
    vector<string> getTokens(); //Get private member tokens
private:
    vector<string> tokens; // Store the file contents into tokens
};



///////////////////main function////////////////////
int main(int argc, char *argv[]) {
    vector <string> tokens;
    vector <int> nodeLatency,nodeReady;
    Myfile f;
    Graph g;
    f.readFile(argv);
    tokens = f.getTokens();
    g.setNodeInfo(tokens);
    g.genList(tokens);
    g.ALAP();
    g.LIST_R();
    f.writeFile(g,argv);
    f.deleteSpace(argv);
}
////////////////////end main//////////////////////////





/////////// Function start of Class Myfile ///////////

Myfile::Myfile(){

}

void Myfile::readFile(char* argv[]) {

    ifstream inFile(argv[1]); //argv[1] = The name of input file
    string line;
    while (getline(inFile, line)) {
        stringstream ss(line);
        string token;
        while (ss >> token) {
            tokens.push_back(token); // Store it into tokens
        }
    }
    inFile.close(); // close file

}

void Myfile::writeFile(Graph g, char* argv[]) {

    int l=1;
    string filename=string(argv[1])+".out";
    vector<int> nodeLevel = g.getNodeLevel();
    vector<string> nodeType = g.getNodeType();
    int nodeSize = g.getNodeSize();
    int* a = g.getArray();

  

    ofstream outFile (filename,ios::out); // argv[2] = output file name
    if( !outFile ){
        cerr << "File could not be opened" << endl;
        exit(1);
    }
    outFile<<a[0]<<endl;
    outFile<<a[1]<<endl;

    while( l <= nodeLevel[nodeSize]){
        for(int i=1;i<nodeSize;i++){
            //The addition node will be scheduled across three levels.
            if(nodeType[i] == "+" ){
                if(nodeLevel[i] == l) {
                    outFile<<i<<" ";

                }
            }
            //The multiplication node will be scheduled across three levels.
            else if(nodeType[i] == "*" ){
                if( (l >= nodeLevel[i]) && (l <= nodeLevel[i]+2) ) {
                    outFile<<i<<" ";
                }

            }

        }


        if(l != nodeLevel[nodeSize]) outFile <<endl;
        l=l+1;

    }
}

void Myfile::deleteSpace(char* argv[]){

    string filename =string( argv[1] )+".out";
    ifstream infile(filename); // Open file

    if (!infile) {
        cerr << "無法開啟檔案: " << filename << endl;
    }

    // Read file
    string line;
    string new_content = "";
    while (getline(infile, line)) {
        if (!line.empty() && line[line.length()-1] == ' ') {
            // Remove the trailing whitespace characters at the end of each line
            size_t last_char = line.find_last_not_of(" \t\r\n");
            if (last_char != string::npos) {
                line.erase(last_char+1);
            }
        }
        new_content += line;
        if (!infile.eof()) {
            new_content += "\n";
        }
    }
    infile.close(); // Close file

    ofstream outfile(filename); // Open the file for writing

    if (!outfile) {
        cerr << "無法開啟檔案: " << filename << endl;
    }

    // Write the new content back to the original file
    if (!new_content.empty() && new_content.back() == '\n') {
        new_content.erase(new_content.find_last_not_of(" \t\r\n") + 1);
    }


    outfile << new_content;
    outfile.close(); // Close file
}


vector<string> Myfile::getTokens(){
    return tokens;
}
/////////////////Function end of Class Myfile////////////////////////



/////////////////Function start of Class Graph///////////////////////
Graph::Graph(){
    //Set node info for input nop (node0)
    nodeIndex.push_back(0);
    nodeLatency.push_back(0);
    nodeReady.push_back(0);
    nodeType.push_back("n0"); //"n0" means input nop( node 0 )
    preList.push_back(vector<int>());
    sucList.push_back(vector<int>());
    vReady.push_back(1);
    lambda = 0;
    nodeSize=0;
    a[0]=1; // At the beginning, there is an adder.
    a[1]=1; //At the beginning, there is an multiplier.
}

//Generate the successor and predecessor for each node.
void Graph::genList(vector<string> vec){
    //"temp1" is a temporary variable used to store data that is being converted from a String data type to an integer data type
    //"dif" may refer to the number of nodes that follow an operator in a parsed file
    //"cnt" is used to count the number of successors of a node
    //"cnt2" is used to count the number of predecessors of a node
    int temp1,dif,cnt=0,cnt2=0;
    //Create the predecessor and successor lists of a node in a graph
    for (int i=1; i < nodeSize ; i++){
        if(nodeType[i] == "o"){
            sucList[i].push_back(nodeSize);
            preList[nodeSize].push_back(i);
        } //end if

        else if(nodeType[i] == "i"){

            if(i == (sucList.size()-2) ){
                dif = vec.size()-nodeIndex[i]-1;
            }
            else{
                dif = nodeIndex[i+1]-nodeIndex[i]-2;
            }

            for(int j=1;j<=dif;j++){
                temp1 = stoi(vec[nodeIndex[i]+j]);
                sucList[i].push_back(temp1);
                preList[temp1].push_back(i);


            }
            sucList[0].push_back(i);
            preList[i].push_back(0);

        }//end else if
        else if(nodeType[i] == "*"){

            if(i == (sucList.size()-2) ){ //15:32改這裡
                dif = vec.size()-nodeIndex[i]-1;
            }
            else{
                dif = nodeIndex[i+1]-nodeIndex[i]-2;
            }

            for(int j=1;j<=dif;j++){
                temp1 = stoi(vec[nodeIndex[i]+j]);
                sucList[i].push_back(temp1);
                preList[temp1].push_back(i);
            }
        }//end else if
        else if(nodeType[i] == "+"){

            if(i == (sucList.size()-2) ){
                dif = vec.size()-nodeIndex[i]-1;
            }
            else{
                dif = nodeIndex[i+1]-nodeIndex[i]-2;
            }

            for(int j=1;j<=dif;j++){
                temp1 = stoi(vec[nodeIndex[i]+j]);
                sucList[i].push_back(temp1);
                preList[temp1].push_back(i);
            }
        }//end else if
        else{

        }//end else
    }//end for

    sucList[nodeSize].push_back(0); // put value to output nop ("n1")

    //Count the number of successors  of a node
    for(int i = 0; i <= nodeSize; i++)
	{
		for(int j=0;j<sucList[i].size();j++){
                cnt++;
		}
		if (nodeType[i] == "o") {
            cnt=1;
            sucCount.push_back(cnt);
		}
		else if(nodeType[i]=="n1") {
            cnt=0;
            sucCount.push_back(cnt);
		}
		else{
            sucCount.push_back(cnt);
		}

		cnt=0;

	}
    //Count the number of predecessors  of a node
	for(int i=0;i<=nodeSize;i++){
        for(int j=0;j<preList[i].size();j++){
            cnt2++;

        }
        if (nodeType[i] == "n0") {
            cnt2=0;
            preCount.push_back(cnt2);
		}
		else if(nodeType[i] == "i"){
            cnt2=1;
            preCount.push_back(cnt2);
		}
		else{
            preCount.push_back(cnt2);
		}//end else
		cnt2=0;
	}//end for

}//end genList

void Graph::setNodeInfo(vector<string> vec){

    lambda = stoi(vec[5]); //Set time constraint

    //Set node inforamtion
    for(int i=0;i<vec.size();i++){
        if(vec[i] == "i" ){
            nodeIndex.push_back(i);
            nodeLatency.push_back(0);
            nodeReady.push_back(0);
            vReady.push_back(1);
            nodeType.push_back("i");
            preList.push_back(vector<int>());
            sucList.push_back(vector<int>());
        }
        else if(vec[i] == "o"){
            nodeIndex.push_back(i);
            nodeLatency.push_back(0);
            nodeReady.push_back(1); // 2023/04/02 3:48pm
            vReady.push_back(0);
            nodeType.push_back("o");
            preList.push_back(vector<int>());
            sucList.push_back(vector<int>());
        }
        else if(vec[i] == "*"){
            nodeIndex.push_back(i);
            nodeLatency.push_back(3);
            nodeReady.push_back(0);
            vReady.push_back(0);
            nodeType.push_back("*");
            preList.push_back(vector<int>());
            sucList.push_back(vector<int>());
        }
        else if(vec[i] == "+"){
            nodeIndex.push_back(i);
            nodeLatency.push_back(1);
            nodeReady.push_back(0);
            vReady.push_back(0);
            nodeType.push_back("+");
            preList.push_back(vector<int>());
            sucList.push_back(vector<int>());
        }
        else { //nop

        }//end else
    }//end for

    preList.push_back(vector<int>()); //Output to NOP node
    sucList.push_back(vector<int>());  // NodeSize mean the node number of NOP
    nodeLatency.push_back(0);
    nodeReady.push_back(1);
    vReady.push_back(0);
    nodeType.push_back("n1"); //"n1" mean output to  nop
    nodeIndex.push_back(0);
    nodeSize = preList.size()-1;

    //Initial node_maxReady
    //Initial nodeSlack
    //Initial nodeLevel
    //Initial nodeCmpt
    for(int i=0;i<=nodeSize;i++){
        node_maxReadyTime.push_back(lambda+1);
        nodeSlack.push_back(1);
        nodeLevel.push_back(1);
        nodeCmpt.push_back(0);
    }//end for
}//end setNodeInfo


void Graph::ALAP(){ //ALAP algorithm
    node_maxReadyTime[nodeSize] = lambda+1 ;
    vector<int> temp; // Store node's maxReadyTime temporarily
    int tmp;
    // Stop while loop when input nop (node0) has been scheduled
    while(nodeReady[0] != 1){ //nodeReady = 1 mean that the node has been scheduled
        for(int i = nodeSize-1;i>=0;i--){ //Checking from the maximum node number
	    temp = vector<int>{};
            for(int j=0;j<sucList[i].size();j++){//Check a node's successors
		if(nodeReady[i]==1) break;
                if(nodeReady[sucList[i][j]] == 1){
                    sucCount[i]--;
                    tmp = node_maxReadyTime[ sucList[i][j] ] - nodeLatency[i] ;
                    temp.push_back( tmp );
                }//end if
                if(sucCount[i] == 0){
                    nodeReady[i] = 1;
                    //To compare all possible latest times on a node and choose the minimum
                    auto min_it = min_element(temp.begin(), temp.end());
                    node_maxReadyTime[i] = *min_it ;
                    temp = vector<int>{};
                }//end if
            }//end for
        }//end for
    }//end while
}//end ALAP

void Graph::LIST_R(){
    //"tmp" pre-storing the preCount of each node and popping it back when jumping to the next level
    int tmp=0,l=1; //l means current Level
    int addCnt=0,mulCnt=0; //"addCnt" represents the number of adders that are required at the current leve
    if(node_maxReadyTime[0] < 0){
        cout << "not feasible" <<endl;
    }//end if
    else{
        while(vReady[nodeSize] != 1){//Stop while loop when output nop ("n1") has been scheduled
            for( int i=1; i<=nodeSize ; i++){//Checking from the minimum node number
                tmp = preCount[i];
                for(int j=0;j < preList[i].size() ; j++){
                    if(nodeType[i] == "+"){
                        if(vReady[i] == 1) break;

                        if( (vReady[ preList[i][j] ] == 1) && (nodeCmpt[ preList[i][j] ] == 0) ) {
                            preCount[i]--;
                        }//end if

                        if(preCount[i] == 0){ //When the node's predecessor has been scheduled
                            nodeSlack[i]=node_maxReadyTime[i]-l; //Calculate node's slack
                        }//end if

                        if(nodeSlack[i] == 0){//When slack=0, schedule the node
                            vReady[i] = 1;
                            nodeCmpt[i] =1; //When the node is being executed, nodeCmpt=1

                            addCnt++;

                            nodeLevel[i]=l;

                            if(addCnt > a[0] ){ //At the beginning, there is one adder
                                a[0] = a[0] +1;
                            }
                            else{

                            }

                        }//end if
                        else{
                            vReady[i]=0; //When slack !=0, the node cannot be scheduled
                        }//end else

                    }//end if
                    else if(nodeType[i] == "*"){
                        if(vReady[i] == 1) break;

                        if( (vReady[ preList[i][j] ] == 1) && (nodeCmpt[ preList[i][j] ] == 0) ) {
                            preCount[i]--;
                        }

                        if(preCount[i] == 0){
                            nodeSlack[i]=node_maxReadyTime[i]-l;
                            if(nodeSlack[i]<=0 && nodeSlack[i] >=-2){ //Because the execution time for multiplication is 3 cycles
                                if(nodeSlack[i]==0) nodeLevel[i]=l; //end if
                                mulCnt++;
                                nodeCmpt[i]=1;
                                if(mulCnt > a[1]) a[1]=a[1]+1; //end if
                            }//end if
                        }//end if

                        if(nodeSlack[i] == -3){
                            vReady[i]=1;
                            nodeCmpt[i]=0;
                        }//end if
                        else{
                            vReady[i]=0;
                        }
                    } // end else if
                    else{
                        if(vReady[ preList[i][j] ] == 1 ) {
                            preCount[i]--;

                        }
                        if(preCount[i] == 0){
                            vReady[i] = 1;
                            if(nodeType[i] == "i") nodeLevel[i] = 1;
                            else nodeLevel[i]=l;
                        }
                    }//end else
                }//end inner for Loop
                for(int i=1;i<nodeSize;i++){ //Reset nodeCmpt
                    if(nodeType[i] =="+"){
                        nodeCmpt[i]=0;
                    }
                    else if(nodeType[i] =="*"){
                        if( l == nodeLevel[i]+2 ) nodeCmpt[i]=0;
                    }
                    else{
                        nodeCmpt[i]=0;
                    }
                }//end for
                preCount[i] = tmp;
            } //end outer for Loop
            addCnt=0;
            mulCnt=0;
            l=l+1;

        } //end while
    } // end else

    cout<<"Need "<<a[0]<<" adder, "<<a[1]<<" multiplier. "<<endl;

}//end list scheduling algorithm
vector<string> Graph::getNodeType(){
    return nodeType;
}
vector<int> Graph::getNodeLevel(){
    return nodeLevel;
}
int *Graph::getArray(){
    return a;
}
int Graph::getNodeSize(){
    return nodeSize;
}

/////////////////Function end of Class Graph///////////////////////
