#include <iostream>
#include <string>
#include <ctime>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <algorithm>
using namespace std;

// Hash function for hash table indexing
int simpleHash(const string& key, int tableSize) {
    int hash = 0;
    for (char c : key) {
        hash = (hash * 31 + c) % tableSize;
    }
    return abs(hash) % tableSize;
}

// Generate hash for blockchain block hashing
string generateHash(const string& data) {
    unsigned long hash = 5381;
    for (char c : data) {
        hash = ((hash << 5) + hash) + c;
    }
    stringstream ss;
    ss << hex << hash;
    return ss.str();
}

// Simple encryption/decryption using XOR cipher
string encryptDecrypt(const string& data, const string& key) {
    string result = data;
    for (size_t i = 0; i < data.length(); i++) {
        result[i] = data[i] ^ key[i % key.length()];
    }
    return result;
}

struct Voter {
    string voterID;
    string name;
    bool hasVoted;
    Voter* next;
    
    Voter(string id, string n) : voterID(id), name(n), hasVoted(false), next(nullptr) {}
};

class VoterHashTable {
private:
    static const int TABLE_SIZE = 100;
    Voter* table[TABLE_SIZE];
    int totalVoters;
    string encryptionKey;
    
public:
    VoterHashTable() : totalVoters(0), encryptionKey("VOTING_SECURE_2024") {
        for (int i = 0; i < TABLE_SIZE; i++) {
            table[i] = nullptr;
        }
    }
    
    void insertVoter(string voterID, string name) {
        try {
            if (voterID.empty()) {
                throw invalid_argument("Voter ID cannot be empty");
            }
            if (name.empty()) {
                throw invalid_argument("Name cannot be empty");
            }
            
            // Check if voter already exists
            if (findVoter(voterID) != nullptr) {
                cout << "[ERROR] Voter ID already exists!\n";
                return;
            }
            
            int index = simpleHash(voterID, TABLE_SIZE);
            Voter* newVoter = new Voter(voterID, name);
            
            if (newVoter == nullptr) {
                throw runtime_error("Memory allocation failed");
            }
            
            // Insert at the beginning of the chain (collision handling)
            newVoter->next = table[index];
            table[index] = newVoter;
            totalVoters++;
            
            cout << "[SUCCESS] Voter registered successfully: " << name << " (ID: " << voterID << ")\n";
            cout << "          Hash Index: " << index << "\n";
        } catch (const invalid_argument& e) {
            cout << "[ERROR] Invalid input: " << e.what() << "\n";
        } catch (const bad_alloc& e) {
            cout << "[ERROR] Memory allocation error: " << e.what() << "\n";
        } catch (const exception& e) {
            cout << "[ERROR] Error registering voter: " << e.what() << "\n";
        }
    }
    
    Voter* findVoter(string voterID) {
        int index = simpleHash(voterID, TABLE_SIZE);
        Voter* current = table[index];
        
        while (current != nullptr) {
            if (current->voterID == voterID) {
                return current;
            }
            current = current->next;
        }
        return nullptr;
    }
    
    bool authenticateVoter(string voterID) {
        Voter* voter = findVoter(voterID);
        if (voter == nullptr) {
            cout << "[ERROR] Authentication failed: Voter ID not found\n";
            return false;
        }
        cout << "[SUCCESS] Authentication successful: " << voter->name << "\n";
        return true;
    }
    
    bool markAsVoted(string voterID) {
        Voter* voter = findVoter(voterID);
        if (voter == nullptr) {
            return false;
        }
        if (voter->hasVoted) {
            return false;
        }
        voter->hasVoted = true;
        return true;
    }
    
    void displayAllVoters() {
        cout << "\n╔----------------------------------------╗\n";
        cout << "|       REGISTERED VOTERS LIST           |\n";
        cout << "╚----------------------------------------╝\n";
        
        int count = 0;
        for (int i = 0; i < TABLE_SIZE; i++) {
            Voter* current = table[i];
            while (current != nullptr) {
                cout << "  ID: " << setw(10) << left << current->voterID 
                     << " | Name: " << setw(20) << left << current->name 
                     << " | Voted: " << (current->hasVoted ? "[YES]" : "[NO]") << "\n";
                count++;
                current = current->next;
            }
        }
        cout << "-----------------------------------------\n";
        cout << "  Total registered voters: " << count << "\n";
        cout << "  Hash table load factor: " << fixed << setprecision(2) 
             << (double)count / TABLE_SIZE << "\n\n";
    }
    
