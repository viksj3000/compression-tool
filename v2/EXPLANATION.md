# Huffman Compression - Interview Guide & Deep Dive

This document explains every single concept used in the single-file `v2/main.cpp` Huffman Compression tool. It is designed to act as a study guide for understanding Data Structures, Algorithms, Systems Programming, and C++ specific mechanics present in the code.

## 1. What is Huffman Coding?
Huffman Coding is a **lossless data compression algorithm**. The core idea is to assign variable-length codes to input characters, with shorter codes assigned to more frequent characters. 
- A standard ASCII character always takes exactly 8 bits (1 byte) of space.
- In Huffman coding, a very common character like 'e' might be assigned a code like `10` (only 2 bits), while a rare character like 'Z' might be assigned `110101` (6 bits). Over a large file, this saves massive amounts of space.

## 2. Core Data Structures & Algorithms Used

### A. Hash Map (`std::unordered_map`)
- **Use Case in Code:** Tracking character frequencies and storing the generated prefix codes.
- **Why?** An `unordered_map` provides **$O(1)$** average time complexity for lookups and insertions. This allows extremely fast counting during the first pass of the file: `characterFrequencies[ch]++`.

### B. Min-Heap / Priority Queue (`std::priority_queue`)
- **Use Case in Code:** Building the Huffman Tree from the character frequencies.
- **Why?** To build a Huffman Tree, we always need to merge the two nodes with the lowest frequencies. A Min-Heap automatically keeps the smallest elements at the top, allowing **$O(\log N)$** retrieval of the minimum element, and **$O(\log N)$** insertion of the merged node.
- **Custom Comparator (`CompareNodes`):** C++'s priority queue acts as a Max-Heap by default. We implemented a custom `operator()` that returns `true` when the left node is greater than the right node. This flips the heap into a Min-Heap.

### C. Binary Trees (The Huffman Tree)
- **Use Case in Code:** A custom `Node` struct storing a character, its frequency, and `left`/`right` pointers.
- **Tree Properties:** 
  - Every character exists *only* in a **leaf node** (a node with no children). 
  - Moving `left` down the tree represents a `0` bit.
  - Moving `right` down the tree represents a `1` bit.
- **Prefix Property:** Because characters are only at the leaves, no code is a prefix of another code. This makes decoding continuous bitstreams deterministic and unambiguous.

## 3. Systems Programming Concepts

### A. Binary I/O (`std::ios::binary`)
- **Use Case in Code:** Reading and writing `.huff` and `.txt` files.
- **Why?** If a file is opened in text mode, the OS might auto-convert newline characters (`\n` to `\r\n` on Windows) or stop reading when it encounters an EOF byte (`0x1A`). Since compression results in arbitrary bits (not readable ASCII text), we *must* read/write raw bytes continuously using `ios::binary`.

### B. Bitwise Operations (Bit Manipulation)
Standard streams operate on `bytes` (8 bits), but Huffman codes are jagged (e.g., 3 bits, 5 bits). We have to pack individual bits together into full bytes before writing to a file.
- **Bitwise Left Shift (`<<`):** 
  `bitBuffer = (bitBuffer << 1) | (bit - '0');`
  Shifts existing bits left to make room for a new 1 or 0 (using bitwise OR `|`).
- **Bitwise AND (`&`):** 
  `int bitValue = (byte >> i) & 1;`
  In decompression, we shift to the right by `i` positions and mask it with `1` to isolate exactly one bit to determine if we should traverse left or right in our tree.

### C. Padding the Output
Since we are clustering bits manually, the total number of bits for a file might not be perfectly divisible by 8.
- **Solution:** At the end of compression, if `bitsInCurrentByte` is for example `3`, we left-shift the remaining bits to the top of the byte `bitBuffer <<= (8 - bitsInCurrentByte)` and write it out.
- **The Decoded Size Safety:** Because of padded bits (extra 0s at the end), decompression might decode an extra phantom character. We prevent this by writing the `originalSize` of the file (e.g., 4715 bytes) into the header. During decompression, we stop exactly when `charactersWritten == originalSize`.

## 4. Tree Serialization (Pre-Order Traversal)
- **Problem:** To decompress the file later, the extractor needs to know the exact Huffman Tree architecture that was used to compress it. We can't just write RAM pointers to a file.
- **Solution:** We use **Pre-Order Traversal** to flatten the tree into a string.
  - If we hit an internal node, we append a `'0'` to the string, then explore left, then right.
  - If we hit a leaf node, we append a `'1'` to the string, immediately followed by the actual character.
- **Deserialization:** Reading back the string is a recursive mirroring process. If it sees a `'1'`, it grabs the next character and constructs a leaf node. If it sees a `'0'`, it knows to spawn an internal node and recursively process the left and right children.

## 5. Potential Interview Questions

**Q1: What is the Time Complexity of your program?**
- **Analysis (Counting Frequencies):** $O(N)$ where $N$ is the number of characters in the file.
- **Building the Tree:** $O(K \log K)$ where $K$ is the number of unique characters.
- **Encoding file:** $O(N)$.
- **Overall Time Complexity:** $O(N \log K)$ — The bottleneck is building the tree.

**Q2: What is the Space Complexity?**
- **Overall Space Complexity:** $O(K)$ space required to store the frequency map, the Huffman tree, and the tree structure string in RAM. The bit writing utilizes an $O(1)$ constant buffer space.

**Q3: How do you prevent memory leaks?**
- I used a custom destructor in the `Node` struct: `~Node() { delete left; delete right; }`. When the root of the tree is deleted (in the class `~HuffmanV2()` destructor), it recursively propagates downwards and deletes every allocated child pointer in the tree, ensuring zero memory leaks without needing a complicated smart pointer structure.