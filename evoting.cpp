#include <iostream>
#include <string>
#include <ctime>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <chrono>
#include <limits>
using namespace std;
using namespace std::chrono;

// Simple hash function
int simpleHash(const string& key, int tableSize) {
    if (tableSize <= 0) return 0;
    unsigned int hash = 0;
    for (size_t i = 0; i < key.length(); i++) {
        hash = (hash * 31 + static_cast<unsigned int>(key[i])) % tableSize;
    }
    return hash % tableSize;
}

// Generate hash for blockchain
string generateHash(const string& data) {
    unsigned long hash = 5381;
    for (size_t i = 0; i < data.length(); i++) {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(data[i]);
    }
    stringstream ss;
    ss << hex << hash;
    return ss.str();
}

// SIMPLIFIED ENCRYPTION: Caesar Cipher
string simpleEncrypt(const string& data, const string& key) {
    string result = data;
    int keyValue = 0;
    for (size_t i = 0; i < key.length(); i++) {
        keyValue += static_cast<int>(key[i]);
    }
    keyValue = keyValue % 26;
    for (size_t i = 0; i < data.length(); i++) {
        if (isalpha(data[i])) {
            if (isupper(data[i])) {
                result[i] = 'A' + (data[i] - 'A' + keyValue) % 26;
            } else {
                result[i] = 'a' + (data[i] - 'a' + keyValue) % 26;
            }
        } else if (isdigit(data[i])) {
            result[i] = '0' + (data[i] - '0' + keyValue) % 10;
        } else {
            result[i] = data[i];
        }
    }
    return result;
}

// SIMPLIFIED DECRYPTION: Reverse the Caesar Cipher
string simpleDecrypt(const string& data, const string& key) {
    string result = data;
    int keyValue = 0;
    for (size_t i = 0; i < key.length(); i++) {
        keyValue += static_cast<int>(key[i]);
    }
    keyValue = keyValue % 26;
    for (size_t i = 0; i < data.length(); i++) {
        if (isalpha(data[i])) {
            if (isupper(data[i])) {
                result[i] = 'A' + (data[i] - 'A' - keyValue + 26) % 26;
            } else {
                result[i] = 'a' + (data[i] - 'a' - keyValue + 26) % 26;
            }
        } else if (isdigit(data[i])) {
            result[i] = '0' + (data[i] - '0' - keyValue + 10) % 10;
        } else {
            result[i] = data[i];
        }
    }
    return result;
}

// Input validation
bool isValidID(const string& id) {
    if (id.empty() || id.length() > 20) return false;
    for (size_t i = 0; i < id.length(); i++) {
        if (!isalnum(id[i])) return false;
    }
    return true;
}

bool isValidName(const string& name) {
    if (name.empty() || name.length() > 50) return false;
    return true;
}

// Voter structure
struct Voter {
    string voterID;
    string name;
    bool hasVoted;
    Voter* next;
    Voter(string id, string n) : voterID(id), name(n), hasVoted(false), next(NULL) {}
};

// Hash Table for storing voters with dynamic resizing
class VoterHashTable {
private:
    Voter** table;
    int totalVoters;
    int capacity;
    string encryptionKey;
    const double LOAD_FACTOR_THRESHOLD;
    const int INITIAL_CAPACITY;
    
    void resizeTable() {
        auto start = high_resolution_clock::now();
        int oldCapacity = capacity;
        Voter** oldTable = table;
        capacity = capacity * 2;
        table = new Voter*[capacity];
        for (int i = 0; i < capacity; i++) {
            table[i] = NULL;
        }
        totalVoters = 0;
        for (int i = 0; i < oldCapacity; i++) {
            Voter* current = oldTable[i];
            while (current != NULL) {
                Voter* next = current->next;
                int newIndex = simpleHash(current->voterID, capacity);
                current->next = table[newIndex];
                table[newIndex] = current;
                totalVoters++;
                current = next;
            }
        }
        delete[] oldTable;
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end - start);
        cout << "[HASH TABLE] Resized from " << oldCapacity << " to " << capacity << "\n";
        cout << "             [TIME] Resize operation: " << duration.count() << " microseconds\n";
    }
    