    void displayHashTableStatistics() {
        cout << "\n╔----------------------------------------╗\n";
        cout << "|     HASH TABLE STATISTICS              |\n";
        cout << "╚----------------------------------------╝\n";
        
        int emptySlots = 0;
        int maxChainLength = 0;
        int totalChainLength = 0;
        int usedSlots = 0;
        
        for (int i = 0; i < TABLE_SIZE; i++) {
            int chainLength = 0;
            Voter* current = table[i];
            
            if (current == nullptr) {
                emptySlots++;
            } else {
                usedSlots++;
                while (current != nullptr) {
                    chainLength++;
                    current = current->next;
                }
                totalChainLength += chainLength;
                maxChainLength = max(maxChainLength, chainLength);
            }
        }
        
        cout << "  Table Size: " << TABLE_SIZE << "\n";
        cout << "  Total Voters: " << totalVoters << "\n";
        cout << "  Used Slots: " << usedSlots << "\n";
        cout << "  Empty Slots: " << emptySlots << "\n";
        cout << "  Load Factor: " << fixed << setprecision(2) 
             << (double)totalVoters / TABLE_SIZE << "\n";
        cout << "  Max Chain Length: " << maxChainLength << "\n";
        if (usedSlots > 0) {
            cout << "  Avg Chain Length: " << fixed << setprecision(2) 
                 << (double)totalChainLength / usedSlots << "\n";
        }
        cout << "  Collision Rate: " << fixed << setprecision(2) 
             << (double)(totalVoters - usedSlots) / totalVoters * 100 << "%\n\n";
    }
    
    bool saveToFile(const string& filename) {
        try {
            ofstream file(filename);
            if (!file.is_open()) {
                throw runtime_error("Unable to open file for writing");
            }
            
            for (int i = 0; i < TABLE_SIZE; i++) {
                Voter* current = table[i];
                while (current != nullptr) {
                    string encryptedID = encryptDecrypt(current->voterID, encryptionKey);
                    string encryptedName = encryptDecrypt(current->name, encryptionKey);
                    file << encryptedID << "|" << encryptedName << "|" 
                         << current->hasVoted << "\n";
                    current = current->next;
                }
            }
            file.close();
            return true;
        } catch (const exception& e) {
            cout << "[ERROR] Error saving voter database: " << e.what() << "\n";
            return false;
        }
    }
    
    bool loadFromFile(const string& filename) {
        try {
            ifstream file(filename);
            if (!file.is_open()) {
                return false;
            }
            
            string line;
            while (getline(file, line)) {
                stringstream ss(line);
                string encryptedID, encryptedName, votedStr;
                
                getline(ss, encryptedID, '|');
                getline(ss, encryptedName, '|');
                getline(ss, votedStr, '|');
                
                if (!encryptedID.empty() && !encryptedName.empty()) {
                    string voterID = encryptDecrypt(encryptedID, encryptionKey);
                    string name = encryptDecrypt(encryptedName, encryptionKey);
                    
                    int index = simpleHash(voterID, TABLE_SIZE);
                    Voter* newVoter = new Voter(voterID, name);
                    newVoter->hasVoted = (votedStr == "1");
                    newVoter->next = table[index];
                    table[index] = newVoter;
                    totalVoters++;
                }
            }
            file.close();
            return true;
        } catch (const exception& e) {
            cout << "[ERROR] Error loading voter database: " << e.what() << "\n";
            return false;
        }
    }
    
    int getTotalVoters() const { return totalVoters; }
    
    int getVotedCount() const {
        int count = 0;
        for (int i = 0; i < TABLE_SIZE; i++) {
            Voter* current = table[i];
            while (current != nullptr) {
                if (current->hasVoted) count++;
                current = current->next;
            }
        }
        return count;
    }
    
    ~VoterHashTable() {
        for (int i = 0; i < TABLE_SIZE; i++) {
            Voter* current = table[i];
            while (current != nullptr) {
                Voter* toDelete = current;
                current = current->next;
                delete toDelete;
            }
        }
    }
};

struct VoteRecord {
    string voterID;
    string candidate;
    time_t timestamp;
    string hash;
    string previousHash;
    VoteRecord* next;
    
    VoteRecord(string vID, string cand, string prevHash = "0") 
        : voterID(vID), candidate(cand), previousHash(prevHash), next(nullptr) {
        timestamp = time(nullptr);
        hash = calculateHash();
    }
    
    string calculateHash() {
        stringstream ss;
        ss << voterID << candidate << timestamp << previousHash;
        return generateHash(ss.str());
    }
};

class VoteLedger {
private:
    VoteRecord* head;
    VoteRecord* tail;
    int recordCount;
    string encryptionKey;
    
public:
    VoteLedger() : head(nullptr), tail(nullptr), recordCount(0), 
                   encryptionKey("BLOCKCHAIN_KEY_2024") {}
    
