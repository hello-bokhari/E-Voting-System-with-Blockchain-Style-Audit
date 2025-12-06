#include <iostream>
#include <string>
#include <ctime>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <vector>
using namespace std;

// ==================== UTILITY FUNCTIONS ====================

// Simple but decent hash for hash table
unsigned int DJB2Hash(const string& key, unsigned int tableSize) {
    unsigned int hash = 5381;
    for (char c : key) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash % tableSize;
}

// SHA-1 inspired hash (simplified for student project)
string generateSecureHash(const string& data) {
    unsigned int h0 = 0x67452301;
    unsigned int h1 = 0xEFCDAB89;
    unsigned int h2 = 0x98BADCFE;
    unsigned int h3 = 0x10325476;
    unsigned int h4 = 0xC3D2E1F0;
    
    // Process data in blocks
    for (size_t i = 0; i < data.length(); i += 4) {
        unsigned int block = 0;
        for (int j = 0; j < 4 && (i + j) < data.length(); j++) {
            block |= (static_cast<unsigned int>(data[i + j]) << (8 * j));
        }
        
        // Mix the block into the hash state
        h0 = (h0 ^ block) + ((h1 & h2) | (~h1 & h3));
        h1 = (h1 ^ block) + ((h2 & h3) | (~h2 & h4));
        h2 = (h2 ^ block) + ((h3 & h4) | (~h3 & h0));
        h3 = (h3 ^ block) + ((h4 & h0) | (~h4 & h1));
        h4 = (h4 ^ block) + ((h0 & h1) | (~h0 & h2));
        
        // Circular shift
        h0 = (h0 << 1) | (h0 >> 31);
    }
    
    // Combine into final hash
    stringstream ss;
    ss << hex << h0 << h1 << h2 << h3 << h4;
    return ss.str();
}

// Simple input validation
bool isValidVoterID(const string& id) {
    if (id.length() < 3 || id.length() > 20) return false;
    for (char c : id) {
        if (!isalnum(c) && c != '_' && c != '-') return false;
    }
    return true;
}

bool isValidName(const string& name) {
    if (name.length() < 2 || name.length() > 50) return false;
    for (char c : name) {
        if (!isalpha(c) && c != ' ' && c != '.' && c != '\'') return false;
    }
    return true;
}

// ==================== VOTER STRUCTURE ====================

struct Voter {
    string voterID;
    string name;
    string pinHash;  // Hashed PIN for authentication
    bool hasVoted;
    Voter* next;
    time_t registrationTime;
    
    Voter(string id, string n, string pin) 
        : voterID(id), name(n), hasVoted(false), next(nullptr) {
        registrationTime = time(nullptr);
        // Simple hash of PIN for demo (in real system use bcrypt/scrypt)
        pinHash = generateSecureHash(pin);
    }
    
    bool verifyPin(const string& pin) const {
        return pinHash == generateSecureHash(pin);
    }
};

// ==================== VOTER HASH TABLE ====================

class VoterHashTable {
private:
    struct TableSlot {
        Voter* head;
        int count;
        TableSlot() : head(nullptr), count(0) {}
    };
    
    TableSlot* table;
    int tableSize;
    int totalVoters;
    const double MAX_LOAD_FACTOR = 0.75;
    
    void rehash() {
        int oldSize = tableSize;
        TableSlot* oldTable = table;
        
        tableSize = tableSize * 2 + 1;  // Ensure odd size
        table = new TableSlot[tableSize];
        totalVoters = 0;
        
        for (int i = 0; i < oldSize; i++) {
            Voter* current = oldTable[i].head;
            while (current != nullptr) {
                Voter* next = current->next;
                insertVoterDirect(current);
                current = next;
            }
        }
        
        delete[] oldTable;
    }
    
    void insertVoterDirect(Voter* voter) {
        unsigned int index = DJB2Hash(voter->voterID, tableSize);
        voter->next = table[index].head;
        table[index].head = voter;
        table[index].count++;
        totalVoters++;
    }
    
public:
    VoterHashTable(int initialSize = 101) : tableSize(initialSize), totalVoters(0) {
        table = new TableSlot[tableSize];
    }
    
    ~VoterHashTable() {
        clear();
        delete[] table;
    }
    