public:
    VoterHashTable() : totalVoters(0), encryptionKey("VOTE2024"), 
                       LOAD_FACTOR_THRESHOLD(0.7), INITIAL_CAPACITY(10), capacity(INITIAL_CAPACITY) {
        table = new Voter*[capacity];
        for (int i = 0; i < capacity; i++) {
            table[i] = NULL;
        }
    }
    
    void insertVoter(string voterID, string name) {
        auto start = high_resolution_clock::now();
        try {
            if (!isValidID(voterID)) {
                throw invalid_argument("Invalid Voter ID! Use max 20 alphanumeric chars.");
            }
            if (!isValidName(name)) {
                throw invalid_argument("Invalid name! Max 50 characters.");
            }
            if (findVoter(voterID) != NULL) {
                cout << "[ERROR] Voter ID already exists!\n";
                return;
            }
            double loadFactor = (double)(totalVoters + 1) / capacity;
            if (loadFactor > LOAD_FACTOR_THRESHOLD) {
                cout << "[HASH TABLE] Load factor " << fixed << setprecision(2) << loadFactor 
                     << " > " << LOAD_FACTOR_THRESHOLD << ", resizing...\n";
                resizeTable();
            }
            int index = simpleHash(voterID, capacity);
            Voter* newVoter = new Voter(voterID, name);
            newVoter->next = table[index];
            table[index] = newVoter;
            totalVoters++;
            auto end = high_resolution_clock::now();
            auto duration = duration_cast<microseconds>(end - start);
            cout << "[SUCCESS] Voter registered: " << name << " (ID: " << voterID << ")\n";
            cout << "          Stored at hash index: " << index << "\n";
            cout << "          Current capacity: " << capacity << ", Load factor: " 
                 << fixed << setprecision(2) << (double)totalVoters / capacity << "\n";
            cout << "          [TIME] Hash Table Insert: " << duration.count() << " microseconds\n";
        } catch (const exception& e) {
            cout << "[ERROR] " << e.what() << "\n";
        }
    }
    
    Voter* findVoter(string voterID) {
        int index = simpleHash(voterID, capacity);
        Voter* current = table[index];
        while (current != NULL) {
            if (current->voterID == voterID) {
                return current;
            }
            current = current->next;
        }
        return NULL;
    }
    
    bool authenticateVoter(string voterID) {
        auto start = high_resolution_clock::now();
        Voter* voter = findVoter(voterID);
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end - start);
        if (voter == NULL) {
            cout << "[ERROR] Voter ID not found!\n";
            cout << "        [TIME] Hash Table Search: " << duration.count() << " microseconds\n";
            return false;
        }
        cout << "[SUCCESS] Welcome, " << voter->name << "!\n";
        cout << "          [TIME] Hash Table Search: " << duration.count() << " microseconds\n";
        return true;
    }
    
    bool markAsVoted(string voterID) {
        Voter* voter = findVoter(voterID);
        if (voter == NULL) return false;
        if (voter->hasVoted) return false;
        voter->hasVoted = true;
        return true;
    }
    
    void displayAllVoters() {
        cout << "\n+========================================+\n";
        cout << "|       REGISTERED VOTERS LIST           |\n";
        cout << "+========================================+\n";
        for (int i = 0; i < capacity; i++) {
            Voter* current = table[i];
            while (current != NULL) {
                cout << "  ID: " << setw(10) << left << current->voterID
                     << " | Name: " << setw(20) << left << current->name
                     << " | Voted: " << (current->hasVoted ? "YES" : "NO") << "\n";
                current = current->next;
            }
        }
        cout << "\n  Total voters: " << totalVoters << "\n\n";
    }
    
    void displayHashTableStats() {
        cout << "\n+========================================+\n";
        cout << "|     HASH TABLE STATISTICS              |\n";
        cout << "+========================================+\n";
        int usedSlots = 0;
        int maxChain = 0;
        int totalChains = 0;
        for (int i = 0; i < capacity; i++) {
            if (table[i] != NULL) {
                usedSlots++;
                int chainLen = 0;
                Voter* current = table[i];
                while (current != NULL) {
                    chainLen++;
                    current = current->next;
                }
                maxChain = max(maxChain, chainLen);
                totalChains += chainLen;
            }
        }
        double avgChainLength = usedSlots > 0 ? (double)totalChains / usedSlots : 0.0;
        cout << "  Current Capacity: " << capacity << "\n";
        cout << "  Used Slots: " << usedSlots << "\n";
        cout << "  Empty Slots: " << (capacity - usedSlots) << "\n";
        cout << "  Total Voters: " << totalVoters << "\n";
        cout << "  Load Factor: " << fixed << setprecision(2)
             << (double)totalVoters / capacity << " (threshold: " << LOAD_FACTOR_THRESHOLD << ")\n";
        cout << "  Longest Chain: " << maxChain << "\n";
        cout << "  Average Chain Length: " << fixed << setprecision(2) << avgChainLength << "\n";
        cout << "  Resizing Strategy: Double capacity when load > " << LOAD_FACTOR_THRESHOLD << "\n";
        cout << "  Average Time Complexity: O(1) for search/insert\n";
        cout << "  Worst Case (with collisions): O(" << maxChain << ")\n\n";
    }
    
    bool saveToFile(const string& filename) {
        try {
            ofstream file(filename.c_str());
            if (!file.is_open()) {
                throw runtime_error("Cannot open file");
            }
            for (int i = 0; i < capacity; i++) {
                Voter* current = table[i];
                while (current != NULL) {
                    string encID = simpleEncrypt(current->voterID, encryptionKey);
                    string encName = simpleEncrypt(current->name, encryptionKey);
                    file << encID << "|" << encName << "|" << current->hasVoted << "\n";
                    current = current->next;
                }
            }
            file.close();
            return true;
        } catch (const exception& e) {
            cout << "[ERROR] Save failed: " << e.what() << "\n";
            return false;
        }
    }
    
    // File loading with validation
    bool loadFromFile(const string& filename) {
        try {
            ifstream file(filename.c_str());
            if (!file.is_open()) return false;
            
            // Clear existing data
            for (int i = 0; i < capacity; i++) {
                Voter* current = table[i];
                while (current != NULL) {
                    Voter* toDelete = current;
                    current = current->next;
                    delete toDelete;
                }
                table[i] = NULL;
            }
            totalVoters = 0;
            
            string line;
            int loadedCount = 0;
            
            while (getline(file, line)) {
                // Skip empty or whitespace-only lines
                if (line.empty() || line.find_first_not_of(" \t\r\n") == string::npos) {
                    continue;
                }
                
                size_t pos1 = line.find('|');
                size_t pos2 = line.find('|', pos1 + 1);
                
                // Validate format
                if (pos1 == string::npos || pos2 == string::npos || pos1 >= pos2) {
                    cout << "[WARNING] Skipping invalid line in file\n";
                    continue;
                }
                
                string encID = line.substr(0, pos1);
                string encName = line.substr(pos1 + 1, pos2 - pos1 - 1);
                string votedStr = line.substr(pos2 + 1);
                
                // Validate encrypted data is not empty
                if (encID.empty() || encName.empty()) {
                    cout << "[WARNING] Skipping line with empty data\n";
                    continue;
                }
                
                try {
                    string voterID = simpleDecrypt(encID, encryptionKey);
                    string name = simpleDecrypt(encName, encryptionKey);
                    bool voted = (votedStr == "1");
                    
                    // Validate decrypted data
                    if (!isValidID(voterID) || !isValidName(name)) {
                        cout << "[WARNING] Skipping invalid voter data\n";
                        continue;
                    }
                    
                    // Check if resize needed
                    if ((double)(totalVoters + 1) / capacity > LOAD_FACTOR_THRESHOLD) {
                        resizeTable();
                    }
                    
                    int index = simpleHash(voterID, capacity);
                    Voter* newVoter = new Voter(voterID, name);
                    newVoter->hasVoted = voted;
                    newVoter->next = table[index];
                    table[index] = newVoter;
                    totalVoters++;
                    loadedCount++;
                    
                } catch (const exception& e) {
                    cout << "[WARNING] Error processing voter: " << e.what() << "\n";
                    continue;
                }
            }
            
            file.close();
            cout << "[INFO] Loaded " << loadedCount << " voters from file\n";
            return true;
            
        } catch (const exception& e) {
            cout << "[ERROR] Load failed: " << e.what() << "\n";
            return false;
        }
    }
    
    int getTotalVoters() const { return totalVoters; }
    
    int getVotedCount() const {
        int count = 0;
        for (int i = 0; i < capacity; i++) {
            Voter* current = table[i];
            while (current != NULL) {
                if (current->hasVoted) count++;
                current = current->next;
            }
        }
        return count;
    }
    
    ~VoterHashTable() {
        for (int i = 0; i < capacity; i++) {
            Voter* current = table[i];
            while (current != NULL) {
                Voter* toDelete = current;
                current = current->next;
                delete toDelete;
            }
        }
        delete[] table;
    }
};

