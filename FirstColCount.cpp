#include<iostream>
#include<fstream>
#include<string>

// Program 1

using std::cout; using std::fstream; using std::streamoff; using std::streampos; using std::ios; using std::string; using std::flush;

#define ull unsigned long long

string lastColFilename = "chrX_last_col.txt"; 
string rankFirstColFilename = "FirstCol.txt";

void printAndSave(ull, ull, ull, ull);

int main(int argc, char const *argv[]){
    fstream lastColFile(lastColFilename, ios::in|ios::app);
    char ch;
    ull A_count = 0 ,C_count = 0 ,G_count = 0 ,T_count = 0;

    if(!lastColFile.is_open()){
        cout<<"| Unable to open "<<lastColFilename<<".\n";
        return EXIT_FAILURE;
    }

    cout<<"Parsing...";
    while (lastColFile.get(ch)){
        if (ch == '\n') continue;
        switch(ch){
            case 'A': A_count++; break;
            case 'C': C_count++; break;
            case 'G': G_count++; break;
            case 'T': T_count++; break;
            case '$': break;
        }
    }
    cout<<"\rParsed.\n";
    
    printAndSave(A_count,C_count,G_count,T_count);

    lastColFile.close();
}

void printAndSave(ull A_count, ull C_count, ull G_count, ull T_count){
    fstream rankFirstColFile(rankFirstColFilename, ios::out|ios::trunc);

    if(!rankFirstColFile.is_open()){
        cout<<"| Unable to open "<<"FirstCol.txt"<<" to write into it. Output binned.\n";
        return;
    }

    cout<<"A="<<A_count<<"\nC="<<C_count<<"\nG="<<G_count<<"\nT="<<T_count<<"\nSum="<<A_count+C_count+G_count+T_count<<"\n";
    rankFirstColFile<<A_count<<'\n'<<C_count<<'\n'<<G_count<<'\n'<<T_count<<'\n';

    rankFirstColFile.close();
}