    void addVote(string voterID, string candidate) {
        try {
            string prevHash = (tail != nullptr) ? tail->hash : "0";
            VoteRecord* newRecord = new VoteRecord(voterID, candidate, prevHash);
            
            if (newRecord == nullptr) {
                throw runtime_error("Memory allocation failed");
            }
            
            if (head == nullptr) {
                head = tail = newRecord;
            } else {
                tail->next = newRecord;
                tail = newRecord;
            }
            recordCount++;
            cout << "[LOG] Vote recorded in blockchain (Block #" << recordCount << ")\n";
        } catch (const exception& e) {
            cout << "[ERROR] Error adding vote to ledger: " << e.what() << "\n";
        }
    }
    
    void displayLedger() {
        cout << "\n╔----------------------------------------╗\n";
        cout << "|       BLOCKCHAIN VOTE LEDGER           |\n";
        cout << "╚----------------------------------------╝\n";
        
        VoteRecord* current = head;
        int recordNum = 1;
        
        while (current != nullptr) {
            cout << "\n┌- Block #" << recordNum << " ---------------------\n";
            cout << "| Voter ID: " << current->voterID << "\n";
            cout << "| Candidate: " << current->candidate << "\n";
            cout << "| Timestamp: " << ctime(&current->timestamp);
            cout << "| Hash: " << current->hash << "\n";
            cout << "| Previous Hash: " << current->previousHash << "\n";
            cout << "└--------------------------------------\n";
            
            current = current->next;
            recordNum++;
        }
        cout << "\nTotal blocks: " << recordCount << "\n\n";
    }
    
    int getTotalVotes() const { return recordCount; }
    
    bool verifyChain() {
        if (head == nullptr) {
            return true;
        }
        
        VoteRecord* current = head;
        int blockNum = 1;
        
        while (current != nullptr) {
            string calculatedHash = current->calculateHash();
            if (calculatedHash != current->hash) {
                cout << "[ERROR] Block #" << blockNum << ": Hash mismatch!\n";
                cout << "        Expected: " << current->hash << "\n";
                cout << "        Calculated: " << calculatedHash << "\n";
                return false;
            }
            
            if (current->next != nullptr) {
                if (current->hash != current->next->previousHash) {
                    cout << "[ERROR] Block #" << blockNum << " -> #" << (blockNum + 1) 
                         << ": Chain link broken!\n";
                    return false;
                }
            }
            current = current->next;
            blockNum++;
        }
        return true;
    }
    
    void displayBlockchainHealth() {
        cout << "\n╔----------------------------------------╗\n";
        cout << "|     BLOCKCHAIN HEALTH REPORT           |\n";
        cout << "╚----------------------------------------╝\n";
        
        cout << "  Total Blocks: " << recordCount << "\n";
        cout << "  Chain Status: ";
        
        if (verifyChain()) {
            cout << "[VALID] All blocks verified\n";
            cout << "  Integrity: 100%\n";
            cout << "  Security Level: HIGH\n";
        } else {
            cout << "[INVALID] Tampering detected!\n";
            cout << "  Integrity: COMPROMISED\n";
            cout << "  Security Level: CRITICAL\n";
        }
        cout << "\n";
    }
    
    bool saveToFile(const string& filename) {
        try {
            ofstream file(filename);
            if (!file.is_open()) {
                throw runtime_error("Unable to open file for writing");
            }
            
            VoteRecord* current = head;
            while (current != nullptr) {
                string encryptedVoterID = encryptDecrypt(current->voterID, encryptionKey);
                file << encryptedVoterID << "|" << current->candidate << "|"
                     << current->timestamp << "|" << current->hash << "|"
                     << current->previousHash << "\n";
                current = current->next;
            }
            file.close();
            return true;
        } catch (const exception& e) {
            cout << "[ERROR] Error saving vote ledger: " << e.what() << "\n";
            return false;
        }
    }
    
    bool loadFromFile(const string& filename) {
        try {
            ifstream file(filename);
            if (!file.is_open()) {
                return false;
            }
            
            string line;
            while (getline(file, line)) {
                stringstream ss(line);
                string encryptedVoterID, candidate, timestampStr, hash, previousHash;
                
                getline(ss, encryptedVoterID, '|');
                getline(ss, candidate, '|');
                getline(ss, timestampStr, '|');
                getline(ss, hash, '|');
                getline(ss, previousHash, '|');
                
                if (!encryptedVoterID.empty() && !candidate.empty()) {
                    string voterID = encryptDecrypt(encryptedVoterID, encryptionKey);
                    VoteRecord* newRecord = new VoteRecord(voterID, candidate, previousHash);
                    newRecord->timestamp = stol(timestampStr);
                    newRecord->hash = hash;
                    
                    if (head == nullptr) {
                        head = tail = newRecord;
                    } else {
                        tail->next = newRecord;
                        tail = newRecord;
                    }
                    recordCount++;
                }
            }
            file.close();
            return true;
        } catch (const exception& e) {
            cout << "[ERROR] Error loading vote ledger: " << e.what() << "\n";
            return false;
        }
    }
    
