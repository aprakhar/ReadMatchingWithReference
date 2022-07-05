#include<iostream>
#include<fstream>
#include<vector>

using std::string;
using std::vector;
using std::ios;
using std::cout;
using std::fstream;

#define ull unsigned long long
#define DELTA 4

void printVector(char, vector<int>);
void printVectors(vector<int>,vector<int>,vector<int>,vector<int>);


int main() {
    fstream lastCol("chrX_last_col_test.txt", ios::in | ios::app);
    fstream lastColRank("last_col_rank.txt", ios::out | ios::trunc);

    // 4 ACGT char arrays
    vector<int> A; 
    vector<int> G; 
    vector<int> C; 
    vector<int> T;
    ull A_count = 0 ,C_count = 0 ,G_count = 0 ,T_count = 0;

    // traverse firstCol and for every 100th index,
    // store rank of that char
    ull counter = 0;
    char chrLastCol;
    while (lastCol.get(chrLastCol)) {
        if (chrLastCol=='\n') continue;
        if (counter%DELTA==0) {
            // output the sum in respective arrays
            A.push_back(A_count);
            C.push_back(C_count);
            G.push_back(G_count);
            T.push_back(T_count);
            lastColRank<<A_count<<','<<C_count<<','<<G_count<<','<<T_count<<','<<'\n';
        }
        switch(chrLastCol){
            case 'A': A_count++;break;
            case 'C': C_count++;break;
            case 'G': G_count++;break;
            case 'T': T_count++;break;
            case '$': break;
            default: exit(EXIT_FAILURE);
        }

        // no need to push_back again coz the remaining chars are already counted from milestone prev to it.
        counter++;
    }
    cout<<"Parsing finished.\n";
    
    // printVectors(A, C, G, T);

    lastCol.close();
    if(lastCol.fail()){
        cout<<"| Failed to close "<<"chrX_last_col.txt"<<" file.\n";
    }
    lastColRank.close();
    if(lastColRank.fail()){
        cout<<"| Failed to close "<<"last_col_rank.txt"<<" file.\n";
    }
    return EXIT_SUCCESS;
}

void printVectors(vector<int> A,vector<int> C,vector<int> G,vector<int> T){
    printVector('A', A);
    printVector('C', C);
    printVector('G', G);
    printVector('T', T);
}

void printVector(char VectorName, vector<int> R){
    cout<<"Array of "<<VectorName<<"\n";
    for (ull i = 0; i < R.size(); i++){
        cout<<R[i]<<" ";
    }
    cout<<"\n";
}