// Blockchain block structure
struct VoteRecord {
    string voterID;
    string candidate;
    time_t timestamp;
    string hash;
    string previousHash;
    VoteRecord* next;
    
    VoteRecord(string vID, string cand, string prevHash = "0")
        : voterID(vID), candidate(cand), previousHash(prevHash), next(NULL) {
        timestamp = time(NULL);
        hash = calculateHash();
    }
    
    string calculateHash() {
        stringstream ss;
        ss << voterID << candidate << timestamp << previousHash;
        return generateHash(ss.str());
    }
};

// Blockchain ledger
class VoteLedger {
private:
    VoteRecord* head;
    VoteRecord* tail;
    int recordCount;
    
public:
    VoteLedger() : head(NULL), tail(NULL), recordCount(0) {}
    
    void addVote(string voterID, string candidate) {
        auto start = high_resolution_clock::now();
        try {
            string prevHash = (tail != NULL) ? tail->hash : "0";
            VoteRecord* newRecord = new VoteRecord(voterID, candidate, prevHash);
            if (head == NULL) {
                head = tail = newRecord;
            } else {
                tail->next = newRecord;
                tail = newRecord;
            }
            recordCount++;
            auto end = high_resolution_clock::now();
            auto duration = duration_cast<microseconds>(end - start);
            cout << "[BLOCKCHAIN] Vote recorded in Block #" << recordCount << "\n";
            cout << "             [TIME] Blockchain Insert: " << duration.count() << " microseconds\n";
            cout << "             Time Complexity: O(1) - append to end\n";
        } catch (const exception& e) {
            cout << "[ERROR] Blockchain error: " << e.what() << "\n";
        }
    }
    