    void clear() {
        for (int i = 0; i < tableSize; i++) {
            Voter* current = table[i].head;
            while (current != nullptr) {
                Voter* next = current->next;
                delete current;
                current = next;
            }
            table[i].head = nullptr;
            table[i].count = 0;
        }
        totalVoters = 0;
    }
    
    bool registerVoter(const string& voterID, const string& name, const string& pin) {
        if (!isValidVoterID(voterID) || !isValidName(name) || pin.length() < 4) {
            return false;
        }
        
        if (findVoter(voterID) != nullptr) {
            return false;  // Voter already exists
        }
        
        Voter* newVoter = new Voter(voterID, name, pin);
        
        // Check load factor
        double loadFactor = static_cast<double>(totalVoters) / tableSize;
        if (loadFactor > MAX_LOAD_FACTOR) {
            rehash();
        }
        
        insertVoterDirect(newVoter);
        return true;
    }
    
    Voter* findVoter(const string& voterID) const {
        if (!isValidVoterID(voterID)) return nullptr;
        
        unsigned int index = DJB2Hash(voterID, tableSize);
        Voter* current = table[index].head;
        
        while (current != nullptr) {
            if (current->voterID == voterID) {
                return current;
            }
            current = current->next;
        }
        return nullptr;
    }
    
    bool authenticateVoter(const string& voterID, const string& pin) const {
        Voter* voter = findVoter(voterID);
        return (voter != nullptr && !voter->hasVoted && voter->verifyPin(pin));
    }
    
    bool castVote(const string& voterID) {
        Voter* voter = findVoter(voterID);
        if (voter == nullptr || voter->hasVoted) {
            return false;
        }
        voter->hasVoted = true;
        return true;
    }
    
    void displayStatistics() const {
        cout << "\n╔════════════════════════════════════════╗\n";
        cout << "║     VOTER DATABASE STATISTICS        ║\n";
        cout << "╚════════════════════════════════════════╝\n";
        
        int emptySlots = 0;
        int maxChain = 0;
        int totalChains = 0;
        int usedSlots = 0;
        
        for (int i = 0; i < tableSize; i++) {
            if (table[i].head == nullptr) {
                emptySlots++;
            } else {
                usedSlots++;
                totalChains += table[i].count;
                if (table[i].count > maxChain) {
                    maxChain = table[i].count;
                }
            }
        }
        
        cout << "  Table Size: " << tableSize << "\n";
        cout << "  Total Voters: " << totalVoters << "\n";
        cout << "  Used Slots: " << usedSlots << "\n";
        cout << "  Empty Slots: " << emptySlots << "\n";
        cout << "  Load Factor: " << fixed << setprecision(2) 
             << (double)totalVoters / tableSize << "\n";
        cout << "  Max Chain Length: " << maxChain << "\n";
        if (usedSlots > 0) {
            cout << "  Avg Chain Length: " << fixed << setprecision(2) 
                 << (double)totalChains / usedSlots << "\n";
        }
        cout << "\n";
    }
    
    int getTotalVoters() const { return totalVoters; }
    int getVotedCount() const {
        int count = 0;
        for (int i = 0; i < tableSize; i++) {
            Voter* current = table[i].head;
            while (current != nullptr) {
                if (current->hasVoted) count++;
                current = current->next;
            }
        }
        return count;
    }
};

// ==================== ANONYMOUS VOTE RECORD ====================

struct VoteBlock {
    string voteHash;      // Hash of (previousHash + candidate + salt)
    string previousHash;
    string candidate;
    string salt;          // Random salt for each vote
    time_t timestamp;
    VoteBlock* next;
    
    VoteBlock(const string& cand, const string& prevHash) 
        : candidate(cand), previousHash(prevHash), next(nullptr) {
        timestamp = time(nullptr);
        salt = generateSecureHash(to_string(rand()) + to_string(timestamp));
        voteHash = calculateHash();
    }
    
    string calculateHash() const {
        string data = previousHash + candidate + salt + to_string(timestamp);
        return generateSecureHash(data);
    }
    
    // Verify this block's integrity
    bool verify() const {
        return voteHash == calculateHash();
    }
};

class AnonymousVoteChain {
private:
    VoteBlock* head;
    VoteBlock* tail;
    int blockCount;
    string genesisHash;
    
public:
    AnonymousVoteChain() : head(nullptr), tail(nullptr), blockCount(0) {
        genesisHash = generateSecureHash("GENESIS_BLOCK_ELECTION_SYSTEM");
    }
    
