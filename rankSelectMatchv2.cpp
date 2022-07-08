#include<iostream>
#include<string>
#include<fstream>
#include<vector>
#include<sstream>
#include<algorithm>
#include<stdexcept>
#include<chrono>

using std::string; using std::cout; using std::fstream; using std::ios; using std::vector; using std::flush; using std::endl; using std::stringstream;
using std::transform; using std::reverse; using std::invalid_argument; using std::chrono::high_resolution_clock; using std::chrono::duration_cast;
using std::chrono::microseconds;

#define DELTA 3
#define ull unsigned long long
#define ll long long

string readsFilename = "reads";
// string readsFilename = "test/testCase1/reads_test";  
string milestoneMapFilename = "chrX_map_milestone.txt";
string readsMatchPositionFilename = "readsMatchPosition.txt";

vector<ull> getFirstCol();
vector<vector<ull>> getACGTLastRank();
vector<int> getLastColBitMap();
string cleanupReadString(string);
string makeReversedComplement(string);
char replaceNWithA(char);
char dnaComplement(char);
void rankSelectInit(string, vector<vector<ull>>&, vector<int>&, vector<ull>&, fstream&, vector<ll>&);
bool rankSelect(string&, ll&, ll&, vector<vector<ull>>&, vector<int>&, vector<ull>&);
ll charToBitStr(char);
ll getMileStoneRankFromRankDS(char, ll, vector<vector<ull>>&);
ll getBandInFirstColFromRank(char, ll, vector<ull>&);
void getPosInRef(ll, ll, vector<vector<ull>>&, vector<ull>&, vector<int>&, string, fstream&, vector<ll>&);
char getLastCharAtRow(int);

int main(int argc, char const *argv[]){
    fstream readsFile(readsFilename, ios::in);
    fstream readsMatchPositionFile(readsMatchPositionFilename, ios::out|ios::trunc);
    fstream milestoneMapFile(milestoneMapFilename, ios::in);

    if(!readsFile.is_open()){
        cout<<"| Unable to open "<<readsFilename<<".\n";
        return EXIT_FAILURE;
    }
    if(!readsMatchPositionFile.is_open()){
        cout<<"| Unable to open "<<readsMatchPositionFilename<<". Cannot save output. Exiting program.\n";
        return EXIT_FAILURE;
    }
    if(!milestoneMapFile.is_open()){
        cout<<"| Unable to open "<<milestoneMapFilename<<".\n";
        exit(EXIT_FAILURE);
    }

    vector<string> readsAndRevComplReads; vector<ll> milestoneMap; vector<ull> firstCol; vector<vector<ull>> ACGT_last_rank; vector<int> lastColBitMap;
    string read, complementReversed;
    ll readsArraySize;

    cout<<"Reading files & storing in memory...\n"<<flush;

    firstCol = getFirstCol(); ACGT_last_rank = getACGTLastRank(); lastColBitMap = getLastColBitMap();

    while(getline(readsFile, read)){
        read = cleanupReadString(read);
        complementReversed = makeReversedComplement(read);
        readsAndRevComplReads.push_back(read);
        readsAndRevComplReads.push_back(complementReversed);
    }

    while(getline(milestoneMapFile, read)){
        milestoneMap.push_back(stoll(read));
    }
    cout<<"Read into memory completed.\n";

    cout<<"Processing read strings.\n";
    readsArraySize = readsAndRevComplReads.size();

    for (ll i = 0; i < readsArraySize; i++){
        if (!(i%100)) cout<<"\rProgress: "<<(double)i*100/readsArraySize<<"%"<<flush;
        rankSelectInit(readsAndRevComplReads[i], ACGT_last_rank, lastColBitMap, firstCol,readsMatchPositionFile, milestoneMap);  
    }
    cout<<"\rProgress: 100%"<<flush;
}