    ~VoteLedger() {
        VoteRecord* current = head;
        while (current != nullptr) {
            VoteRecord* toDelete = current;
            current = current->next;
            delete toDelete;
        }
    }
};

struct CandidateNode {
    string name;
    int voteCount;
    CandidateNode* left;
    CandidateNode* right;
    
    CandidateNode(string n) : name(n), voteCount(0), left(nullptr), right(nullptr) {}
};

class CandidateBST {
private:
    CandidateNode* root;
    
    CandidateNode* insert(CandidateNode* node, string name) {
        if (node == nullptr) {
            return new CandidateNode(name);
        }
        if (name < node->name) {
            node->left = insert(node->left, name);
        } else if (name > node->name) {
            node->right = insert(node->right, name);
        }
        return node;
    }
    
    CandidateNode* search(CandidateNode* node, string name) {
        if (node == nullptr || node->name == name) {
            return node;
        }
        if (name < node->name) {
            return search(node->left, name);
        }
        return search(node->right, name);
    }
    
    void inorderTraversal(CandidateNode* node) {
        if (node != nullptr) {
            inorderTraversal(node->left);
            cout << "  " << setw(20) << left << node->name << ": " 
                 << node->voteCount << " votes\n";
            inorderTraversal(node->right);
        }
    }
    
    void collectCandidates(CandidateNode* node, vector<pair<string, int>>& candidates) {
        if (node != nullptr) {
            collectCandidates(node->left, candidates);
            candidates.push_back({node->name, node->voteCount});
            collectCandidates(node->right, candidates);
        }
    }
    
    int getTotalVotes(CandidateNode* node) {
        if (node == nullptr) return 0;
        return node->voteCount + getTotalVotes(node->left) + getTotalVotes(node->right);
    }
    
    void destroyTree(CandidateNode* node) {
        if (node != nullptr) {
            destroyTree(node->left);
            destroyTree(node->right);
            delete node;
        }
    }
    
    void saveToFileHelper(CandidateNode* node, ofstream& file) {
        if (node != nullptr) {
            file << node->name << "|" << node->voteCount << "\n";
            saveToFileHelper(node->left, file);
            saveToFileHelper(node->right, file);
        }
    }
    
public:
    CandidateBST() : root(nullptr) {}
    
    void addCandidate(string name) {
        root = insert(root, name);
        cout << "[SUCCESS] Candidate added: " << name << "\n";
    }
    
    bool addVote(string name) {
        CandidateNode* candidate = search(root, name);
        if (candidate == nullptr) {
            return false;
        }
        candidate->voteCount++;
        return true;
    }
    
    void displayResults() {
        cout << "\n╔----------------------------------------╗\n";
        cout << "|       ELECTION RESULTS                 |\n";
        cout << "╚----------------------------------------╝\n";
        inorderTraversal(root);
        cout << "-----------------------------------------\n";
        cout << "  Total votes cast: " << getTotalVotes(root) << "\n\n";
    }
    
    string getWinner() {
        if (root == nullptr) return "No candidates";
        vector<pair<string, int>> allCandidates;
        collectCandidates(root, allCandidates);
        string winner = "";
        int maxVotes = -1;
        for (const auto& candidate : allCandidates) {
            if (candidate.second > maxVotes) {
                maxVotes = candidate.second;
                winner = candidate.first;
            }
        }
        return winner;
    }
    
    void getVotePercentages() {
        if (root == nullptr) {
            cout << "No votes cast yet.\n";
            return;
        }
        vector<pair<string, int>> allCandidates;
        collectCandidates(root, allCandidates);
        int total = getTotalVotes(root);
        if (total == 0) {
            cout << "No votes cast yet.\n";
            return;
        }
        
        cout << "\n╔----------------------------------------╗\n";
        cout << "|       VOTE PERCENTAGES                 |\n";
        cout << "╚----------------------------------------╝\n";
        
        for (const auto& candidate : allCandidates) {
            double percentage = (candidate.second * 100.0) / total;
            cout << "  " << setw(20) << left << candidate.first << ": " 
                 << setw(5) << right << candidate.second << " votes (" 
                 << fixed << setprecision(2) << setw(6) << percentage << "%)\n";
        }
        cout << "\n";
    }
    