    ~AnonymousVoteChain() {
        clear();
    }
    
    void clear() {
        VoteBlock* current = head;
        while (current != nullptr) {
            VoteBlock* next = current->next;
            delete current;
            current = next;
        }
        head = tail = nullptr;
        blockCount = 0;
    }
    
    // Add vote without voter identification (anonymous)
    bool addVote(const string& candidate) {
        string prevHash = (tail != nullptr) ? tail->voteHash : genesisHash;
        VoteBlock* newBlock = new VoteBlock(candidate, prevHash);
        
        if (!newBlock->verify()) {
            delete newBlock;
            return false;
        }
        
        if (head == nullptr) {
            head = tail = newBlock;
        } else {
            tail->next = newBlock;
            tail = newBlock;
        }
        blockCount++;
        return true;
    }
    
    bool verifyChain() const {
        if (head == nullptr) return true;
        
        VoteBlock* current = head;
        string expectedPrevHash = genesisHash;
        
        while (current != nullptr) {
            // Verify block hash
            if (!current->verify()) {
                return false;
            }
            
            // Verify link to previous block
            if (current->previousHash != expectedPrevHash) {
                return false;
            }
            
            expectedPrevHash = current->voteHash;
            current = current->next;
        }
        return true;
    }
    
    int countVotesForCandidate(const string& candidate) const {
        int count = 0;
        VoteBlock* current = head;
        while (current != nullptr) {
            if (current->candidate == candidate) {
                count++;
            }
            current = current->next;
        }
        return count;
    }
    
    void displayChainHealth() const {
        cout << "\n╔════════════════════════════════════════╗\n";
        cout << "║     ANONYMOUS VOTE CHAIN HEALTH       ║\n";
        cout << "╚════════════════════════════════════════╝\n";
        
        cout << "  Total Votes: " << blockCount << "\n";
        cout << "  Chain Integrity: ";
        
        if (verifyChain()) {
            cout << "[VERIFIED]\n";
            cout << "  Tamper Detection: [ACTIVE]\n";
            cout << "  Anonymity: [PRESERVED]\n";
        } else {
            cout << "[COMPROMISED]\n";
            cout << "  Tamper Detection: [ALERT]\n";
            cout << "  Action Required: Audit needed\n";
        }
        cout << "\n";
    }
    
    int getTotalVotes() const { return blockCount; }
};

// ==================== CANDIDATE MANAGER ====================

struct Candidate {
    string name;
    int voteCount;
    Candidate* next;
    
    Candidate(const string& n) : name(n), voteCount(0), next(nullptr) {}
};

class CandidateManager {
private:
    Candidate* head;
    int candidateCount;
    
    Candidate* findCandidate(const string& name) const {
        Candidate* current = head;
        while (current != nullptr) {
            if (current->name == name) {
                return current;
            }
            current = current->next;
        }
        return nullptr;
    }
    
public:
    CandidateManager() : head(nullptr), candidateCount(0) {}
    
    ~CandidateManager() {
        clear();
    }
    
    void clear() {
        Candidate* current = head;
        while (current != nullptr) {
            Candidate* next = current->next;
            delete current;
            current = next;
        }
        head = nullptr;
        candidateCount = 0;
    }
    
    bool addCandidate(const string& name) {
        if (!isValidName(name) || findCandidate(name) != nullptr) {
            return false;
        }
        
        Candidate* newCandidate = new Candidate(name);
        newCandidate->next = head;
        head = newCandidate;
        candidateCount++;
        return true;
    }
    
    bool recordVote(const string& name) {
        Candidate* candidate = findCandidate(name);
        if (candidate == nullptr) {
            return false;
        }
        candidate->voteCount++;
        return true;
    }
    