    void displayLedger() {
        cout << "\n+========================================+\n";
        cout << "|       BLOCKCHAIN VOTE LEDGER           |\n";
        cout << "+========================================+\n";
        VoteRecord* current = head;
        int blockNum = 1;
        while (current != NULL) {
            cout << "\n+-- Block #" << blockNum << " -------------------------\n";
            cout << "| Voter: " << current->voterID << "\n";
            cout << "| Candidate: " << current->candidate << "\n";
            char* timeStr = ctime(&current->timestamp);
            cout << "| Time: " << timeStr;
            cout << "| Hash: " << current->hash << "\n";
            cout << "| Previous: " << current->previousHash << "\n";
            cout << "+--------------------------------------\n";
            current = current->next;
            blockNum++;
        }
        cout << "\nTotal blocks: " << recordCount << "\n";
        cout << "Traversal Time Complexity: O(n) where n = " << recordCount << "\n\n";
    }
    
    bool verifyChain() {
        auto start = high_resolution_clock::now();
        if (head == NULL) return true;
        VoteRecord* current = head;
        int blockNum = 1;
        while (current != NULL) {
            string calculatedHash = current->calculateHash();
            if (calculatedHash != current->hash) {
                cout << "[ALERT] Block #" << blockNum << " has been tampered!\n";
                return false;
            }
            if (current->next != NULL) {
                if (current->hash != current->next->previousHash) {
                    cout << "[ALERT] Chain broken between Block #" << blockNum
                         << " and #" << (blockNum + 1) << "!\n";
                    return false;
                }
            }
            current = current->next;
            blockNum++;
        }
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end - start);
        cout << "[TIME] Blockchain Verification: " << duration.count() << " microseconds\n";
        cout << "       Time Complexity: O(n) where n = " << recordCount << "\n";
        return true;
    }
    
    void auditBlockchain() {
        cout << "\n+========================================+\n";
        cout << "|     BLOCKCHAIN SECURITY AUDIT          |\n";
        cout << "+========================================+\n";
        cout << "  Total Blocks: " << recordCount << "\n";
        cout << "  Chain Status: ";
        if (verifyChain()) {
            cout << "VALID (No tampering detected)\n";
            cout << "  Security: HIGH\n";
        } else {
            cout << "INVALID (Tampering detected!)\n";
            cout << "  Security: COMPROMISED\n";
        }
        cout << "\n";
    }
    
    int getTotalVotes() const { return recordCount; }
    
    ~VoteLedger() {
        VoteRecord* current = head;
        while (current != NULL) {
            VoteRecord* toDelete = current;
            current = current->next;
            delete toDelete;
        }
    }
};

