#include<iostream>
#include<fstream>
#include<bitset>
#include<string>

using std::cout;
using std::fstream;
using std::bitset;
using std::ios;
using std::string;

#define ull unsigned long long
#define CHARS_COUNT 152611567

void addCharToBitmap(bitset<CHARS_COUNT>&,bitset<CHARS_COUNT>&,bitset<CHARS_COUNT>&,bitset<CHARS_COUNT>&, char, ull);
void printVector(const bitset<CHARS_COUNT>, const bitset<CHARS_COUNT>, const bitset<CHARS_COUNT>, const bitset<CHARS_COUNT>);

int main(int argc, char** argv){
    bitset<CHARS_COUNT> A;
    bitset<CHARS_COUNT> C;
    bitset<CHARS_COUNT> G;
    bitset<CHARS_COUNT> T;

    ull iter = 0;

    const int A_mask = 1000, C_mask = 0100, G_mask = 0010, T_mask = 0001;

    fstream chrXLastColFile("chrX_last_col.txt", ios::in | ios::app);

    char bwtLastColChar;

    if(!chrXLastColFile.is_open()){
        cout<<"| Unable to open "<<"chrX_last_col.txt"<<".\n";
        return EXIT_FAILURE;
    }

    cout<<"Parsing...";
    while (chrXLastColFile.get(bwtLastColChar)) {
        if (bwtLastColChar == '\n') continue;
        addCharToBitmap(A, C, G, T, bwtLastColChar, iter);
        iter++;
    }
    cout<<"\rParsing finished.\n";

    printVector(A, C, G, T);

}

void addCharToBitmap(bitset<CHARS_COUNT>& A, bitset<CHARS_COUNT>& C, bitset<CHARS_COUNT>& G, bitset<CHARS_COUNT>& T, char ch, ull index) {
    switch(ch){
        case 'A': A[index] = 1; break;
        case 'C': C[index] = 1; break;
        case 'G': G[index] = 1; break;
        case 'T': T[index] = 1; break;
        // case 'C': bitMap.push_back(0100); break;
        // case 'G': bitMap.push_back(0010); break;
        // case 'T': bitMap.push_back(0001); break;
    }
}

void printVector(const bitset<CHARS_COUNT> A, const bitset<CHARS_COUNT> C, const bitset<CHARS_COUNT> G, const bitset<CHARS_COUNT> T){
    fstream lastColBitmap("chrX_last_col_bitmap.txt", ios::out | ios::trunc);

    cout<<"Writing to file...";

    for (long long i = 0; i < CHARS_COUNT; i++){
        lastColBitmap<<A[i]<<' '<<C[i]<<' '<<G[i]<<' '<<T[i]<<'\n';
    }

    cout<<"\rWriting finished.\n";

    lastColBitmap.close();
    if(lastColBitmap.fail()){
        cout<<"| Failed to close "<<"last_col_rank_bitmap.txt"<<" file.\n";
    }
}