void rankSelectInit(string read, vector<vector<ull>>& ACGT_last_rank, vector<int>& lastColBitMap, vector<ull>& firstCol, fstream& readsMatchPositionFile, vector<ll>& milestoneMap) {
    string readCopy = read;
    ll band_start, band_end;
    ll readStrLen = read.size();  
    char lastChar = read.back();
    bool found = false;

    read.pop_back();

    /*
        A   A   C   C   G   G   T
        0   1   2   3   4   5   6
        firstCol[A] = 2
        firstCol[C] = 2
        firstCol[G] = 2
        firstCol[T] = 1
    */
    ll A_start, C_start, G_start, T_start, A_end, C_end, G_end, T_end;
    A_start = 0; C_start = A_start + firstCol[0]; G_start = C_start + firstCol[1]; T_start = G_start + firstCol[2];
    A_end = firstCol[0] -1; C_end = A_end + firstCol[1]; G_end = C_end + firstCol[2]; T_end = G_end + firstCol[3];

    switch(lastChar) {
        case 'A': band_start = A_start; band_end = A_end; break;
        case 'C': band_start = C_start; band_end = C_end; break;
        case 'G': band_start = G_start; band_end = G_end; break;
        case 'T': band_start = T_start; band_end = T_end; break;
        default: cout<<"Invalid string. Found unknown character: "<<lastChar<<".\n"; exit(EXIT_FAILURE);
    }
    // auto start_rankSelect = high_resolution_clock::now();
    found = rankSelect(read, band_start, band_end, ACGT_last_rank, lastColBitMap, firstCol);
    // auto stop_rankSelect = high_resolution_clock::now();

    // auto start_fileWrite = high_resolution_clock::now();
    if (found) {
        getPosInRef(band_start, band_end,ACGT_last_rank, firstCol, lastColBitMap, readCopy, readsMatchPositionFile, milestoneMap);
    } else readsMatchPositionFile<<"Not found. ["<<readCopy<<"]\n";
    // auto stop_fileWrite = high_resolution_clock::now();

    // cout<<"Time track for "<<readCopy<<"\n"
    //     <<"\tRank finding time:"<<(duration_cast<microseconds>(stop_rankSelect - start_rankSelect).count())<<"\n"
    //     <<"\tFile writing finding time:"<<(duration_cast<microseconds>(stop_fileWrite - start_fileWrite).count())<<"\n";
}

