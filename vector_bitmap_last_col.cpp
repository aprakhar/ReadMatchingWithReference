#include<iostream>
#include<fstream>
#include<vector>
#include<string>

using std::cout;using std::fstream;using std::vector;using std::ios;using std::string;using std::flush;

#define ull unsigned long long

string lastColfilename = "chrX_last_col.txt"; 
// string lastColfilename = "test/testCase1/chrX_last_col_test.txt"; 

void addCharToBitmap(vector<int>&, char);
void printVector(const vector<int>);

int main(int argc, char** argv){
    vector<int> ACGTBitmap; 
    const int A_mask = 1000, C_mask = 0100, G_mask = 0010, T_mask = 0001;
    fstream chrXLastColFile(lastColfilename, ios::in | ios::app);
    ull A_count = 0 ,C_count = 0 ,G_count = 0 ,T_count = 0;
    char bwtLastColChar;

    if(!chrXLastColFile.is_open()){
        cout<<"| Unable to open "<<lastColfilename<<".\n";
        return EXIT_FAILURE;
    }

    cout<<"Parsing..."<<flush;
    while (chrXLastColFile.get(bwtLastColChar)) {
        if (bwtLastColChar == '\n') continue;
        addCharToBitmap(ACGTBitmap, bwtLastColChar);
    }
    cout<<"\rParsing finished.\n";

    printVector(ACGTBitmap);
    return EXIT_SUCCESS;
}

void addCharToBitmap(vector<int>& bitMap, char ch) {
    switch(ch){
        case 'A': bitMap.push_back(1000); break;
        case 'C': bitMap.push_back(100); break;
        case 'G': bitMap.push_back(10); break;
        case 'T': bitMap.push_back(1); break;
        default: bitMap.push_back(0); break;
    }
}

void printVector(const vector<int> R){
    fstream lastColBitmap("chrX_last_col_bitmap.txt", ios::out | ios::trunc);

    cout<<"Writing to file..."<<flush;
    for (long long i = 0; i < R.size(); i++){
        lastColBitmap<<R[i]<<'\n';
    }
    cout<<"\rWriting finished.\n";

    lastColBitmap.close();
    if(lastColBitmap.fail()){
        cout<<"| Failed to close "<<"last_col_rank_bitmap.txt"<<" file.\n";
    }
}
