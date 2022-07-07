#include<iostream>
#include<string>
#include<fstream>
#include<vector>
#include<sstream>
#include<algorithm>
#include<stdexcept>

using std::string; using std::cout; using std::fstream; using std::ios; using std::vector; using std::flush; using std::endl; using std::stringstream;
using std::transform; using std::reverse; using std::invalid_argument;

#define DELTA 3
#define ull unsigned long long
#define ll long long

string readsFilename = "reads";
// string readsFilename = "test/testCase1/reads_test";  
string milestoneMapFilename = "chrX_map_milestone.txt";
string readsMatchIndexFilename = "readsMatchPosition.txt";

vector<ull> getFirstCol();
vector<vector<ull>> getACGTLastRank();
vector<int> getLastColBitMap();
string cleanupReadString(string);
string makeReversedComplement(string);
char replaceNWithA(char);
char dnaComplement(char);
void rankSelectInit(string, vector<vector<ull>>&, vector<int>&, vector<ull>&, fstream&, vector<ll>&);
bool rankSelect(string&, ll&, ll&, vector<vector<ull>>&, vector<int>&, vector<ull>&);
bool noChInBand(char, ll, ll, vector<int>&);
ll charToBitStr(char);
ll getMileStoneRankFromRankDS(char, ll, vector<vector<ull>>&);
ll getBandInFirstColFromRank(char, ll, vector<ull>&);
void getPosInRef(ll, ll, vector<vector<ull>>&, vector<ull>&, vector<int>&, string, fstream&, vector<ll>&);
char getLastCharAtRow(ll,  vector<int>&);
ll getFileSize(fstream&);

