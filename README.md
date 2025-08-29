# supermarket-management-c
Console based supermarket back office project written in C.  
Implements products, customers (with club members), shopping carts, sorting & searching, and persistent storage.  
Focuses on **OOP-style design in C** (structs + function pointers), a **generic linked list (void* payload)**, and file I/O (binary + text).  

---

## ✨ Features
  - **Generic Linked List (void*)**
  - Insert-sorted cart by barcode (no post-sort needed).  
  - Destructor callbacks to prevent leaks/double-free.  

- **Customers & Club Members (Polymorphism in C)**  
  - Base `Customer` struct, `ClubMember` extends via first-field embedding.  
  - Discount rules applied via function pointers.  

- **Products: Sort & Search**  
  - Sort by Name / Quantity / Price with `qsort`.  
  - Search with `bsearch` (only when array is sorted by the same key).  

- **Persistent Storage**  
  - Binary file `SuperMarket.bin`: products.  
  - Text file `Customers.txt`: customers.  
  - Safe, length-prefixed strings for portability.  

---

## 🛠️ Technologies
- C (structs, pointers, function pointers)  
- File I/O (binary & text)  
- Algorithms: `qsort`, `bsearch`  
- Visual Studio (Windows)  

---

## ▶️ How to Run
1. Clone the repository:
   ```bash
   git clone https://github.com/YuvalKandov/supermarket-management-c.git
Open in Visual Studio (Windows).

Build & Run.

On first run, if files are missing/corrupt → initialize via console.

On exit → supermarket & customers are saved to files.

## 📚 What I Learned
Emulating OOP in C with composition, embedding, and function pointers.

Designing generic containers with void* + safe destruction policies.

Using qsort/bsearch with custom comparators.

File persistence: binary vs. text formats, defensive error handling.

Writing modular, memory-safe C code.
