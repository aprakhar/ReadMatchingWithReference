#include<iostream>
#include<fstream>
#include<vector>
#include<string>

using std::cout;
using std::fstream;
using std::vector;
using std::ios;
using std::string;
using std::flush;

#define ull unsigned long long

int main(int argc, char** argv){
    // vector<int> ACGTBitmap; 
    // const int A_mask = 1000, C_mask = 0100, G_mask = 0010, T_mask = 0001;
    fstream chrXLastColFile("chrX_last_col.txt", ios::in | ios::app);
    fstream lastColBitmap("chrX_last_col_bitmap.txt", ios::out | ios::trunc);

    ull A_count = 0 ,C_count = 0 ,G_count = 0 ,T_count = 0;
    char bwtLastColChar;

    if(!chrXLastColFile.is_open()){
        cout<<"| Unable to open "<<"chrX_last_col.txt"<<".\n";
        return EXIT_FAILURE;
    }

    cout<<"Parsing..."<<flush;
    while (chrXLastColFile.get(bwtLastColChar)) {
        if (bwtLastColChar == '\n') continue;
        switch(bwtLastColChar){
            case 'A': lastColBitmap<<"1000\n"; break;
            case 'C': lastColBitmap<<"0100\n"; break;
            case 'G': lastColBitmap<<"0010\n"; break;
            case 'T': lastColBitmap<<"0001\n"; break;
        }
    }
    cout<<"\rParsing finished.\n";

    chrXLastColFile.close();
    if(chrXLastColFile.fail()){
        cout<<"| Failed to close "<<"chrX_last_col.txt"<<" file.\n";
    }
    lastColBitmap.close();
    if(lastColBitmap.fail()){
        cout<<"| Failed to close "<<"last_col_rank_bitmap.txt"<<" file.\n";
    }

    // printVector(ACGTBitmap);
    return EXIT_SUCCESS;
}
