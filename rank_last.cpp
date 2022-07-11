#include<iostream>
#include<fstream>
#include<vector>

// Program 2

using std::string; using std::vector; using std::ios; using std::cout; using std::fstream; using std::flush;

#define ull unsigned long long
#define DELTA 3

string lastColFilename = "chrX_last_col.txt";
string lastColRankFilename = "last_col_rank.txt";

void printVector(char, vector<int>);
void printVectors(vector<int>, vector<int>, vector<int>, vector<int>);

int main(int argc, char const *argv[]){
    fstream lastColFile(lastColFilename, ios::in|ios::app);
    fstream lastColRankFile(lastColRankFilename, ios::out|ios::trunc);

    if(!lastColFile.is_open()){
        cout<<"| Unable to open "<<lastColFilename<<".\n";
        return EXIT_FAILURE;
    }
    if(!lastColRankFile.is_open()){
        cout<<"| Unable to open "<<"last_col_rank.txt"<<".\n";
        return EXIT_FAILURE;
    }

    vector<int> A; vector<int> G; vector<int> C; vector<int> T;
    ull A_count = 0 ,C_count = 0 ,G_count = 0 ,T_count = 0, $_count = 0, $_pos;
    ull counter = 0;
    char chrLastCol;
    
    cout<<"Parsing..."<<flush;
    while (lastColFile.get(chrLastCol)) {
        if (chrLastCol=='\n') continue;
        if (counter%DELTA==0) {
            A.push_back(A_count); C.push_back(C_count); G.push_back(G_count); T.push_back(T_count);
            lastColRankFile<<A_count<<','<<C_count<<','<<G_count<<','<<T_count<<','<<'\n'; //TODO add $ milestones also
        }
        switch(chrLastCol){
            case 'A': A_count++; break;
            case 'C': C_count++; break;
            case 'G': G_count++; break;
            case 'T': T_count++; break;
            case '$': $_count++; $_pos = A_count + G_count + C_count + T_count; break;
            default: throw std::invalid_argument("Wrong character "+std::to_string(chrLastCol)+" read in main in "+argv[0]+".\n");
        }
        counter++;
    }
    if ($_count != 1) {
        if(remove(lastColRankFilename.c_str()) != 0 )
            perror(("Error deleting "+lastColRankFilename+" file").c_str());
        throw std::invalid_argument("Wrong character "+std::to_string(chrLastCol)+" read in main in "+argv[0]+".\n");
    }
    lastColRankFile<<"$ position:"<<$_pos<<"\n";
    cout<<"\rParsed.\n";

    lastColFile.close();
    lastColRankFile.close();
}