int main(int argc, char const *argv[]){
    fstream readsFile(readsFilename, ios::in);
    fstream readsMatchIndexFile(readsMatchIndexFilename, ios::out | ios::trunc);
    string read, complementReversed;
    ll readsFileSize;
    vector<string> readsAndRevComplReads;
    ////////////
    fstream milestoneMapFile(milestoneMapFilename, ios::in);

    if(!milestoneMapFile.is_open()){
        cout<<"| Unable to open "<<milestoneMapFilename<<".\n";
        exit(EXIT_FAILURE);
    }

    vector<ll> milestoneMap;

    while(getline(milestoneMapFile, read)){
        milestoneMap.push_back(stoll(read));
    }
    //////////

    if(!readsFile.is_open()){
        cout<<"| Unable to open "<<readsFilename<<".\n";
        return EXIT_FAILURE;
    }
    if(!readsMatchIndexFile.is_open()){
        cout<<"| Unable to open "<<readsMatchIndexFilename<<". Cannot save output. Exiting program.\n";
        return EXIT_FAILURE;
    }
    
    cout<<"Reading from files and storing in memory...\n";
    vector<ull> firstCol(getFirstCol());
    vector<vector<ull>> ACGT_last_rank(getACGTLastRank());
    vector<int> lastColBitMap(getLastColBitMap());
    readsFileSize = getFileSize(readsFile);
    cout<<"Read into memory completed.\n";

    while(getline(readsFile, read)){
        // cout<<"Found "<<read<<"? ";
        // cout<<"\rProgress: "<<(double)readsFile.tellg()*100/readsFileSize<<"%"<<flush;
        
        read = cleanupReadString(read);
        readsAndRevComplReads.push_back(read);
        complementReversed = makeReversedComplement(read);
        readsAndRevComplReads.push_back(complementReversed);
    }
    cout<<"Reading read strings.\n";
    ll readsArraySize = readsAndRevComplReads.size();
    for (ll i = 0; i < readsArraySize; i++){
        cout<<"\rProgress: "<<(double)i*100/readsArraySize<<"%"<<flush;
        rankSelectInit(readsAndRevComplReads[i], ACGT_last_rank, lastColBitMap, firstCol,readsMatchIndexFile, milestoneMap);  
    }
    
    // while(getline(readsFile, read)){
    //     // cout<<"Found "<<read<<"? ";
    //     cout<<"\rProgress: "<<(double)readsFile.tellg()*100/readsFileSize<<"%"<<flush;
        
    //     read = cleanupReadString(read);
    //     complementReversed = makeReversedComplement(read); 
    //     rankSelectInit(read, ACGT_last_rank, lastColBitMap, firstCol,readsMatchIndexFile, milestoneMap);  
    //     rankSelectInit(complementReversed, ACGT_last_rank, lastColBitMap, firstCol, readsMatchIndexFile, milestoneMap);  
    //     // ll band_start, band_end;
    //     // char lastChar = read.back();
    //     // bool found = false;

    //     // read = cleanupReadString(read);
    //     // read_complement = makeReversedComplement(read);
    //     // readStrLen = read.size();
    //     // read.pop_back();

    //     // /*
    //     //     A   A   C   C   G   G   T
    //     //     0   1   2   3   4   5   6
    //     //     firstCol[A] = 2
    //     //     firstCol[C] = 2
    //     //     firstCol[G] = 2
    //     //     firstCol[T] = 1
    //     // */
    //     // ll A_start, C_start, G_start, T_start, A_end, C_end, G_end, T_end;
    //     // A_start = 0; C_start = A_start + firstCol[0]; G_start = C_start + firstCol[1]; T_start = G_start + firstCol[2];
    //     // A_end = firstCol[0] -1; C_end = A_end + firstCol[1]; G_end = C_end + firstCol[2]; T_end = G_end + firstCol[3];

    //     // switch(lastChar) {
    //     //     case 'A': band_start = A_start; band_end = A_end; break;
    //     //     case 'C': band_start = C_start; band_end = C_end; break;
    //     //     case 'G': band_start = G_start; band_end = G_end; break;
    //     //     case 'T': band_start = T_start; band_end = T_end; break;
    //     //     default: cout<<"Invalid string. Found unknown character: "<<lastChar<<".\n"; exit(EXIT_FAILURE);
    //     // }
    //     // found = rankSelect(read, band_start, band_end, ACGT_last_rank, lastColBitMap, firstCol);
    //     // if (found) {
    //     //     cout<<"BS:"<<band_start<<", BE:"<<band_end<<".\n";
    //     //     getPosInRef(band_start, band_end,ACGT_last_rank, firstCol, lastColBitMap);
    //     // } else cout<<"Cannot find this read string.\n";
    //     // cout<<endl;
    // }
    cout<<"\n";

    cout<<endl;   
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
    // for (ull i = 0; i < str.length(); i++) {
    //     switch (str[i]){
    //         case 'A':
    //         case 'C':
    //         case 'G':
    //         case 'T': break;
    //         case 'N': str[i] = 'A';
    //         default: cout<<"Unknown character in read string: "<<str<<".\n"; exit(EXIT_FAILURE);
    //     }
    // }
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

void rankSelectInit(string read, vector<vector<ull>>& ACGT_last_rank, vector<int>& lastColBitMap, vector<ull>& firstCol, fstream& readsMatchIndexFile, vector<ll>& milestoneMap) {
    string readCopy = read;
    ll band_start, band_end;
    char lastChar = read.back();
    bool found = false;
    ll readStrLen = read.size();  

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
    
    // if (found) {
    //     // cout<<"BS:"<<band_start<<", BE:"<<band_end<<".\n";
    //     getPosInRef(band_start, band_end,ACGT_last_rank, firstCol, lastColBitMap, readCopy, readsMatchIndexFile, milestoneMap);
    // } else cout<<"Cannot find this read string.\n";
    // cout<<endl;

    if (found) {
        // cout<<"BS:"<<band_start<<", BE:"<<band_end<<".\n";
        getPosInRef(band_start, band_end,ACGT_last_rank, firstCol, lastColBitMap, readCopy, readsMatchIndexFile, milestoneMap);
    } else readsMatchIndexFile<<"Not found. ["<<readCopy<<"]\n";
}

bool rankSelect(string& read, ll& band_start, ll& band_end, vector<vector<ull>>& ACGT_last_rank, vector<int>& lastColBitMap, vector<ull>& firstCol){
    // cout<<"bs:"<<band_start<<", be:"<<band_end<<", remaining read:"<<read<<".\n";
    if (read.size() == 0) return true; // recheck this condition
    if (band_end < band_start) return false; // recheck this condition

    char ch = read.back();
    read.pop_back();
    if (noChInBand(ch, band_start, band_end, lastColBitMap)) return false;

    /*
        We have the band_start & band_end.
        The read string's last char is last char to be found in last col,
        i.e., we need the sub-band within this band 
    */
    ll rank_first_ch, rank_last_ch, index_nearestMilestone_bitmap, index_nearestMilestone_rankDS, count_ch_fromMilestone_toBandStart = 0, count_ch_fromMileStone_toBandEnd = 0;
    index_nearestMilestone_rankDS = band_start/DELTA; // index of nearest milstone in rank DS (rankDS = rank data structure)
    index_nearestMilestone_bitmap = band_start - (band_start%DELTA); // index of nearest milstone in bitmap array
    // for (ll i = index_nearestMilestone_bitmap; i < band_start; i++){
    //     if(lastColBitMap[i] == charToBitStr(ch)) count_ch_fromMilestone_toBandStart++; // counts until just before band_start char (see definition of rank).
    // }

    // get number of ch in between index_nearestMilestone_bitmap to the first occurence of ch in the band (index_nearestMilestone_bitmap inclusive only)
    ll iter = index_nearestMilestone_bitmap;
    while(true) {
        if (iter >= band_end) break;
        if (iter >= band_start && lastColBitMap[iter] == charToBitStr(ch)) break;
        if (lastColBitMap[iter] == charToBitStr(ch)) count_ch_fromMilestone_toBandStart++;

        iter++;
    }
    // Use this logic 1 or 2
    /*
    // Logic 1
    count_ch_fromMileStone_toBandEnd = count_ch_fromMilestone_toBandStart;
    for (ll i = band_start; i < band_end; i++){
        if(lastColBitMap[i] == ch) count_ch_fromMileStone_toBandEnd++; // counts until just before band_end char (see definition of rank).
    }
    if(count_ch_fromMileStone_toBandEnd < 0) return false; // check this condition against test cases
    */
    // Logic 2
    // for (ll i = index_nearestMilestone_bitmap; i < band_end; i++){
    //     if(lastColBitMap[i] == charToBitStr(ch)) count_ch_fromMileStone_toBandEnd++; // counts until just before band_end char (see definition of rank).
    // }

    // get number of ch in between index_nearestMilestone_bitmap to the last occurence of ch in the band (index_nearestMilestone_bitmap inclusive only)
    iter = index_nearestMilestone_bitmap;
    while(true) {
        if (iter > band_end) break;
        if (lastColBitMap[iter] == charToBitStr(ch)) count_ch_fromMileStone_toBandEnd++;
        iter++;
    }
    count_ch_fromMileStone_toBandEnd--; // remove the last ch that got counted.
    
    // if(count_ch_fromMileStone_toBandEnd - count_ch_fromMilestone_toBandStart < 0) return false;
    rank_first_ch = getMileStoneRankFromRankDS(ch, index_nearestMilestone_rankDS, ACGT_last_rank) + count_ch_fromMilestone_toBandStart;
    rank_last_ch = getMileStoneRankFromRankDS(ch, index_nearestMilestone_rankDS, ACGT_last_rank) + count_ch_fromMileStone_toBandEnd;

    /* 
        take the ranks found in last column to first column
        and locate the band start & end for the ranks first 
        & last, respectively
    */
    band_start = getBandInFirstColFromRank(ch, rank_first_ch, firstCol);
    band_end = getBandInFirstColFromRank(ch, rank_last_ch, firstCol);

    // cout
    // <<"[index_nearestMilestone_rankDS:"<<index_nearestMilestone_rankDS
    // <<",index_nearestMilestone_bitmap:"<<index_nearestMilestone_bitmap
    // <<",count_ch_fromMilestone_toBandStart:"<<count_ch_fromMilestone_toBandStart
    // <<",count_ch_fromMileStone_toBandEnd:"<<count_ch_fromMileStone_toBandEnd
    // <<",getMileStoneRankFromRankDS():"<<getMileStoneRankFromRankDS(ch, index_nearestMilestone_rankDS, ACGT_last_rank)
    // <<",rank_first_ch:"<<rank_first_ch
    // <<",rank_last_ch:"<<rank_last_ch
    // <<",band_start:"<<band_start
    // <<",band_end:"<<band_end
    // <<"]\n"
    // ;
    
    return rankSelect(read, band_start, band_end, ACGT_last_rank, lastColBitMap, firstCol);
}

bool noChInBand(char ch, ll band_start, ll band_end, vector<int>& lastColBitMap) {
    ll seenCh = 0;
    for (ll i = band_start; i <= band_end; i++){
        if(lastColBitMap[i] == charToBitStr(ch)) seenCh++;
    }
    return !(bool)seenCh;
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

void getPosInRef(ll band_start, ll band_end, vector<vector<ull>>& ACGT_last_rank, vector<ull>& firstCol, vector<int>& lastColBitMap, string read, fstream& readsMatchIndexFile, vector<ll>& milestoneMap) {
    // fstream readsFile(milestoneMapFilename, ios::in);
    // string read;
    ll startIndexInRef;
    ull count_jumps = 0;

    // if(!readsFile.is_open()){
    //     cout<<"| Unable to open "<<milestoneMapFilename<<".\n";
    //     exit(EXIT_FAILURE);
    // }

    // vector<ll> milestoneMap;

    // while(getline(readsFile, read)){
    //     milestoneMap.push_back(stoll(read));
    // }

    for (ll band_iter = band_start; band_iter <= band_end; band_iter++){
        ll band = band_iter;
        // cout<<"\t\t(Band:"<<band<<")";
        while(band%DELTA != 0){
            count_jumps++;
            char last_ch_band_start = getLastCharAtRow(band, lastColBitMap);
            ll rank_ch_band_start;
            ll rank_first_ch, index_nearestMilestone_bitmap, index_nearestMilestone_rankDS, count_ch_fromMilestone_toBandStart = 0;

            index_nearestMilestone_rankDS = band/DELTA; // index of nearest milstone in rank DS (rankDS = rank data structure)
            index_nearestMilestone_bitmap = band - (band%DELTA); // index of nearest milstone in bitmap array
            for (ll i = index_nearestMilestone_bitmap; i < band; i++){
                if(lastColBitMap[i] == charToBitStr(last_ch_band_start)) count_ch_fromMilestone_toBandStart++; // counts until just before band char (see definition of rank).
            }
            rank_first_ch = getMileStoneRankFromRankDS(last_ch_band_start, index_nearestMilestone_rankDS, ACGT_last_rank) + count_ch_fromMilestone_toBandStart;
            band = getBandInFirstColFromRank(last_ch_band_start, rank_first_ch, firstCol);

            // cout<<"\t\t(Band:"<<band<<")";
        }
        // cout<<"\tStart position in ref string is at: "<<((milestoneMap[band]+count_jumps >= DELTA) ? (milestoneMap[band]+count_jumps)%DELTA : milestoneMap[band]+count_jumps)<<".\n";
        // cout<<"\tStart position in ref string is at: "<<milestoneMap[band]+count_jumps<<". (count_jumps="<<count_jumps<<", milestoneMap[band]"<<milestoneMap[band]<<", band:"<<band<<")\n";
        startIndexInRef = milestoneMap[(ll)(band/DELTA)]+count_jumps;
        // cout<<"Start position in ref string is at: "<<startIndexInRef<<"\n";
        readsMatchIndexFile<<startIndexInRef<<" ["<<read<<"]\n"<<flush;
        count_jumps = 0;
    }

    // readsMatchIndexFile<<startIndexInRef<<" ["<<read<<"]\n";
}

char getLastCharAtRow(ll index, vector<int>& lastColBitMap) {
    switch(lastColBitMap[index]) {
        case 1000: return 'A';
        case 100: return 'C';
        case 10: return 'G';
        case 1: return 'T';
        case 0: return '$';
        default: cout<<"Invalid bitstr from lastColBitMap.\n"; exit(EXIT_FAILURE);
    };
    return 'X';
}

ll getFileSize(fstream& file){
    ll currPos = file.tellg();
    file.seekg(0, ios::end);
    ll size = file.tellg();
    file.seekg(currPos, ios::beg);
    return size;
}