// BST Node for candidates
struct CandidateNode {
    string name;
    int voteCount;
    CandidateNode* left;
    CandidateNode* right;
    CandidateNode(string n) : name(n), voteCount(0), left(NULL), right(NULL) {}
};

// Binary Search Tree for candidates
class CandidateBST {
private:
    CandidateNode* root;
    
    CandidateNode* insert(CandidateNode* node, string name) {
        if (node == NULL) {
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
        if (node == NULL || node->name == name) {
            return node;
        }
        if (name < node->name) {
            return search(node->left, name);
        }
        return search(node->right, name);
    }
    
    void inorder(CandidateNode* node) {
        if (node != NULL) {
            inorder(node->left);
            cout << "  " << setw(20) << left << node->name
                 << ": " << node->voteCount << " votes\n";
            inorder(node->right);
        }
    }
    
    int getTotalVotes(CandidateNode* node) {
        if (node == NULL) return 0;
        return node->voteCount + getTotalVotes(node->left) + getTotalVotes(node->right);
    }
    
    int getHeight(CandidateNode* node) {
        if (node == NULL) return 0;
        int leftHeight = getHeight(node->left);
        int rightHeight = getHeight(node->right);
        return 1 + max(leftHeight, rightHeight);
    }
    
    void destroy(CandidateNode* node) {
        if (node != NULL) {
            destroy(node->left);
            destroy(node->right);
            delete node;
        }
    }
    
    void displayPercent(CandidateNode* node, int total) {
        if (node != NULL) {
            displayPercent(node->left, total);
            double percent = (node->voteCount * 100.0) / total;
            cout << "  " << setw(20) << left << node->name
                 << ": " << setw(5) << node->voteCount << " votes ("
                 << fixed << setprecision(1) << percent << "%)\n";
            displayPercent(node->right, total);
        }
    }
    
public:
    CandidateBST() : root(NULL) {}
    
    void addCandidate(string name) {
        root = insert(root, name);
        cout << "[SUCCESS] Candidate added: " << name << "\n";
    }
    
    bool addVote(string name) {
        auto start = high_resolution_clock::now();
        CandidateNode* candidate = search(root, name);
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end - start);
        if (candidate == NULL) {
            return false;
        }
        candidate->voteCount++;
        int height = getHeight(root);
        cout << "          [TIME] BST Search: " << duration.count() << " microseconds\n";
        cout << "          Tree Height: " << height << " | Time Complexity: O(log n) avg, O(" << height << ") this case\n";
        return true;
    }
    
