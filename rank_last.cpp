#include<iostream>
#include<fstream>
#include<vector>

using std::string; using std::vector; using std::ios; using std::cout; using std::fstream; using std::flush;

#define ull unsigned long long
#define DELTA 3

string lastColFilename = "chrX_last_col.txt"; 
// string lastColFilename = "test/testCase1/chrX_last_col_test.txt"; 

void printVector(char, vector<int>);
void printVectors(vector<int>, vector<int>, vector<int>, vector<int>);

int main(int argc, char const *argv[]){
    fstream lastColFile(lastColFilename, ios::in|ios::app);
    fstream lastColRankFile("last_col_rank.txt", ios::out|ios::trunc);

    if(!lastColFile.is_open()){
        cout<<"| Unable to open "<<lastColFilename<<".\n";
        return EXIT_FAILURE;
    }
    if(!lastColRankFile.is_open()){
        cout<<"| Unable to open "<<"last_col_rank.txt"<<".\n";
        return EXIT_FAILURE;
    }

    vector<int> A; vector<int> G; vector<int> C; vector<int> T;
    ull A_count = 0 ,C_count = 0 ,G_count = 0 ,T_count = 0;
    ull counter = 0;
    char chrLastCol;
    
    cout<<"Parsing..."<<flush;
    while (lastColFile.get(chrLastCol)) {
        if (chrLastCol=='\n') continue;
        if (counter%DELTA==0) {
            A.push_back(A_count); C.push_back(C_count); G.push_back(G_count); T.push_back(T_count);
            lastColRankFile<<A_count<<','<<C_count<<','<<G_count<<','<<T_count<<','<<'\n';
        }
        switch(chrLastCol){
            case 'A': A_count++; break;
            case 'C': C_count++; break;
            case 'G': G_count++; break;
            case 'T': T_count++; break;
            case '$': break;
            default: cout<<"Unknown character ("<<chrLastCol<<") read in "<<lastColFilename<<"\n"; exit(EXIT_FAILURE);
        }
        counter++;
    }

    cout<<"\rParsed.\n";
    lastColFile.close();
    lastColRankFile.close();
}