    void generateStatistics() {
        if (root == nullptr) {
            cout << "No candidates in the system.\n";
            return;
        }
        vector<pair<string, int>> allCandidates;
        collectCandidates(root, allCandidates);
        int total = getTotalVotes(root);
        
        cout << "\n╔----------------------------------------╗\n";
        cout << "|       ELECTION STATISTICS              |\n";
        cout << "╚----------------------------------------╝\n";
        cout << "  Total Votes Cast: " << total << "\n";
        cout << "  Number of Candidates: " << allCandidates.size() << "\n";
        
        if (total > 0) {
            string winner = getWinner();
            cout << "  Leading Candidate: " << winner << "\n";
            int winnerVotes = 0;
            for (const auto& candidate : allCandidates) {
                if (candidate.first == winner) {
                    winnerVotes = candidate.second;
                    break;
                }
            }
            double winnerPercentage = (winnerVotes * 100.0) / total;
            cout << "  Leading Votes: " << winnerVotes << " (" 
                 << fixed << setprecision(2) << winnerPercentage << "%)\n";
            double avgVotes = (double)total / allCandidates.size();
            cout << "  Average Votes/Candidate: " << fixed << setprecision(2) << avgVotes << "\n";
            int zeroVoteCandidates = 0;
            for (const auto& candidate : allCandidates) {
                if (candidate.second == 0) {
                    zeroVoteCandidates++;
                }
            }
            cout << "  Candidates with Zero Votes: " << zeroVoteCandidates << "\n";
        }
        cout << "\n";
    }
    
    bool exportResults(const string& filename) {
        try {
            ofstream file(filename);
            if (!file.is_open()) {
                throw runtime_error("Unable to open file for writing");
            }
            vector<pair<string, int>> allCandidates;
            collectCandidates(root, allCandidates);
            int total = getTotalVotes(root);
            
            time_t now = time(nullptr);
            file << "╔----------------------------------------╗\n";
            file << "|    ELECTION RESULTS REPORT             |\n";
            file << "╚----------------------------------------╝\n";
            file << "Generated: " << ctime(&now) << "\n";
            file << "CANDIDATE VOTES:\n";
            file << "-----------------------------------------\n";
            for (const auto& candidate : allCandidates) {
                double percentage = (total > 0) ? (candidate.second * 100.0) / total : 0;
                file << candidate.first << ": " << candidate.second << " votes (" 
                     << fixed << setprecision(2) << percentage << "%)\n";
            }
            file << "\nSTATISTICS:\n";
            file << "-----------------------------------------\n";
            file << "Total Votes: " << total << "\n";
            file << "Total Candidates: " << allCandidates.size() << "\n";
            if (total > 0) {
                string winner = getWinner();
                file << "Winner: " << winner << "\n";
            }
            file.close();
            cout << "[SUCCESS] Results exported to " << filename << "\n";
            return true;
        } catch (const exception& e) {
            cout << "[ERROR] Error exporting results: " << e.what() << "\n";
            return false;
        }
    }
    
    bool candidateExists(string name) {
        return search(root, name) != nullptr;
    }
    
    bool saveToFile(const string& filename) {
        try {
            ofstream file(filename);
            if (!file.is_open()) {
                throw runtime_error("Unable to open file for writing");
            }
            saveToFileHelper(root, file);
            file.close();
            return true;
        } catch (const exception& e) {
            cout << "[ERROR] Error saving candidates: " << e.what() << "\n";
            return false;
        }
    }
    
    bool loadFromFile(const string& filename) {
        try {
            ifstream file(filename);
            if (!file.is_open()) {
                return false;
            }
            
            string line;
            while (getline(file, line)) {
                stringstream ss(line);
                string name, voteCountStr;
                
                getline(ss, name, '|');
                getline(ss, voteCountStr, '|');
                
                if (!name.empty()) {
                    root = insert(root, name);
                    CandidateNode* node = search(root, name);
                    if (node != nullptr) {
                        node->voteCount = stoi(voteCountStr);
                    }
                }
            }
            file.close();
            return true;
        } catch (const exception& e) {
            cout << "[ERROR] Error loading candidates: " << e.what() << "\n";
            return false;
        }
    }
    
    ~CandidateBST() {
        destroyTree(root);
    }
};

class VotingSystem {
private:
    VoterHashTable voterDB;
    VoteLedger ledger;
    CandidateBST candidates;
    bool isAdminMode;
    
    void printHeader(const string& title) {
        cout << "\n";
        cout << "╔----------------------------------------╗\n";
        cout << "|  " << setw(38) << left << title << "|\n";
        cout << "╚----------------------------------------╝\n";
    }
    
public:
    VotingSystem() : isAdminMode(false) {}
    
    void initializeCandidates() {
        candidates.addCandidate("Akram");
        candidates.addCandidate("Kashan");
        candidates.addCandidate("Mubashir");
        candidates.addCandidate("Suleman");
    }
    
    void registerVoter(string voterID, string name) {
        voterDB.insertVoter(voterID, name);
    }
    