    void displayResults() {
        cout << "\n+========================================+\n";
        cout << "|       ELECTION RESULTS                 |\n";
        cout << "+========================================+\n";
        inorder(root);
        cout << "\n  Total votes: " << getTotalVotes(root) << "\n";
        cout << "  BST Height: " << getHeight(root) << "\n";
        cout << "  Inorder Traversal: O(n) where n = number of candidates\n\n";
    }
    
    void displayPercentages() {
        int total = getTotalVotes(root);
        if (total == 0) {
            cout << "No votes cast yet.\n";
            return;
        }
        cout << "\n+========================================+\n";
        cout << "|       VOTE PERCENTAGES                 |\n";
        cout << "+========================================+\n";
        displayPercent(root, total);
        cout << "\n";
    }
    
    bool candidateExists(string name) {
        return search(root, name) != NULL;
    }
    
    ~CandidateBST() {
        destroy(root);
    }
};

// Main voting system
class VotingSystem {
private:
    VoterHashTable voterDB;
    VoteLedger ledger;
    CandidateBST candidates;
    bool candidatesInitialized;
    
public:
    VotingSystem() : candidatesInitialized(false) {}
    
    void initializeCandidates() {
        if (!candidatesInitialized) {
            candidates.addCandidate("Akram");
            candidates.addCandidate("Kashan");
            candidates.addCandidate("Mubashir");
            candidates.addCandidate("Suleman");
            candidatesInitialized = true;
        }
    }
    
    void registerVoter(string id, string name) {
        voterDB.insertVoter(id, name);
    }
    
    void castVote(string voterID, string candidate) {
        cout << "\n========== VOTE CASTING PROCESS ==========\n";
        auto totalStart = high_resolution_clock::now();
        try {
            if (!voterDB.authenticateVoter(voterID)) {
                throw runtime_error("Authentication failed!");
            }
            Voter* voter = voterDB.findVoter(voterID);
            if (voter == NULL) {
                throw runtime_error("Voter not found!");
            }
            if (voter->hasVoted) {
                throw runtime_error("You have already voted!");
            }
            if (!candidates.candidateExists(candidate)) {
                throw runtime_error("Invalid candidate!");
            }
            if (!voterDB.markAsVoted(voterID)) {
                throw runtime_error("Failed to mark as voted");
            }
            ledger.addVote(voterID, candidate);
            candidates.addVote(candidate);
            auto totalEnd = high_resolution_clock::now();
            auto totalDuration = duration_cast<microseconds>(totalEnd - totalStart);
            cout << "\n[SUCCESS] Vote successfully cast for " << candidate << "!\n";
            cout << "==========================================\n";
            cout << "[TOTAL TIME] Complete voting process: " << totalDuration.count() << " microseconds\n";
            cout << "             (" << fixed << setprecision(3) << totalDuration.count() / 1000.0 << " milliseconds)\n";
            cout << "==========================================\n\n";
        } catch (const exception& e) {
            cout << "[ERROR] " << e.what() << "\n";
        }
    }
    
    void showResults() { candidates.displayResults(); }
    void showPercentages() { candidates.displayPercentages(); }
    void showVoters() { voterDB.displayAllVoters(); }
    void showLedger() { ledger.displayLedger(); }
    void showHashStats() { voterDB.displayHashTableStats(); }
    void auditBlockchain() { ledger.auditBlockchain(); }
    
