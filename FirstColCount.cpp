#include<iostream>
#include<fstream>
#include<string>

using std::cout;using std::fstream;using std::streamoff;using std::streampos;using std::ios;using std::string;using std::flush;

#define ull unsigned long long

<<<<<<< HEAD
// string lastColfilename = "test/testCase1/chrX_last_col_test.txt"; 
string lastColfilename = "chrX_last_col.txt"; 
=======
string lastColfilename = "test/testCase1/chrX_last_col_test.txt"; 
>>>>>>> 4aa3d04 (Refactored logic to be such that $ has lowest priority)

void printAndSave(ull, ull, ull, ull);

int main(int argc, char const *argv[]){
    fstream LastCol(lastColfilename, ios::in | ios::app);
    char ch;
    ull A_count = 0 ,C_count = 0 ,G_count = 0 ,T_count = 0;

    if(!LastCol.is_open()){
        cout<<"| Unable to open "<<lastColfilename<<".\n";
        return EXIT_FAILURE;
    }

    cout<<"Starting parse."<<flush;
    while (LastCol.get(ch)){
        if (ch == '\n') continue;
        switch(ch){
            case 'A': A_count++;break;
            case 'C': C_count++;break;
            case 'G': G_count++;break;
            case 'T': T_count++;break;
            case '$': break;
        }
    }
    cout<<"\rParsing finished.\n";
    
    printAndSave(A_count,C_count,G_count,T_count);

    LastCol.close();
}

void printAndSave(ull A_count, ull C_count, ull G_count, ull T_count){
    fstream RankFirstCol("FirstCol.txt", ios::out | ios::trunc);

    if(!RankFirstCol.is_open()){
        cout<<"| Unable to open "<<"FirstCol.txt"<<" to write into it. Output binned.\n";
        return;
    }

    cout<<"A="<<A_count<<"\n";
    cout<<"C="<<C_count<<"\n";
    cout<<"G="<<G_count<<"\n";
    cout<<"T="<<T_count<<"\n";
    cout<<"Sum="<<A_count+C_count+G_count+T_count<<"\n";
    RankFirstCol<<A_count<<'\n';
    RankFirstCol<<C_count<<'\n';
    RankFirstCol<<G_count<<'\n';
    RankFirstCol<<T_count<<'\n';

    RankFirstCol.close();
}


// #define chrXMapFile "chrX_map.txt"
// #define refChrXFile "chrX.fa"
// #define firstColRankFile "FirstCol.txt"
// #define ull unsigned long long

// streamoff getNewLineOffsetPos(ull, streampos);
// void printAndSave(ull, ull, ull, ull);
// streampos getNoOfCharsInFile(fstream&);

// int main(int argc, char **argv){
//     fstream FirstCol(chrXMapFile, ios::in | ios::app);
//     fstream RefChrX(refChrXFile, ios::in | ios::app);
//     streampos refChrXStartPos,refChrXChars = getNoOfCharsInFile(RefChrX);

//     if(!FirstCol.is_open()){
//         cout<<"| Unable to open "<<chrXMapFile<<".\n";
//         return EXIT_FAILURE;
//     }
//     if(!RefChrX.is_open()){
//         cout<<"| Unable to open "<<refChrXFile<<".\n";
//         return EXIT_FAILURE;
//     } else {
//         char ch;
//         RefChrX.get(ch);
//         if (ch != '>') {
//             cout<<"| Wrong file formatting. File description heading missing (>...) in "<<refChrXFile<<".\n";
//             return EXIT_FAILURE;
//         }
//          else {
//             string fileDesc;
//             getline(RefChrX, fileDesc);
//             refChrXStartPos = RefChrX.tellg();
//         }
//     }

//     string position;
//     ull cutoff = 20, iter = 0;
//     ull A_count = 0 ,C_count = 0 ,G_count = 0 ,T_count = 0;

//     // ull counter = 0;
//     cout<<"Starting parse.\n";
//     while (getline(FirstCol, position)) {
//         char ch;
//         ull pos = stoull(position);
//         RefChrX.seekg(getNewLineOffsetPos(pos, refChrXStartPos));
//         RefChrX.get(ch);
//         switch(ch){
//             case 'A': A_count++;break;
//             case 'C': C_count++;break;
//             case 'G': G_count++;break;
//             case 'T': T_count++;break;
//             case '$': break;
//         }
//         iter++;
//         // if (iter >= cutoff) break;
//         // counter++;
//         if (!(iter%100000)) cout<<((float)iter/refChrXChars)*100<<"%\r";

//     }
//     cout<<"\nParsing finished.\n";

//     printAndSave(A_count,C_count,G_count,T_count);
//     // cout<<"A="<<A_count<<"\n";
//     // cout<<"C="<<C_count<<"\n";
//     // cout<<"G="<<G_count<<"\n";
//     // cout<<"T="<<T_count<<"\n";
//     // cout<<"Sum="<<A_count+C_count+G_count+T_count<<"\n";
//     // RankFirstCol<<A_count<<'\n';
//     // RankFirstCol<<C_count<<'\n';
//     // RankFirstCol<<G_count<<'\n';
//     // RankFirstCol<<T_count<<'\n';

//     FirstCol.close();
//     if(FirstCol.fail()){
//         cout<<"| Failed to close "<<chrXMapFile<<" file.\n";
//     }
//     RefChrX.close();
//     if(RefChrX.fail()){
//         cout<<"| Failed to close "<<refChrXFile<<" file.\n";
//     }
//     // RankFirstCol.close();
//     // if(RankFirstCol.fail()){
//     //     cout<<"| Failed to close "<<firstColRankFile<<" file.\n";
//     // }
//     // return EXIT_SUCCESS;
// }

// streamoff getNewLineOffsetPos(ull pos, streampos s) {
//     ull lineNo = (ull)(pos/100);
//     ull colNo = pos%100;

//     streamoff posInFile = lineNo*101+colNo+s;
//     return posInFile;
// }

// void printAndSave(ull A_count, ull C_count, ull G_count, ull T_count){
//     fstream RankFirstCol(firstColRankFile, ios::out | ios::trunc);

//     cout<<"A="<<A_count<<"\n";
//     cout<<"C="<<C_count<<"\n";
//     cout<<"G="<<G_count<<"\n";
//     cout<<"T="<<T_count<<"\n";
//     cout<<"Sum="<<A_count+C_count+G_count+T_count<<"\n";
//     RankFirstCol<<A_count<<'\n';
//     RankFirstCol<<C_count<<'\n';
//     RankFirstCol<<G_count<<'\n';
//     RankFirstCol<<T_count<<'\n';

//     RankFirstCol.close();
//     if(RankFirstCol.fail()){
//         cout<<"| Failed to close "<<firstColRankFile<<" file.\n";
//     }
// }

// streampos getNoOfCharsInFile(fstream& FileObj){
//     streampos begin, end, current;

//     current=FileObj.tellg();

//     FileObj.seekg(0,ios::end);
//     end=FileObj.tellg();

//     FileObj.seekg(0,ios::beg);
//     begin=FileObj.tellg();

//     FileObj.seekg(current);

//     return end-begin;
// }