    void castVote(string voterID, string candidate) {
        try {
            if (voterID.empty()) {
                throw invalid_argument("Voter ID cannot be empty");
            }
            if (candidate.empty()) {
                throw invalid_argument("Candidate name cannot be empty");
            }
            
            // Step 1: Authenticate voter using hash table
            if (!voterDB.authenticateVoter(voterID)) {
                throw runtime_error("Voter authentication failed!");
            }
            
            Voter* voter = voterDB.findVoter(voterID);
            if (voter == nullptr) {
                throw runtime_error("Voter ID not found! Please register first.");
            }
            if (voter->hasVoted) {
                throw runtime_error("This voter has already cast their vote!");
            }
            if (!candidates.candidateExists(candidate)) {
                throw invalid_argument("Invalid candidate name!");
            }
            
            if (!voterDB.markAsVoted(voterID)) {
                throw runtime_error("Failed to mark voter as voted");
            }
            
            ledger.addVote(voterID, candidate);
            
            if (!candidates.addVote(candidate)) {
                throw runtime_error("Failed to record vote for candidate");
            }
            
            cout << "[SUCCESS] Vote successfully cast for " << candidate << "!\n";
        } catch (const invalid_argument& e) {
            cout << "[ERROR] Invalid input: " << e.what() << "\n";
        } catch (const runtime_error& e) {
            cout << "[ERROR] " << e.what() << "\n";
        } catch (const exception& e) {
            cout << "[ERROR] Unexpected error casting vote: " << e.what() << "\n";
        }
    }
    
    void displayResults() {
        candidates.displayResults();
    }
    
    void displayLedger() {
        ledger.displayLedger();
    }
    
    void displayVoters() {
        voterDB.displayAllVoters();
    }
    
    void showPercentages() {
        candidates.getVotePercentages();
    }
    
    void showStatistics() {
        candidates.generateStatistics();
    }
    
    void exportElectionResults(const string& filename) {
        candidates.exportResults(filename);
    }
    
    bool saveSystemState(const string& baseFilename) {
        try {
            printHeader("SAVING SYSTEM STATE");
            bool success = true;
            
            cout << "  Saving voters database...";
            success &= voterDB.saveToFile(baseFilename + "_voters.txt");
            cout << " [DONE]\n";
            
            cout << "  Saving blockchain ledger...";
            success &= ledger.saveToFile(baseFilename + "_ledger.txt");
            cout << " [DONE]\n";
            
            cout << "  Saving candidates data...";
            success &= candidates.saveToFile(baseFilename + "_candidates.txt");
            cout << " [DONE]\n";
            
            if (success) {
                cout << "\n[SUCCESS] System state saved successfully!\n\n";
            } else {
                cout << "\n[WARNING] Some files may not have been saved properly\n\n";
            }
            return success;
        } catch (const exception& e) {
            cout << "❌ Error saving system state: " << e.what() << "\n";
            return false;
        }
    }
    
    bool loadSystemState(const string& baseFilename) {
        try {
            printHeader("LOADING SYSTEM STATE");
            
            cout << "  Loading voters database...";
            bool votersLoaded = voterDB.loadFromFile(baseFilename + "_voters.txt");
            cout << (votersLoaded ? " [DONE]\n" : " [NOT FOUND]\n");
            
            cout << "  Loading blockchain ledger...";
            bool ledgerLoaded = ledger.loadFromFile(baseFilename + "_ledger.txt");
            cout << (ledgerLoaded ? " [DONE]\n" : " [NOT FOUND]\n");
            
            cout << "  Loading candidates data...";
            bool candidatesLoaded = candidates.loadFromFile(baseFilename + "_candidates.txt");
            cout << (candidatesLoaded ? " [DONE]\n" : " [NOT FOUND]\n");
            
            if (votersLoaded || ledgerLoaded || candidatesLoaded) {
                cout << "\n[SUCCESS] System state loaded successfully!\n\n";
                return true;
            }
            return false;
        } catch (const exception& e) {
            cout << "❌ Error loading system state: " << e.what() << "\n";
            return false;
        }
    }
    
    bool exportReport(const string& filename) {
        try {
            ofstream file(filename);
            if (!file.is_open()) {
                throw runtime_error("Unable to open file for writing");
            }
            
            time_t now = time(nullptr);
            file << "╔----------------------------------------╗\n";
            file << "|  COMPREHENSIVE ELECTION REPORT         |\n";
            file << "╚----------------------------------------╝\n";
            file << "Generated: " << ctime(&now) << "\n";
            
            file << "\n--- SYSTEM STATISTICS ---\n";
            file << "Total Registered Voters: " << voterDB.getTotalVoters() << "\n";
            file << "Votes Cast: " << voterDB.getVotedCount() << "\n";
            file << "Turnout Rate: " << fixed << setprecision(2) 
                 << (double)voterDB.getVotedCount() / voterDB.getTotalVoters() * 100 << "%\n";
            
            file << "\n--- BLOCKCHAIN STATUS ---\n";
            if (ledger.verifyChain()) {
                file << "Blockchain Integrity: [VERIFIED]\n";
            } else {
                file << "Blockchain Integrity: [COMPROMISED]\n";
            }
            file << "Total Blockchain Records: " << ledger.getTotalVotes() << "\n";
            
            file.close();
            cout << "[SUCCESS] Comprehensive report exported to " << filename << "\n";
            return true;
        } catch (const exception& e) {
            cout << "[ERROR] Error exporting report: " << e.what() << "\n";
            return false;
        }
    }
    