bool rankSelect(string& read, ll& band_start, ll& band_end, vector<vector<ull>>& ACGT_last_rank, vector<int>& lastColBitMap, vector<ull>& firstCol){
    if (read.size() == 0) return true;
    if (band_start > band_end) return false; // probably never reached

    char ch = read.back();
    read.pop_back();

    /*
        We have the band_start & band_end.
        The read string's last char is last char to be found in last col,
        i.e., we need the sub-band within this band 
    */
    ll rank_first_ch, rank_last_ch, indexOfNearestMilestoneFromBandStartInBitmap, indexOfNearestMilestoneFromBandStartInRankDS,
        indexOfNearestMilestoneFromBandEndInBitmap, indexOfNearestMilestoneFromBandEndInRankDS, rankOfNearestMilestoneFromStart,
        rankOfNearestMilestoneFromEnd, countOfChFromStartMilestoneToJustBeforeFirstCh = 0, countOfChFromEndMilestoneToLastCh = 0
    ;
    indexOfNearestMilestoneFromBandStartInRankDS = band_start/DELTA; // index of nearest milstone in rank DS (rankDS = rank data structure)
    indexOfNearestMilestoneFromBandEndInRankDS = band_end/DELTA; // index of nearest milstone in rank DS (rankDS = rank data structure)
    indexOfNearestMilestoneFromBandStartInBitmap = band_start - (band_start%DELTA); // index of nearest milstone in bitmap array
    indexOfNearestMilestoneFromBandEndInBitmap = band_end - (band_end%DELTA); // index of nearest milstone in bitmap array
    rankOfNearestMilestoneFromStart = getMileStoneRankFromRankDS(ch, indexOfNearestMilestoneFromBandStartInRankDS, ACGT_last_rank);
    rankOfNearestMilestoneFromEnd = getMileStoneRankFromRankDS(ch, indexOfNearestMilestoneFromBandEndInRankDS, ACGT_last_rank);

    // get number of ch in between indexOfNearestMilestoneFromBandStartInBitmap to the first occurence of ch in the band (indexOfNearestMilestoneFromBandStartInBitmap inclusive only)
    ll iter = indexOfNearestMilestoneFromBandStartInBitmap;
    while(iter < band_end) {
        if (iter >= band_start && lastColBitMap[iter] == charToBitStr(ch)) break;
        if (lastColBitMap[iter] == charToBitStr(ch)) countOfChFromStartMilestoneToJustBeforeFirstCh++;
        iter++;
    }
    iter = indexOfNearestMilestoneFromBandEndInBitmap;
    while(iter <= band_end) {
        if (lastColBitMap[iter] == charToBitStr(ch)) countOfChFromEndMilestoneToLastCh++; // last ch count --if here-- will be removed later
        iter++;
    }

    ll noOfChInBand = (rankOfNearestMilestoneFromEnd + countOfChFromEndMilestoneToLastCh) - (rankOfNearestMilestoneFromStart + countOfChFromStartMilestoneToJustBeforeFirstCh);
    if (noOfChInBand == 0) return false;

    rank_first_ch = rankOfNearestMilestoneFromStart + countOfChFromStartMilestoneToJustBeforeFirstCh;
    rank_last_ch = rankOfNearestMilestoneFromEnd + countOfChFromEndMilestoneToLastCh -1; // to exclude the last ch counted. (reason: see definition of rank)

    /* 
        take the ranks found in last column to first column
        and locate the band start & end for the ranks first 
        & last, respectively
    */
    band_start = getBandInFirstColFromRank(ch, rank_first_ch, firstCol);
    band_end = getBandInFirstColFromRank(ch, rank_last_ch, firstCol);

    // cout<<"Time track for "<<read<<"\n"
    //     // <<"\tRank finding time:"<<(duration_cast<microseconds>(stop_rankSelect - start_rankSelect).count())<<"\n"
    //     <<"\tFile writing finding time:"<<(duration_cast<microseconds>(stop_getBand - start_getBand).count())<<"\n";

    return rankSelect(read, band_start, band_end, ACGT_last_rank, lastColBitMap, firstCol);
}

vector<ull> getFirstCol() {
    vector<ull> firstCol;
    fstream firstColRankFile("FirstCol.txt", ios::in);
    string readStr;

    while(getline(firstColRankFile, readStr)){
        firstCol.push_back(stoull(readStr));
    }

    if(firstColRankFile.is_open())  firstColRankFile.close();

    return firstCol;
}

vector<vector<ull>> getACGTLastRank(){
    vector<vector<ull>> ACGT_last_rank;
    fstream lastColRankFile("last_col_rank.txt", ios::in);
    string readStr;

    while(getline(lastColRankFile, readStr)){
        string rank;
        stringstream ss(readStr);
        vector<ull> rank_row;
        for (int i = 0; i < 4; i++){
            getline(ss, rank, ',');
            rank_row.push_back(stoull(rank));
        }
        ACGT_last_rank.push_back(rank_row);
    }

    if(lastColRankFile.is_open())  lastColRankFile.close();

    return ACGT_last_rank;
}

vector<int> getLastColBitMap() {
    vector<int> lastColBitMap;
    fstream lastColBitmapFile("chrX_last_col_bitmap.txt", ios::in);
    string readStr;
    while(getline(lastColBitmapFile, readStr)){
        lastColBitMap.push_back(stoull(readStr));
    }
    if(lastColBitmapFile.is_open())  lastColBitmapFile.close();
    return lastColBitMap;
}

char replaceNWithA(char ch) { return ch == 'N' ? 'A' : ch; } 

string cleanupReadString(string str){
    transform(str.begin(), str.end(), str.begin(), replaceNWithA);
    return str;
}

char dnaComplement(char ch){
    switch (ch){
        case 'A': return 'T';
        case 'C': return 'G';
        case 'G': return 'C';
        case 'T': return 'A';
    }
    throw invalid_argument("Unknown character: " + ch);
}

