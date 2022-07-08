#include<iostream>
#include<fstream>
#include<string>

using std::cout; using std::fstream; using std::streamoff; using std::streampos; using std::ios; using std::string; using std::flush;

#define ull unsigned long long

void printAndSave(ull, ull, ull, ull);

int main(int argc, char const *argv[]){
    fstream LastCol("chrX_last_col_test.txt", ios::in | ios::app);
    char ch;
    ull A_count = 0 ,C_count = 0 ,G_count = 0 ,T_count = 0;

    if(!LastCol.is_open()){
        cout<<"| Unable to open "<<"chrX_last_col.txt"<<".\n";
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

    cout<<"A="<<A_count<<"\n"<<"C="<<C_count<<"\n"<<"G="<<G_count<<"\n"<<"T="<<T_count<<"\n";
    cout<<"Sum="<<A_count+C_count+G_count+T_count<<"\n";
    RankFirstCol<<A_count<<'\n'<<C_count<<'\n'<<G_count<<'\n'<<T_count<<'\n';

    RankFirstCol.close();
}
