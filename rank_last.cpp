#include<iostream>
#include<fstream>
#include<vector>

using std::string; using std::vector; using std::ios; using std::cout; using std::fstream;

#define ull unsigned long long
#define DELTA 4

void printVector(char, vector<int>);
void printVectors(vector<int>,vector<int>,vector<int>,vector<int>);


int main(int argc, char** argv){
    fstream lastCol("chrX_last_col_test.txt", ios::in | ios::app);
    fstream lastColRank("last_col_rank.txt", ios::out | ios::trunc);

    vector<int> A; vector<int> G; vector<int> C; vector<int> T;
    ull A_count = 0 ,C_count = 0 ,G_count = 0 ,T_count = 0;

    ull counter = 0;
    char chrLastCol;

    while (lastCol.get(chrLastCol)) {
        if (chrLastCol=='\n') continue;
        if (counter%DELTA==0) {
            A.push_back(A_count); C.push_back(C_count); G.push_back(G_count); T.push_back(T_count);
            lastColRank<<A_count<<','<<C_count<<','<<G_count<<','<<T_count<<','<<'\n';
        }
        switch(chrLastCol){
            case 'A': A_count++; break;
            case 'C': C_count++; break;
            case 'G': G_count++; break;
            case 'T': T_count++; break;
            case '$': break;
            default: exit(EXIT_FAILURE);
        }
        counter++;
    }
    cout<<"Parsing finished.\n";

    lastCol.close();
    lastColRank.close();
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