    void showDashboard() {
        cout << "\n+========================================+\n";
        cout << "|       ADMIN DASHBOARD                  |\n";
        cout << "+========================================+\n";
        int total = voterDB.getTotalVoters();
        int voted = voterDB.getVotedCount();
        cout << "  Total Registered: " << total << "\n";
        cout << "  Votes Cast: " << voted << "\n";
        cout << "  Not Voted: " << (total - voted) << "\n";
        if (total > 0) {
            cout << "  Turnout: " << fixed << setprecision(1)
                 << (voted * 100.0 / total) << "%\n";
        }
        cout << "  Blockchain Blocks: " << ledger.getTotalVotes() << "\n";
        cout << "  Security: " << (ledger.verifyChain() ? "SECURE" : "COMPROMISED") << "\n\n";
    }
    
    void showTimeComplexityAnalysis() {
        cout << "\n+========================================+\n";
        cout << "|   TIME COMPLEXITY ANALYSIS             |\n";
        cout << "+========================================+\n";
        cout << "\n1. HASH TABLE (Voter Database):\n";
        cout << "   - Insert:  O(1) average (amortized), O(n) worst\n";
        cout << "   - Search:  O(1) average, O(n) worst\n";
        cout << "   - Delete:  O(1) average, O(n) worst\n";
        cout << "   - Resize:  O(n) - rehash all elements\n";
        cout << "   * n = chain length at index\n";
        cout << "   * Amortized O(1) due to dynamic resizing\n";
        cout << "\n2. BLOCKCHAIN (Vote Ledger):\n";
        cout << "   - Insert:  O(1) - append to end\n";
        cout << "   - Verify:  O(n) - check all blocks\n";
        cout << "   - Search:  O(n) - linear traversal\n";
        cout << "   * n = number of blocks\n";
        cout << "\n3. BINARY SEARCH TREE (Candidates):\n";
        cout << "   - Insert:  O(log n) average, O(n) worst\n";
        cout << "   - Search:  O(log n) average, O(n) worst\n";
        cout << "   - Traverse: O(n) - visit all nodes\n";
        cout << "   * n = number of candidates\n";
        cout << "\n4. COMPLETE VOTING OPERATION:\n";
        cout << "   Total = O(1) amortized + O(1) + O(log n)\n";
        cout << "   Result: O(log n) overall\n";
        cout << "+========================================+\n\n";
    }
    
    bool saveData() {
        cout << "\n[SAVING] Saving system data...\n";
        bool success = voterDB.saveToFile("voters.dat");
        if (success) {
            cout << "[SUCCESS] Data saved successfully!\n\n";
        }
        return success;
    }
    
    bool loadData() {
        cout << "\n[LOADING] Loading system data...\n";
        bool success = voterDB.loadFromFile("voters.dat");
        if (success) {
            cout << "[SUCCESS] Data loaded successfully!\n\n";
        }
        return success;
    }
};

void showBanner() {
    cout << "\n";
    cout << "+========================================+\n";
    cout << "|                                        |\n";
    cout << "|     SECURE E-VOTING SYSTEM             |\n";
    cout << "|                                        |\n";
    cout << "|   Blockchain - Hash Table - BST        |\n";
    cout << "|   WITH TIME COMPLEXITY ANALYSIS        |\n";
    cout << "|                                        |\n";
    cout << "+========================================+\n";
}

