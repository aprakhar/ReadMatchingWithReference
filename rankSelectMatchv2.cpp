#include<iostream>
#include<string>
#include<fstream>
#include<vector>
#include<sstream>
#include<algorithm>
#include<stdexcept>
#include<chrono>
#include<thread>
#include<cmath>
#include<mutex>

using std::string; using std::cout; using std::fstream; using std::ios; using std::vector; using std::flush; using std::endl; using std::stringstream;
using std::transform; using std::reverse; using std::invalid_argument; using std::chrono::high_resolution_clock; using std::chrono::duration;
using std::chrono::microseconds; using std::chrono::time_point; using std::chrono::system_clock; using std::thread; using std::ref;
using std::for_each; using std::mutex; using std::unique_lock; 

#define DELTA 3
#define ull unsigned long long
#define ll long long

string firstColRankFilename = "FirstCol.txt";                   //36B
string readsFilename = "reads";                                 //312MB
string milestoneMapFilename = "chrX_map_milestone.txt";         //467MB
string lastColBitmapFilename = "chrX_last_col_bitmap.txt";      //529MB
string lastColRankFilename = "last_col_rank.txt";               //1.8GB
string readsMatchPositionFilename = "readsMatchPosition.txt";   // (op file) 722MB

void multiThreadRankSelect(vector<string>&,vector<string>&, vector<vector<ull>>&, vector<int>&, vector<ull>&, fstream&, vector<ll>&);
void rankSelectThread(ull, ull, vector<string>&, vector<string>&, vector<vector<ull>>&, vector<int>&, vector<ull>&, fstream&, vector<ll>&, mutex&);
void getFirstCol(vector<ull>&);
void getACGTLastRank(vector<vector<ull>>&);
void getLastColBitMap(vector<int>&);
void getReadsFile(vector<string>&, vector<string>&);
void getMilestoneMapFile(vector<ll>&);
string cleanupReadString(string);
string makeReversedComplement(string);
char replaceNWithA(char);
char dnaComplement(char);
bool rankSelectInit(string, vector<ull> &, vector<vector<ull>>&, vector<int>&, vector<ull>&, fstream&, vector<ll>&, mutex&);
bool rankSelect(string&, ll&, ll&, vector<vector<ull>>&, vector<int>&, vector<ull>&);
ll charToBitStr(char);
ll getMileStoneRankFromRankDS(char, ll, vector<vector<ull>>&);
ll getBandInFirstColFromRank(char, ll, vector<ull>&);
void getPosInRef(ll, ll, vector<ull>&, vector<vector<ull>>&, vector<ull>&, vector<int>&, string, fstream&, vector<ll>&, mutex&);
char getLastCharAtRow(int);
void outputToFile(bool, bool, vector<ull>&, string&, fstream&, mutex&);

int main(int argc, char const *argv[]){
    fstream readsMatchPositionFile(readsMatchPositionFilename, ios::out|ios::trunc);
    if(!readsMatchPositionFile.is_open()){
        cout<<"| Unable to open "<<readsMatchPositionFilename<<". Cannot save output. Exiting program.\n";
        return EXIT_FAILURE;
    }

    ll readsArraySize;
    vector<thread> memInitThreads;
    vector<string> reads, revComplReads; vector<ll> milestoneMap; vector<ull> firstCol; vector<vector<ull>> ACGT_last_rank; vector<int> lastColBitMap;

    cout<<"Reading files & storing in memory...\n"<<flush;

    /* 
        Multithread read small files
        Sequential read big files  
    */
    memInitThreads.push_back(thread(getFirstCol, ref(firstCol)));
    memInitThreads.push_back(thread(getReadsFile, ref(reads), ref(revComplReads)));
    memInitThreads.push_back(thread(getMilestoneMapFile, ref(milestoneMap)));
    getACGTLastRank(ACGT_last_rank);
    getLastColBitMap(lastColBitMap);
    for_each(memInitThreads.begin(), memInitThreads.end(), [](auto &thr) { thr.join();} ); memInitThreads.clear();
    cout<<"Read into memory completed.\n"<<flush;

    cout<<"Processing read strings.\n";
    multiThreadRankSelect(reads, revComplReads, ACGT_last_rank, lastColBitMap, firstCol,readsMatchPositionFile, milestoneMap);

    if(readsMatchPositionFile.is_open())  readsMatchPositionFile.close();
}

void multiThreadRankSelect(vector<string>& reads, vector<string>& revComplReads, vector<vector<ull>>& ACGT_last_rank, vector<int>& lastColBitMap, vector<ull>& firstCol, fstream& readsMatchPositionFile, vector<ll>& milestoneMap) {
    vector<thread> readsMatchingThreads;
    mutex readsMatchOutputMutex;
    ull noOfCpuThreadsOnSystem = thread::hardware_concurrency()*5;
    ull start, end, iter = 0, noOfReadsPerThread = static_cast<ull>(ceil(reads.size() /  noOfCpuThreadsOnSystem));
    for (ull i = 0; i < noOfCpuThreadsOnSystem; i++){
        start = iter;
        end = iter + noOfReadsPerThread > reads.size() ? reads.size() : iter + noOfReadsPerThread;
        iter = end;
        readsMatchingThreads.push_back(thread(rankSelectThread, start, end, ref(reads), ref(revComplReads), ref(ACGT_last_rank), ref(lastColBitMap), ref(firstCol),ref(readsMatchPositionFile), ref(milestoneMap), ref(readsMatchOutputMutex)));
    }
    for_each(readsMatchingThreads.begin(), readsMatchingThreads.end(), [](auto &thr) { thr.join();} ); readsMatchingThreads.clear();
}