    void displayResults() const {
        cout << "\n╔════════════════════════════════════════╗\n";
        cout << "║         ELECTION RESULTS              ║\n";
        cout << "╚════════════════════════════════════════╝\n";
        
        Candidate* current = head;
        int totalVotes = 0;
        
        // First pass: calculate total votes
        while (current != nullptr) {
            totalVotes += current->voteCount;
            current = current->next;
        }
        
        // Second pass: display results
        current = head;
        while (current != nullptr) {
            double percentage = (totalVotes > 0) ? 
                (current->voteCount * 100.0) / totalVotes : 0.0;
            
            cout << "  " << setw(20) << left << current->name 
                 << ": " << setw(5) << current->voteCount << " votes"
                 << " (" << fixed << setprecision(2) << setw(6) << percentage << "%)\n";
            current = current->next;
        }
        
        cout << "─────────────────────────────────────────\n";
        cout << "  Total Votes Cast: " << totalVotes << "\n";
        cout << "  Total Candidates: " << candidateCount << "\n\n";
    }
    
    string getWinner() const {
        Candidate* current = head;
        string winner = "No winner";
        int maxVotes = -1;
        
        while (current != nullptr) {
            if (current->voteCount > maxVotes) {
                maxVotes = current->voteCount;
                winner = current->name;
            }
            current = current->next;
        }
        return winner;
    }
    
    bool candidateExists(const string& name) const {
        return findCandidate(name) != nullptr;
    }
    
    int getCandidateCount() const { return candidateCount; }
};

// ==================== VOTING SYSTEM CORE ====================

class SecureVotingSystem {
private:
    VoterHashTable voterDB;
    AnonymousVoteChain voteChain;
    CandidateManager candidates;
    
    // Admin password (hashed)
    string adminPasswordHash;
    
    // Election state
    bool electionActive;
    time_t electionStartTime;
    time_t electionEndTime;
    
    bool verifyAdminPassword(const string& password) const {
        string inputHash = generateSecureHash(password);
        return inputHash == adminPasswordHash;
    }
    
    // Helper function for printing headers (const version)
    void printHeader(const string& title) const {
        cout << "\n╔════════════════════════════════════════╗\n";
        cout << "║  " << setw(38) << left << title << "║\n";
        cout << "╚════════════════════════════════════════╝\n";
    }
    
public:
    SecureVotingSystem() : electionActive(false) {
        // Set default admin password (should be changed in real system)
        adminPasswordHash = generateSecureHash("Admin@123");
        electionStartTime = electionEndTime = 0;
    }
    
    void initializeElection(const vector<string>& candidateNames, 
                           int durationHours = 24) {
        if (electionActive) {
            cout << "[ERROR] Election is already active!\n";
            return;
        }
        
        printHeader("INITIALIZING ELECTION");
        
        // Clear previous data
        voteChain.clear();
        candidates.clear();
        
        // Add candidates
        for (const string& name : candidateNames) {
            if (candidates.addCandidate(name)) {
                cout << "  Added candidate: " << name << "\n";
            }
        }
        
        // Set election timeframe
        electionStartTime = time(nullptr);
        electionEndTime = electionStartTime + (durationHours * 3600);
        electionActive = true;
        
        cout << "\n[SUCCESS] Election initialized!\n";
        cout << "  Start: " << ctime(&electionStartTime);
        cout << "  End:   " << ctime(&electionEndTime);
        cout << "  Duration: " << durationHours << " hours\n\n";
    }
    
    bool registerVoter(const string& voterID, const string& name, const string& pin) {
        if (!electionActive) {
            cout << "[ERROR] No active election\n";
            return false;
        }
        
        if (voterDB.registerVoter(voterID, name, pin)) {
            cout << "[SUCCESS] Voter registered: " << name << " (ID: " << voterID << ")\n";
            return true;
        }
        cout << "[ERROR] Registration failed. Check ID/name format or duplicate ID.\n";
        return false;
    }
    
    bool castVote(const string& voterID, const string& pin, const string& candidate) {
        if (!electionActive) {
            cout << "[ERROR] No active election\n";
            return false;
        }
        
        time_t now = time(nullptr);
        if (now > electionEndTime) {
            cout << "[ERROR] Election has ended\n";
            electionActive = false;
            return false;
        }
        
        // Authenticate voter
        if (!voterDB.authenticateVoter(voterID, pin)) {
            cout << "[ERROR] Authentication failed. Check ID/PIN or if already voted.\n";
            return false;
        }
        
        // Verify candidate exists
        if (!candidates.candidateExists(candidate)) {
            cout << "[ERROR] Invalid candidate\n";
            return false;
        }
        
        // Record vote in voter database
        if (!voterDB.castVote(voterID)) {
            cout << "[ERROR] Failed to record vote\n";
            return false;
        }
        
        // Add anonymous vote to chain
        if (!voteChain.addVote(candidate)) {
            cout << "[ERROR] Failed to add vote to secure chain\n";
            return false;
        }
        
        // Update candidate vote count
        candidates.recordVote(candidate);
        
        cout << "[SUCCESS] Vote cast for " << candidate << "!\n";
        cout << "          Your vote is recorded anonymously.\n";
        return true;
    }
    