void showMenu() {
    cout << "\n+========================================+\n";
    cout << "|          MAIN MENU                     |\n";
    cout << "+========================================+\n";
    cout << "| VOTER OPERATIONS:                      |\n";
    cout << "|  1. Register New Voter                 |\n";
    cout << "|  2. Cast Vote (with timing)            |\n";
    cout << "|  3. View Election Results              |\n";
    cout << "|  4. View Vote Percentages              |\n";
    cout << "|  5. View Registered Voters             |\n";
    cout << "|                                        |\n";
    cout << "| BLOCKCHAIN & SECURITY:                 |\n";
    cout << "|  6. View Blockchain Ledger             |\n";
    cout << "|  7. Audit Blockchain Security          |\n";
    cout << "|                                        |\n";
    cout << "| ADMIN & ANALYSIS:                      |\n";
    cout << "|  8. Admin Dashboard                    |\n";
    cout << "|  9. Hash Table Statistics              |\n";
    cout << "| 10. Time Complexity Analysis           |\n";
    cout << "|                                        |\n";
    cout << "| FILE OPERATIONS:                       |\n";
    cout << "| 11. Save Data                          |\n";
    cout << "| 12. Load Data                          |\n";
    cout << "|                                        |\n";
    cout << "|  0. Exit                               |\n";
    cout << "+========================================+\n";
    cout << "\nEnter choice: ";
}

// input handling
int getMenuChoice() {
    int choice;
    while (true) {
        if (cin >> choice) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');  // Clear entire buffer
            return choice;
        } else {
            cin.clear();  // Clear error flags
            cin.ignore(numeric_limits<streamsize>::max(), '\n');  // Discard bad input
            cout << "[ERROR] Invalid input! Please enter a number: ";
        }
    }
}

int main() {
    showBanner();
    VotingSystem system;
    
    cout << "\n[INIT] Initializing system...\n";
    
    // initialize candidates
    system.initializeCandidates();
    
    //load voter data
    if (system.loadData()) {
        cout << "[INFO] Previous voter data loaded successfully.\n";
    } else {
        cout << "[INFO] No saved data found. Starting fresh.\n";
        cout << "\n[SETUP] Adding sample voters...\n";
        system.registerVoter("V001", "Abbad Ahmed");
        system.registerVoter("V002", "Talal Khan");
        system.registerVoter("V003", "Haziq Ali");
    }
    
    string id, name, candidate;
    
    while (true) {
        try {
            showMenu();
            int choice = getMenuChoice();
            
            switch (choice) {
                case 1:
                    cout << "\n--- REGISTER VOTER ---\n";
                    cout << "Enter Voter ID: ";
                    getline(cin, id);
                    cout << "Enter Name: ";
                    getline(cin, name);
                    system.registerVoter(id, name);
                    break;
                    
                case 2:
                    cout << "\n--- CAST VOTE ---\n";
                    cout << "Enter your Voter ID: ";
                    getline(cin, id);
                    cout << "\nAvailable Candidates:\n";
                    cout << "  - Akram\n  - Kashan\n  - Mubashir\n  - Suleman\n";
                    cout << "\nEnter candidate name: ";
                    getline(cin, candidate);
                    system.castVote(id, candidate);
                    break;
                    
                case 3:
                    system.showResults();
                    break;
                    
                case 4:
                    system.showPercentages();
                    break;
                    
                case 5:
                    system.showVoters();
                    break;
                    
                case 6:
                    system.showLedger();
                    break;
                    
                case 7:
                    system.auditBlockchain();
                    break;
                    
                case 8:
                    system.showDashboard();
                    break;
                    
                case 9:
                    system.showHashStats();
                    break;
                    
                case 10:
                    system.showTimeComplexityAnalysis();
                    break;
                    
                case 11:
                    system.saveData();
                    break;
                    
                case 12:
                    system.loadData();
                    break;
                    
                case 0:
                    cout << "\n[EXIT] Saving and exiting...\n";
                    system.saveData();
                    cout << "Thank you for using E-Voting System!\n\n";
                    return 0;
                    
                default:
                    cout << "[ERROR] Invalid choice! Please try again.\n";
            }
            
        } catch (const exception& e) {
            cout << "[ERROR] " << e.what() << "\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
    
    return 0;
}