string makeReversedComplement(string str) {
    string reversedComplement(str.length(), 'X');
    transform(str.begin(), str.end(), reversedComplement.begin(), dnaComplement);
    reverse(reversedComplement.begin(), reversedComplement.end());
    return reversedComplement;
}

ll charToBitStr(char ch) {
    switch (ch){
        case 'A': return 1000;
        case 'C': return 100;
        case 'G': return 10;
        case 'T': return 1;
        default: cout<<"Unknown char found. (ch="<<ch<<").\nProgram terminated\n"; exit(EXIT_FAILURE);
    }

    return -1;
}

ll getMileStoneRankFromRankDS(char ch, ll index, vector<vector<ull>>& ACGT_last_rank) {
    switch (ch){
        case 'A': return ACGT_last_rank[index][0];
        case 'C': return ACGT_last_rank[index][1];
        case 'G': return ACGT_last_rank[index][2];
        case 'T': return ACGT_last_rank[index][3];
        default: cout<<"Unknown char found. (ch="<<ch<<").\nProgram terminated\n"; exit(EXIT_FAILURE);
    }

    return -1;
}

ll getBandInFirstColFromRank(char ch, ll rank, vector<ull>& firstCol) {
    /*
        rank = 1, ch = C;
        A   A   C   C   G   G   T
        0   1   2   3   4   5   6
    */
    switch (ch){
        case 'A': return rank;
        case 'C': return firstCol[0] + rank;
        case 'G': return firstCol[0] + firstCol[1] + rank;
        case 'T': return firstCol[0] + firstCol[1] + firstCol[2] + rank;
        
        default: cout<<"Unknown char found. (ch="<<ch<<").\nProgram terminated\n"; exit(EXIT_FAILURE);
    }
    return -1;
}

void getPosInRef(ll band_start, ll band_end, vector<vector<ull>>& ACGT_last_rank, vector<ull>& firstCol, vector<int>& lastColBitMap, string read, fstream& readsMatchPositionFile, vector<ll>& milestoneMap) {
    ll startIndexInRef;
    ull count_jumps = 0;

    for (ll band_iter = band_start; band_iter <= band_end; band_iter++){
        ll band = band_iter;
        while(band%DELTA != 0){
            count_jumps++;
            char lastChInBand = getLastCharAtRow(lastColBitMap[band]);
            ll rank_ch, indexOfNearestMilestoneFromBandStartInBitmap, indexOfNearestMilestoneFromBandStartInRankDS, countOfChFromStartMilestoneToJustBeforeFirstCh = 0;

            indexOfNearestMilestoneFromBandStartInRankDS = band/DELTA; // index of nearest milstone in rank DS (rankDS = rank data structure)
            indexOfNearestMilestoneFromBandStartInBitmap = band - (band%DELTA); // index of nearest milstone in bitmap array
            for (ll i = indexOfNearestMilestoneFromBandStartInBitmap; i < band; i++){
                if(lastColBitMap[i] == charToBitStr(lastChInBand)) countOfChFromStartMilestoneToJustBeforeFirstCh++; // counts until just before band char (see definition of rank).
            }
            rank_ch = getMileStoneRankFromRankDS(lastChInBand, indexOfNearestMilestoneFromBandStartInRankDS, ACGT_last_rank) + countOfChFromStartMilestoneToJustBeforeFirstCh;
            band = getBandInFirstColFromRank(lastChInBand, rank_ch, firstCol);
        }
        startIndexInRef = milestoneMap[(ll)(band/DELTA)]+count_jumps;
        readsMatchPositionFile<<startIndexInRef<<" ["<<read<<"]\n"<<flush;
        count_jumps = 0;
    }
}

char getLastCharAtRow(int chAsInt) {
    switch(chAsInt) {
        case 1000: return 'A';
        case 100: return 'C';
        case 10: return 'G';
        case 1: return 'T';
        case 0: return '$';
        default: cout<<"Invalid bitstr from lastColBitMap.\n"; exit(EXIT_FAILURE);
    };
    return 'X';
}