    void displayCurrentResults() const {
        if (!electionActive) {
            cout << "[INFO] No active election\n";
            return;
        }
        
        printHeader("CURRENT ELECTION RESULTS");
        candidates.displayResults();
        
        // Show election time remaining
        time_t now = time(nullptr);
        if (now < electionEndTime) {
            int remaining = (electionEndTime - now) / 60; // minutes
            cout << "  Time remaining: " << remaining << " minutes\n";
        }
    }
    
    void endElection(const string& adminPassword) {
        if (!verifyAdminPassword(adminPassword)) {
            cout << "[ERROR] Admin authentication failed!\n";
            return;
        }
        
        if (!electionActive) {
            cout << "[INFO] No active election to end\n";
            return;
        }
        
        electionActive = false;
        printHeader("ELECTION FINALIZED");
        
        // Final results
        candidates.displayResults();
        
        // Chain verification
        voteChain.displayChainHealth();
        
        // Statistics
        cout << "  Total Registered Voters: " << voterDB.getTotalVoters() << "\n";
        cout << "  Total Votes Cast: " << voterDB.getVotedCount() << "\n";
        cout << "  Winner: " << candidates.getWinner() << "\n";
        
        // Verify no tampering
        if (!voteChain.verifyChain()) {
            cout << "\n[WARNING] Vote chain integrity check failed!\n";
            cout << "          Election results may be compromised.\n";
        }
    }
    
    void auditSystem(const string& adminPassword) {
        if (!verifyAdminPassword(adminPassword)) {
            cout << "[ERROR] Admin authentication failed!\n";
            return;
        }
        
        printHeader("SYSTEM AUDIT REPORT");
        
        cout << "\n[1] VOTER DATABASE:\n";
        voterDB.displayStatistics();
        
        cout << "\n[2] VOTE CHAIN INTEGRITY:\n";
        voteChain.displayChainHealth();
        
        cout << "\n[3] ELECTION STATUS:\n";
        if (electionActive) {
            time_t now = time(nullptr);
            cout << "  Status: [ACTIVE]\n";
            cout << "  Time elapsed: " << (now - electionStartTime) / 60 << " minutes\n";
            cout << "  Time remaining: " << (electionEndTime - now) / 60 << " minutes\n";
        } else {
            cout << "  Status: [INACTIVE]\n";
        }
        
        cout << "\n[4] DATA CONSISTENCY CHECK:\n";
        int dbVotes = voterDB.getVotedCount();
        int chainVotes = voteChain.getTotalVotes();
        
        if (dbVotes == chainVotes) {
            cout << "  [PASS] Voter database matches vote chain: " << dbVotes << " votes\n";
        } else {
            cout << "  [FAIL] Mismatch detected!\n";
            cout << "         Database: " << dbVotes << " votes\n";
            cout << "         Chain: " << chainVotes << " votes\n";
        }
    }
    
    bool isElectionActive() const { return electionActive; }
    
    void changeAdminPassword(const string& oldPass, const string& newPass) {
        if (!verifyAdminPassword(oldPass)) {
            cout << "[ERROR] Current password incorrect\n";
            return;
        }
        
        if (newPass.length() < 6) {
            cout << "[ERROR] New password must be at least 6 characters\n";
            return;
        }
        
        adminPasswordHash = generateSecureHash(newPass);
        cout << "[SUCCESS] Admin password changed\n";
    }
};

// ==================== USER INTERFACE ====================