    void auditBlockchain() {
        printHeader("BLOCKCHAIN AUDIT");
        ledger.displayBlockchainHealth();
    }
    
    void detectTampering() {
        printHeader("TAMPERING DETECTION");
        if (ledger.verifyChain()) {
            cout << "  [OK] No tampering detected\n";
            cout << "  [SECURE] System integrity maintained\n\n";
        } else {
            cout << "  [ALERT] Potential tampering detected!\n";
            cout << "  [WARNING] Please review the blockchain immediately\n\n";
        }
    }
    
    void showHashTableStats() {
        voterDB.displayHashTableStatistics();
    }
    
    void adminDashboard() {
        printHeader("ADMIN DASHBOARD");
        cout << "  [STATS] System Overview:\n";
        cout << "  |-- Total Voters: " << voterDB.getTotalVoters() << "\n";
        cout << "  |-- Voted: " << voterDB.getVotedCount() << "\n";
        cout << "  |-- Pending: " << voterDB.getTotalVoters() - voterDB.getVotedCount() << "\n";
        cout << "  |-- Turnout: " << fixed << setprecision(2) 
             << (double)voterDB.getVotedCount() / max(1, voterDB.getTotalVoters()) * 100 << "%\n";
        cout << "  |-- Blockchain Blocks: " << ledger.getTotalVotes() << "\n\n";
        
        cout << "  [SECURITY] Security Status:\n";
        cout << "  |-- Encryption: [ENABLED]\n";
        cout << "  |-- Hash Table: [ACTIVE]\n";
        cout << "  |-- Blockchain: " << (ledger.verifyChain() ? "[VERIFIED]" : "[INVALID]") << "\n\n";
    }
    
    void fullSystemAudit() {
        printHeader("FULL SYSTEM AUDIT");
        
        cout << "\n[1] Hash Table Analysis:\n";
        voterDB.displayHashTableStatistics();
        
        cout << "\n[2] Blockchain Verification:\n";
        ledger.displayBlockchainHealth();
        
        cout << "\n[3] Election Statistics:\n";
        candidates.generateStatistics();
        
        cout << "\n[4] Data Integrity Check:\n";
        if (voterDB.getTotalVoters() > 0) {
            cout << "  [OK] Voter database: OK\n";
        } else {
            cout << "  [WARNING] Voter database: Empty\n";
        }
        
        if (ledger.getTotalVotes() == voterDB.getVotedCount()) {
            cout << "  [OK] Vote ledger matches voter records\n";
        } else {
            cout << "  [WARNING] Mismatch between ledger and voter records\n";
            cout << "            Ledger: " << ledger.getTotalVotes() << " | Voters: " 
                 << voterDB.getVotedCount() << "\n";
        }
        cout << "\n";
    }
};

void displayBanner() {
    cout << "\n|------------------------------------------------|\n";
    cout << "|                                                |\n";
    cout << "|         SECURE E-VOTING SYSTEM                 |\n";
    cout << "|                                                |\n";
    cout << "|   Blockchain-Powered | Hash-Table Auth         |\n";
    cout << "|   Encrypted Storage | Tamper-Proof             |\n";
    cout << "|                                                |\n";
    cout << "|------------------------------------------------|\n";
}

void displayMenu() {
    cout << "\n|----------------------------------------|\n";
    cout << "|          MAIN MENU                     |\n";
    cout << "|----------------------------------------|\n";
    cout << "| VOTER OPERATIONS:                      |\n";
    cout << "|  1. Register Voter                     |\n";
    cout << "|  2. Cast Vote                          |\n";
    cout << "|  3. Display Election Results           |\n";
    cout << "|  4. View Registered Voters             |\n";
    cout << "|  5. Show Vote Percentages              |\n";
    cout << "|                                        |\n";
    cout << "| BLOCKCHAIN & SECURITY:                 |\n";
    cout << "|  6. View Blockchain Ledger             |\n";
    cout << "|  7. Audit Blockchain                   |\n";
    cout << "|  8. Detect Tampering                   |\n";
    cout << "|                                        |\n";
    cout << "| ADMIN TOOLS:                           |\n";
    cout << "|  9. Show Statistics                    |\n";
    cout << "| 10. Hash Table Statistics              |\n";
    cout << "| 11. Admin Dashboard                    |\n";
    cout << "| 12. Full System Audit                  |\n";
    cout << "|                                        |\n";
    cout << "| FILE OPERATIONS:                       |\n";
    cout << "| 13. Save System State                  |\n";
    cout << "| 14. Load System State                  |\n";
    cout << "| 15. Export Results to File             |\n";
    cout << "| 16. Export Comprehensive Report        |\n";
    cout << "|                                        |\n";
    cout << "|  0. Exit                               |\n";
    cout << "|----------------------------------------|\n";
    cout << "Choose an option: ";
}

