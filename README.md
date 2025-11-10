# E Voting System with Blockchain Style Audit
A **console-based e-voting system** built entirely in **C++**, featuring object-oriented programming (OOP) principles, data structures (linked lists, binary search trees), file handling, and placeholders for **blockchain-based vote verification**.

This project simulates a secure, transparent, and auditable voting process — ideal for **academic OOP or data structure projects**.

---

## 📘 Features

### ✅ Voter Registration
- Add new voters with unique voter IDs.  
- Prevent duplicate registrations.  

### ✅ Candidate Management (BST)
- Candidates stored in a **Binary Search Tree (BST)** for efficient sorting and searching.  
- Supports vote counting, sorting, and result calculation.  

### ✅ Vote Casting
- Each voter can vote only once.  
- Validates both voter and candidate before accepting a vote.  

### ✅ Vote Ledger
- Linked list-based ledger that records each vote with timestamps.  
- Designed for future blockchain integration.  

### ✅ Election Results
- Sorted results with detailed analysis (rank, percentage, and statistics).  
- Export results to a text file.  

### ✅ Menu-Driven Interface
- Simple terminal interface for interaction and testing.  

---

## 🧩 Object-Oriented Design

| Component | Description |
|------------|-------------|
| **VoterDatabase** | Manages voter registration and lookup using a linked list. |
| **VoteLedger** | Stores vote records (linked list), includes timestamp. |
| **CandidateBST** | Binary Search Tree for managing candidates and votes. |
| **VotingSystem** | Main controller class integrating all subsystems. |

---

## 🧱 Planned Features (TODO Placeholders)

- [ ] **Blockchain Integration** for vote integrity (`verifyChain()`, `calculateHash()`).  
- [ ] **File Handling** for voter and ledger persistence (`saveToFile()`, `loadFromFile()`).  
- [ ] **System State Persistence** (`saveSystemState()`, `loadSystemState()`).  
- [ ] **Blockchain Audit System** (`auditBlockchain()`, `detectTampering()`).  
- [ ] **Exception Handling Improvements** for all critical operations.  

---

## 👨‍💻 Author

**Abbad Hasan**,**Talal Tariq**  
*E Voting System with Blockchain Style Audit*  
© 2025 All Rights Reserved
