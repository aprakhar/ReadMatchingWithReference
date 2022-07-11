#include<iostream>
#include<fstream>
#include<vector>
#include<string>

// Program 3

using std::cout; using std::fstream; using std::vector; using std::ios; using std::string; using std::flush;

#define ull unsigned long long
#define ll long long

string lastColFilename = "chrX_last_col.txt"; 
string lastColBitmapFilename = "chrX_last_col_bitmap.txt";

void addCharToBitmap(vector<int>&, char);
void printVector(vector<int>&);

int main(int argc, char** argv){
    fstream lastColFile(lastColFilename, ios::in|ios::app);

    if(!lastColFile.is_open()){
        cout<<"| Unable to open "<<lastColFilename<<".\n";
        return EXIT_FAILURE;
    }
    
    vector<int> ACGTBitmap; 
    const int A_mask = 1000, C_mask = 0100, G_mask = 0010, T_mask = 0001;
    ull A_count = 0 ,C_count = 0 ,G_count = 0 ,T_count = 0;
    char bwtLastColChar;

    cout<<"Parsing..."<<flush;
    while (lastColFile.get(bwtLastColChar)) {
        if (bwtLastColChar == '\n') continue;
        addCharToBitmap(ACGTBitmap, bwtLastColChar);
    }
    cout<<"\rParsed.\n";
    
    printVector(ACGTBitmap);
}

void addCharToBitmap(vector<int>& bitMap, char ch) {
    switch(ch){
        case 'A': bitMap.push_back(1000); break;
        case 'C': bitMap.push_back(100); break;
        case 'G': bitMap.push_back(10); break;
        case 'T': bitMap.push_back(1); break;
        case '$': bitMap.push_back(0); break;
        default: throw std::invalid_argument("Wrong character "+std::to_string(ch)+" read in addCharToBitmap.\n");
    }
}

void printVector(vector<int>& R){
    fstream lastColBitmapFile(lastColBitmapFilename, ios::out|ios::trunc);

    cout<<"Writing to file..."<<flush;
    for (ll i = 0; i < R.size(); i++){
        lastColBitmapFile<<R[i]<<'\n';
    }
    cout<<"\rWritten to file.\n";

    lastColBitmapFile.close();
}