void displayMainMenu() {
    cout << "\n┌────────────────────────────────────────┐\n";
    cout << "│     SECURE VOTING SYSTEM v2.0        │\n";
    cout << "├────────────────────────────────────────┤\n";
    cout << "│ 1. Register Voter                      │\n";
    cout << "│ 2. Cast Vote                           │\n";
    cout << "│ 3. View Current Results                │\n";
    cout << "│                                        │\n";
    cout << "│ 4. Admin: Initialize Election          │\n";
    cout << "│ 5. Admin: End Election                 │\n";
    cout << "│ 6. Admin: Audit System                 │\n";
    cout << "│ 7. Admin: Change Password              │\n";
    cout << "│                                        │\n";
    cout << "│ 0. Exit                                │\n";
    cout << "└────────────────────────────────────────┘\n";
    cout << "Select: ";
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));
    
    cout << "\n╔════════════════════════════════════════╗\n";
    cout << "║     SECURE VOTING SYSTEM v2.0         ║\n";
    cout << "║     (Student Project - Educational)    ║\n";
    cout << "╚════════════════════════════════════════╝\n";
    
    SecureVotingSystem votingSystem;
    
    // Sample initialization
    vector<string> defaultCandidates = {"Alice", "Bob", "Charlie", "Diana"};
    
    cout << "\n[INFO] System ready. No election active.\n";
    cout << "[INFO] Default candidates available: ";
    for (const auto& c : defaultCandidates) cout << c << " ";
    cout << "\n\n";
    
    int choice;
    string voterID, name, pin, candidate, adminPass, newPass;
    
    while (true) {
        displayMainMenu();
        
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "[ERROR] Invalid input\n";
            continue;
        }
        cin.ignore(); // Clear newline
        
        try {
            switch (choice) {
                case 1: // Register Voter
                    if (!votingSystem.isElectionActive()) {
                        cout << "[ERROR] No active election for registration\n";
                        break;
                    }
                    cout << "\n[VOTER REGISTRATION]\n";
                    cout << "Enter Voter ID (alphanumeric, 3-20 chars): ";
                    getline(cin, voterID);
                    cout << "Enter Full Name: ";
                    getline(cin, name);
                    cout << "Enter 4-digit PIN: ";
                    getline(cin, pin);
                    
                    votingSystem.registerVoter(voterID, name, pin);
                    break;
                    
                case 2: // Cast Vote
                    if (!votingSystem.isElectionActive()) {
                        cout << "[ERROR] No active election\n";
                        break;
                    }
                    cout << "\n[CAST VOTE]\n";
                    cout << "Enter Voter ID: ";
                    getline(cin, voterID);
                    cout << "Enter PIN: ";
                    getline(cin, pin);
                    cout << "Available candidates: Alice, Bob, Charlie, Diana\n";
                    cout << "Enter Candidate Name: ";
                    getline(cin, candidate);
                    
                    votingSystem.castVote(voterID, pin, candidate);
                    break;
                    
                case 3: // View Results
                    votingSystem.displayCurrentResults();
                    break;
                    
                case 4: // Initialize Election
                    cout << "\n[INITIALIZE ELECTION - ADMIN]\n";
                    cout << "Enter admin password: ";
                    getline(cin, adminPass);
                    
                    // In real system, get candidates from admin
                    cout << "Initializing election with default candidates...\n";
                    votingSystem.initializeElection(defaultCandidates, 2); // 2 hour election
                    break;
                    
                case 5: // End Election
                    cout << "\n[END ELECTION - ADMIN]\n";
                    cout << "Enter admin password: ";
                    getline(cin, adminPass);
                    votingSystem.endElection(adminPass);
                    break;
                    
                case 6: // Audit System
                    cout << "\n[SYSTEM AUDIT - ADMIN]\n";
                    cout << "Enter admin password: ";
                    getline(cin, adminPass);
                    votingSystem.auditSystem(adminPass);
                    break;
                    
                case 7: // Change Admin Password
                    cout << "\n[CHANGE ADMIN PASSWORD]\n";
                    cout << "Enter current password: ";
                    getline(cin, adminPass);
                    cout << "Enter new password (min 6 chars): ";
                    getline(cin, newPass);
                    votingSystem.changeAdminPassword(adminPass, newPass);
                    break;
                    
                case 0: // Exit
                    cout << "\n[EXIT] Thank you for using Secure Voting System!\n";
                    return 0;
                    
                default:
                    cout << "[ERROR] Invalid choice\n";
            }
            
            cout << "\nPress Enter to continue...";
            cin.ignore(10000, '\n');
            
        } catch (const exception& e) {
            cout << "[ERROR] " << e.what() << "\n";
        }
    }
    
    return 0;
}