#include<iostream>
#include<fstream>
#include<vector>

using namespace std;

#define chrXMapFile "chrX_map.txt"
#define refChrXFile "chrX_test.fa"
#define firstColRankFile "FirstCol_actual_rank.txt"
#define ull unsigned long long
const int delta = 10;


streamoff getNewLineOffsetPos(ull, streampos);
// Prog to make a file containing rank (delta = 100) of 1st col

int main(){
    // read file
    fstream FirstCol(chrXMapFile, ios::in | ios::app);
    fstream RefChrX(refChrXFile, ios::in | ios::app);
    fstream RankFirstCol(firstColRankFile, ios::out | ios::trunc);
    streampos refChrXStartPos;

    // check if file has opened
    if(!FirstCol.is_open()){
        cerr<<"| Unable to open "<<chrXMapFile<<".\n";
        return EXIT_FAILURE;
    }
    if(!RefChrX.is_open()){
        cerr<<"| Unable to open "<<refChrXFile<<".\n";
        return EXIT_FAILURE;
    } else {
        // if successful, read the first line which is the file description
        char ch;
        RefChrX.get(ch);
        if (ch != '>') {
            cerr<<"| Wrong file formatting. File description heading missing (>...) in "<<refChrXFile<<".\n";
            return EXIT_FAILURE;
        } else {
            string fileDesc;
            getline(RefChrX, fileDesc);
            cout<<"File metadata ["<<fileDesc<<"]\n";
            refChrXStartPos = RefChrX.tellg();
        }
    }

    string position;
    // int cutoff = 20, iter = 0;

    // 4 ACGT char arrays
    vector<int> A; 
    vector<int> G; 
    vector<int> C; 
    vector<int> T;
    ull A_count = 0 ,C_count = 0 ,G_count = 0 ,T_count = 0;

    // traverse firstCol and for every n*delta index,
    // store rank of that char
    ull counter = 0;
    while (getline(FirstCol, position)) {
        char ch;
        ull pos = stoull(position);
        // cout<<pos<<" ";
        // RefChrX.seekg(refChrXStartPos+pos);
        RefChrX.seekg(getNewLineOffsetPos(pos, refChrXStartPos));
        RefChrX.get(ch);
        RankFirstCol<<ch;
        if (counter%delta==0) {
            // output the sum in respective arrays

        } 
        // cout<<ch<<", ";
        // iter++;
        // if (iter >= cutoff) break;
        counter++;
    }
    cout<<"Parsing finished.\n";

    FirstCol.close();
    if(FirstCol.fail()){
        cerr<<"| Failed to close "<<chrXMapFile<<" file.\n";
    }
    RefChrX.close();
    if(RefChrX.fail()){
        cerr<<"| Failed to close "<<refChrXFile<<" file.\n";
    }
    RankFirstCol.close();
    if(RankFirstCol.fail()){
        cerr<<"| Failed to close "<<firstColRankFile<<" file.\n";
    }
    return EXIT_SUCCESS;
}

streamoff getNewLineOffsetPos(ull pos, streampos s) {
    ull lineNo = (ull)(pos/100);
    ull colNo = pos%100;

    streamoff posInFile = lineNo*101+colNo+s;
    // cout<<"\n pos: "<<pos<<", posInFile:"<<posInFile<<"| ";
    return posInFile;
}