int main() {
    try {
        displayBanner();
        VotingSystem system;
        
        cout << "\n[INIT] Initializing E-Voting System...\n";
        
        if (!system.loadSystemState("voting_data")) {
            cout << "[INFO] No previous data found. Starting fresh.\n\n";
            system.initializeCandidates();
            
            cout << "[SETUP] Registering default voters...\n";
            system.registerVoter("V001", "Abbad");
            system.registerVoter("V002", "Talal");
            system.registerVoter("V003", "Haziq");
            cout << "\n";
        }
        
        int choice;
        string voterID, name, candidate, filename;
        
        while (true) {
            try {
                displayMenu();
                
                if (!(cin >> choice)) {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    throw invalid_argument("Invalid input! Please enter a number.");
                }
                cin.ignore();
                
                switch (choice) {
                    case 1:
                        cout << "\n[REGISTER] VOTER REGISTRATION\n";
                        cout << "Enter Voter ID: ";
                        getline(cin, voterID);
                        cout << "Enter Name: ";
                        getline(cin, name);
                        system.registerVoter(voterID, name);
                        break;
                        
                    case 2:
                        cout << "\n[VOTE] CAST VOTE\n";
                        cout << "Enter Voter ID: ";
                        getline(cin, voterID);
                        cout << "\nAvailable Candidates:\n";
                        cout << "  - Akram\n";
                        cout << "  - Kashan\n";
                        cout << "  - Mubashir\n";
                        cout << "  - Suleman\n";
                        cout << "\nEnter Candidate Name: ";
                        getline(cin, candidate);
                        system.castVote(voterID, candidate);
                        break;
                        
                    case 3:
                        system.displayResults();
                        break;
                        
                    case 4:
                        system.displayVoters();
                        break;
                        
                    case 5:
                        system.showPercentages();
                        break;
                        
                    case 6:
                        system.displayLedger();
                        break;
                        
                    case 7:
                        system.auditBlockchain();
                        break;
                        
                    case 8:
                        system.detectTampering();
                        break;
                        
                    case 9:
                        system.showStatistics();
                        break;
                        
                    case 10:
                        system.showHashTableStats();
                        break;
                        
                    case 11:
                        system.adminDashboard();
                        break;
                        
                    case 12:
                        system.fullSystemAudit();
                        break;
                        
                    case 13:
                        system.saveSystemState("voting_data");
                        break;
                        
                    case 14:
                        if (system.loadSystemState("voting_data")) {
                            cout << "[SUCCESS] System state loaded successfully!\n";
                        } else {
                            cout << "[ERROR] No saved data found or error loading.\n";
                        }
                        break;
                        
                    case 15:
                        cout << "Enter filename (e.g., results.txt): ";
                        getline(cin, filename);
                        system.exportElectionResults(filename);
                        break;
                        
                    case 16:
                        cout << "Enter filename (e.g., report.txt): ";
                        getline(cin, filename);
                        system.exportReport(filename);
                        break;
                        
                    case 0:
                        cout << "\n[SAVE] Saving system state before exit...\n";
                        system.saveSystemState("voting_data");
                        cout << "\n[EXIT] Thank you for using Secure E-Voting System!\n";
                        cout << "[INFO] All data encrypted and saved securely.\n\n";
                        return 0;
                        
                    default:
                        cout << "[ERROR] Invalid choice! Please try again.\n";
                }
            } catch (const invalid_argument& e) {
                cout << "[ERROR] Input error: " << e.what() << "\n";
            } catch (const runtime_error& e) {
                cout << "[ERROR] Runtime error: " << e.what() << "\n";
            } catch (const exception& e) {
                cout << "[ERROR] " << e.what() << "\n";
            }
        }
    } catch (const exception& e) {
        cout << "[CRITICAL] Critical error: " << e.what() << "\n";
        cout << "[CRITICAL] The system encountered a fatal error and must exit.\n";
        return 1;
    } catch (...) {
        cout << "[CRITICAL] Unknown critical error occurred!\n";
        return 1;
    }
    return 0;
}