void rankSelectThread(ull start, ull end, vector<string>& reads, vector<string>& revComplReads, vector<vector<ull>>& ACGT_last_rank, vector<int>& lastColBitMap, vector<ull>& firstCol, fstream& readsMatchPositionFile, vector<ll>& milestoneMap, mutex& readsMatchOutputMutex) {
    bool readMatch, revCompReadMatch;
    vector<ull> matchPositions;
    for (ll i = start; i < end; i++){
        readMatch = rankSelectInit(reads[i], matchPositions, ACGT_last_rank, lastColBitMap, firstCol,readsMatchPositionFile, milestoneMap, readsMatchOutputMutex);  
        revCompReadMatch = rankSelectInit(revComplReads[i], matchPositions, ACGT_last_rank, lastColBitMap, firstCol,readsMatchPositionFile, milestoneMap, readsMatchOutputMutex);
        outputToFile(readMatch, revCompReadMatch, matchPositions, reads[i], readsMatchPositionFile, readsMatchOutputMutex);
        matchPositions.clear();  
    }
}

bool rankSelectInit(string read, vector<ull> &matchPositions, vector<vector<ull>>& ACGT_last_rank, vector<int>& lastColBitMap, vector<ull>& firstCol, fstream& readsMatchPositionFile, vector<ll>& milestoneMap, mutex& readsMatchOutputMutex) {
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
    found = rankSelect(read, band_start, band_end, ACGT_last_rank, lastColBitMap, firstCol);

    if (found)
        getPosInRef(band_start, band_end, matchPositions, ACGT_last_rank, firstCol, lastColBitMap, readCopy, readsMatchPositionFile, milestoneMap, readsMatchOutputMutex);
  
    return found;
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

    return rankSelect(read, band_start, band_end, ACGT_last_rank, lastColBitMap, firstCol);
}

void getFirstCol(vector<ull>& firstCol) {
    fstream firstColRankFile(firstColRankFilename, ios::in);
    string readStr;

    while(getline(firstColRankFile, readStr))
        firstCol.push_back(stoull(readStr));

    if(firstColRankFile.is_open())  firstColRankFile.close();
}

void getACGTLastRank(vector<vector<ull>>& ACGT_last_rank){
    fstream lastColRankFile(lastColRankFilename, ios::in);
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
}

void getLastColBitMap(vector<int>& lastColBitMap) {
    fstream lastColBitmapFile(lastColBitmapFilename, ios::in);
    string readStr;
    while(getline(lastColBitmapFile, readStr))
        lastColBitMap.push_back(stoull(readStr));

    if(lastColBitmapFile.is_open())  lastColBitmapFile.close();
}

void getReadsFile(vector<string>& reads, vector<string>& revComplReads) {
    fstream readsFile(readsFilename, ios::in);
    string read, complementReversed;
    while(getline(readsFile, read)){
        read = cleanupReadString(read);
        complementReversed = makeReversedComplement(read);
        reads.push_back(read);
        revComplReads.push_back(complementReversed);
    }
    if(readsFile.is_open())  readsFile.close();
}

void getMilestoneMapFile(vector<ll>& milestoneMap) {
    fstream milestoneMapFile(milestoneMapFilename, ios::in);
    string read;
    while(getline(milestoneMapFile, read))
        milestoneMap.push_back(stoll(read));

    if(milestoneMapFile.is_open())  milestoneMapFile.close();
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

void getPosInRef(ll band_start, ll band_end, vector<ull> &matchPositions, vector<vector<ull>>& ACGT_last_rank, vector<ull>& firstCol, vector<int>& lastColBitMap, string read, fstream& readsMatchPositionFile, vector<ll>& milestoneMap, mutex& readsMatchOutputMutex) {
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
            for (ll i = indexOfNearestMilestoneFromBandStartInBitmap; i < band; i++)
                if(lastColBitMap[i] == charToBitStr(lastChInBand)) countOfChFromStartMilestoneToJustBeforeFirstCh++; // counts until just before band char (see definition of rank).
            rank_ch = getMileStoneRankFromRankDS(lastChInBand, indexOfNearestMilestoneFromBandStartInRankDS, ACGT_last_rank) + countOfChFromStartMilestoneToJustBeforeFirstCh;
            band = getBandInFirstColFromRank(lastChInBand, rank_ch, firstCol);
        }
        startIndexInRef = milestoneMap[(ll)(band/DELTA)]+count_jumps;
        matchPositions.push_back(startIndexInRef);
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

void outputToFile(bool readMatch, bool revCompReadMatch, vector<ull>& matchPositions, string& read, fstream& readsMatchPositionFile, mutex& readsMatchOutputMutex){
/*
    csv formatted output blueprint
    str    readMatch    revCompReadMatch    matchCount    matchPos1    matchPos2...
    ACGT   1            1                       4           1234            5678...
    GGGGT  0            1                       1           34567           
    TTCA   0            0                       0
    ...  
*/
    unique_lock<mutex> ul(readsMatchOutputMutex);
    readsMatchPositionFile<<readMatch<<","<<revCompReadMatch<<","<<matchPositions.size()<<","<<read<<",";
    for (ull j = 0; j < matchPositions.size(); j++)
        readsMatchPositionFile<<matchPositions[j]<<",";
    readsMatchPositionFile<<"\n"<<flush;
    ul.unlock();
}
