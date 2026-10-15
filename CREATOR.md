# CREATOR.md — Complete Project Knowledge Document

# File Compression Tool Using Huffman Coding (C++17)

> **Purpose of this document:** After reading this, you should understand every decision, every algorithm, every line of logic — as if you built this project yourself. This is your viva bible.

---

## 1. THE CORE IDEA

### What problem are we solving?
Text files store each character as 1 byte (8 bits) regardless of how often it appears. The letter 'e' appears hundreds of times in English text but uses the same 8 bits as 'z' which barely appears. This is wasteful.

### What is our solution?
**Huffman Coding** — assign shorter binary codes to frequent characters and longer codes to rare ones. If 'e' appears 200 times and gets a 3-bit code instead of 8-bit, we save `200 × 5 = 1000 bits` just for one character.

### Why Huffman specifically?
- It's a **greedy algorithm** — always makes the locally optimal choice (combine two lowest frequencies)
- It produces an **optimal prefix-free code** — no code is a prefix of another, so decoding is unambiguous
- It's **lossless** — decompressed output is identical to the original, bit for bit

### Real-world analogy
Think of Morse code: 'E' = `.` (1 symbol), 'Q' = `--.-` (4 symbols). Most common letter gets shortest code. Huffman does exactly this but mathematically optimal.

---

## 2. PROJECT ARCHITECTURE

### File Structure
```
compression-tool/
├── HuffmanNode.h              ← Node structure (header only, no .cpp needed)
├── HuffmanTree.h / .cpp       ← Tree building, code generation, serialize/deserialize
├── FrequencyAnalyzer.h / .cpp ← Count character frequencies, display tables
├── BitStreamWriter.h / .cpp   ← Write individual bits to binary file
├── BitStreamReader.h / .cpp   ← Read individual bits from binary file
├── HuffmanCompressor.h / .cpp ← Main controller, orchestrates everything
├── main.cpp                   ← CLI menu, user interaction
├── input.txt                  ← Sample test file
└── README.md                  ← Usage documentation
```

### Why this structure?
Each class has **one responsibility** (Single Responsibility Principle):
- `HuffmanNode` → defines what a node IS
- `HuffmanTree` → manages the tree data structure
- `FrequencyAnalyzer` → handles frequency counting
- `BitStreamWriter/Reader` → handles bit-level I/O
- `HuffmanCompressor` → coordinates the workflow

### Include/Dependency Chain
```
main.cpp
  └── HuffmanCompressor.h
        ├── HuffmanTree.h
        │     └── HuffmanNode.h  ← uses <memory> for shared_ptr
        │         (also uses <map>, <string>, <queue>)
        └── FrequencyAnalyzer.h  ← uses <map>, <string>

HuffmanCompressor.cpp
  ├── BitStreamWriter.h  ← uses <fstream>, <string>
  └── BitStreamReader.h  ← uses <fstream>
```

**Key design decision:** `BitStreamWriter.h` and `BitStreamReader.h` are only included in `HuffmanCompressor.cpp`, NOT in the header. This keeps the header clean — other files don't need to know about bit streaming.

### Header Guards
Every `.h` file uses `#ifndef / #define / #endif` pattern to prevent double inclusion:
```cpp
#ifndef HUFFMAN_NODE_H
#define HUFFMAN_NODE_H
// ... content ...
#endif
```

---

## 3. STANDARD LIBRARY HEADERS USED

| Header | Where Used | Why |
|--------|-----------|-----|
| `<memory>` | HuffmanNode.h | `shared_ptr` for automatic memory management of tree nodes |
| `<map>` | HuffmanTree.h, FrequencyAnalyzer.h | `map<char,int>` for frequencies, `map<char,string>` for codes |
| `<string>` | Most files | `std::string` for text, codes, serialized tree |
| `<queue>` | HuffmanTree.h/.cpp | `priority_queue` for building the min-heap |
| `<fstream>` | BitStream files, HuffmanCompressor.cpp | `ifstream/ofstream` for file I/O |
| `<iostream>` | Most .cpp files | `cout` for output display |
| `<iomanip>` | FrequencyAnalyzer.cpp, HuffmanCompressor.cpp | `setw`, `setprecision`, `left` for formatted tables |
| `<sstream>` | HuffmanCompressor.cpp | `stringstream` to read entire file into string |
| `<limits>` | main.cpp | `numeric_limits` for input validation with `cin.ignore()` |

---

## 4. DATA STRUCTURES USED (DSA Concepts)

### 4.1 Binary Tree (Huffman Tree)
- **Type:** Full binary tree (every node has 0 or 2 children)
- **Leaf nodes:** Store actual characters + frequency
- **Internal nodes:** Store combined frequency of children, character = `'\0'`
- **Property:** Higher frequency characters are closer to root = shorter code

### 4.2 Min-Heap via Priority Queue
```cpp
std::priority_queue<shared_ptr<HuffmanNode>,
                    vector<shared_ptr<HuffmanNode>>,
                    CompareNodes> minHeap;
```
- C++ `priority_queue` is a **max-heap** by default
- We use custom comparator `CompareNodes` that returns `a->frequency > b->frequency`
- This **inverts** the comparison making it a **min-heap**
- Result: `.top()` always gives the node with **lowest** frequency

**Why min-heap?** Huffman algorithm needs to repeatedly extract the two minimum-frequency nodes. Min-heap gives O(log n) extraction.

### 4.3 Map (Hash Table with ordering)
- `map<char, int>` — character to frequency count
- `map<char, string>` — character to binary code string
- `std::map` uses Red-Black Tree internally → O(log n) operations, keys stay sorted

### 4.4 Smart Pointers
```cpp
std::shared_ptr<HuffmanNode>
```
- Used instead of raw pointers (`HuffmanNode*`)
- **Why:** Automatic memory deallocation when no more references exist
- **Why shared_ptr over unique_ptr:** Multiple references possible (parent, priority queue, traversal variables all can point to same node)

---

## 5. ALGORITHMS — DEEP DIVE

### 5.1 Huffman Tree Construction Algorithm

**Type:** Greedy Algorithm
**Time Complexity:** O(n log n) where n = unique characters
**Space Complexity:** O(n)

**Steps:**
1. Create one leaf node per unique character, push all into min-heap
2. While heap has more than 1 node:
   - Pop two nodes with smallest frequency (left, right)
   - Create parent node with `freq = left.freq + right.freq`
   - Set parent's children to left and right
   - Push parent back into heap
3. Last remaining node = root

**Example with "aabbc":**
```
Frequencies: a=2, b=2, c=1
Step 1: Heap = [c(1), a(2), b(2)]
Step 2: Pop c(1) and a(2), create parent(3), push back
        Heap = [b(2), parent(3)]
Step 3: Pop b(2) and parent(3), create root(5)
        Heap = [root(5)] → done!

Tree:        (5)
            /   \
          b(2)  (3)
               /   \
             c(1)  a(2)

Codes: b=0, c=10, a=11
```

**Edge case handled:** If only 1 unique character exists (e.g., "aaaa"), we create a dummy parent so the character still gets code "0".

### 5.2 Code Generation (Tree Traversal)

**Algorithm:** Recursive DFS (Depth-First Search) — Preorder
**Rule:** Going left = append '0', Going right = append '1'

```
generateCodesHelper(node, currentCode):
    if node is NULL: return
    if node is leaf:
        codeMap[node.character] = currentCode
        return
    generateCodesHelper(node.left,  currentCode + "0")
    generateCodesHelper(node.right, currentCode + "1")
```

**Why this produces prefix-free codes:** Characters are ONLY at leaf nodes. To reach leaf A, you follow a specific path. No other leaf's path is a prefix of A's path because you'd have to pass through A (a leaf with no children) to continue.

### 5.3 Tree Serialization (Preorder Traversal)

**Purpose:** Save tree structure in compressed file so decompressor can rebuild it.

**Format:**
- Leaf node → write `'1'` then the character byte
- Internal node → write `'0'` then recurse left, recurse right

**Example:**
```
Tree:    (*)
        /   \
      'a'   (*)
           /   \
         'b'   'c'

Serialized: "01a01b1c"
             0 = internal, 1a = leaf 'a', 0 = internal, 1b = leaf 'b', 1c = leaf 'c'
```

**Deserialization:** Read character by character:
- If `'1'`: next char is the leaf character, create leaf node
- If `'0'`: recursively build left subtree, then right subtree, create internal node

**Why preorder?** Preorder (root→left→right) naturally lets us reconstruct the tree top-down without needing any delimiters.

### 5.4 Bit-Level I/O

**The problem:** Files work in bytes (8 bits). Huffman codes are variable-length (3 bits, 5 bits, etc.).

**BitStreamWriter solution — accumulate bits into a buffer byte:**
```
buffer = 00000000, bitCount = 0

writeBit(1): buffer = (00000000 << 1) | 1 = 00000001, bitCount = 1
writeBit(0): buffer = (00000001 << 1) | 0 = 00000010, bitCount = 2
writeBit(1): buffer = (00000010 << 1) | 1 = 00000101, bitCount = 3
... continues until bitCount == 8, then write byte to file, reset
```

**Padding problem:** If total encoded bits aren't divisible by 8, the last byte has leftover bits. We pad with zeros.
- Example: 5 bits remaining → pad 3 zeros → write byte
- We store the **total character count** in the file so decompressor knows when to stop (doesn't decode padding as characters)

**BitStreamReader — extract bits from MSB to LSB:**
```
readBit():
    if buffer empty: read next byte from file
    bit = (buffer >> 7) & 1    // extract MSB
    buffer = buffer << 1       // shift left for next call
    bitCount--
    return bit
```

**Why MSB first?** Because BitStreamWriter fills bits left-to-right (shifting left and ORing). So reader must also read left-to-right.

---

## 6. COMPRESSED FILE FORMAT

```
Byte Layout:
┌──────────────────┬──────────────────┬──────────────────┬──────────────────┐
│  Tree Size       │  Serialized      │  Char Count      │  Encoded Binary  │
│  (4 bytes)       │  Tree Data       │  (4 bytes)       │  Data            │
│  little-endian   │  (variable)      │  little-endian   │  (variable)      │
└──────────────────┴──────────────────┴──────────────────┴──────────────────┘
```

### Byte-by-byte breakdown:

**Bytes 0-3: Tree header size (uint32, little-endian)**
```cpp
outFile.put((treeSize >> 0)  & 0xFF);  // least significant byte first
outFile.put((treeSize >> 8)  & 0xFF);
outFile.put((treeSize >> 16) & 0xFF);
outFile.put((treeSize >> 24) & 0xFF);  // most significant byte last
```
Little-endian means least significant byte is stored first. Example: size 300 (0x012C) → bytes: `2C 01 00 00`

**Bytes 4 to (4+treeSize-1): Serialized tree**
The preorder-serialized tree string written directly.

**Next 4 bytes: Total character count (uint32, little-endian)**
Tells decompressor exactly how many characters to decode. This solves the padding problem — we stop after decoding this many characters, ignoring any padding bits.

**Remaining bytes: Encoded binary data**
The actual Huffman-encoded content as packed bits.

### Why this format?
1. Tree size first → decompressor knows exactly how many bytes to read for the tree
2. Character count → decompressor knows when to stop (no need to store padding count separately)
3. Everything self-contained → no external codebook needed

---

## 7. FILE-BY-FILE DEEP BREAKDOWN

### 7.1 HuffmanNode.h (Header-only, no .cpp)

**Why no .cpp?** The class is simple enough that everything is defined inline. No complex method bodies.

**Two constructors:**
```cpp
HuffmanNode(char ch, int freq)          // For leaf nodes
HuffmanNode(int freq, shared_ptr left, shared_ptr right)  // For internal nodes
```
Internal nodes use `'\0'` as character — a sentinel value indicating "this is not a real character."

**`isLeaf()` method:** Returns true if both children are nullptr. This is the check used everywhere to determine if we've reached an actual character during traversal.

**`CompareNodes` struct:** A functor (function object) used as the third template parameter of `priority_queue`. The `operator()` returns `a->freq > b->freq` which makes the priority queue behave as a min-heap.

### 7.2 HuffmanTree.h / HuffmanTree.cpp

**Private members:**
- `root` — shared_ptr to root node
- `codeMap` — map<char, string> storing generated codes

**Private helper methods (recursive):**
- `generateCodesHelper()` — DFS traversal building code strings
- `displayTreeHelper()` — prints tree with visual connectors (`|--`, `` `-- ``)
- `serializeHelper()` — preorder traversal writing '0'/'1' markers
- `deserializeHelper()` — rebuilds tree from serialized string using an index reference

**Why pass `int& index` by reference in deserialize?** Because as we recursively build left and right subtrees, the index must advance globally. If passed by value, left subtree processing wouldn't advance the index for the right subtree.

**`buildTree()` special cases:**
1. Empty frequency map → set root to nullptr
2. Single character → create dummy parent with that char as left child, so it gets code "0"

### 7.3 FrequencyAnalyzer.h / FrequencyAnalyzer.cpp

**`calculateFrequency()`:** Single O(n) loop through text, incrementing `frequencyMap[ch]++` for each character. The `++` operator on map creates the entry with value 0 if it doesn't exist, then increments.

**`setCodeMap()`:** Called AFTER tree is built. Stores codes so `displayFrequencyTable()` can show character, frequency, AND code side by side.

**Display handling for special chars:**
- `'\n'` → displays as `\n`
- `' '` → displays as `SPACE`
- `'\t'` → displays as `\t`
- `'\r'` → displays as `\r`

### 7.4 BitStreamWriter.h / BitStreamWriter.cpp

**Members:**
- `outputFile` — reference to ofstream (not a copy, using `&`)
- `buffer` — unsigned char accumulating bits
- `bitCount` — how many bits currently in buffer (0-7)

**`writeBit(int bit)`:** Core operation.
```cpp
buffer = (buffer << 1) | (bit & 1);
```
- `(bit & 1)` ensures only the last bit matters (safety)
- Left shift makes room, OR inserts the new bit

**`writeBitString()`:** Converts string like `"10110"` to actual bits using `writeBit(c - '0')`. The `- '0'` converts ASCII '0'(48) or '1'(49) to integer 0 or 1.

**`flush()`:** Called once at the end.
```cpp
paddingBits = 8 - bitCount;
buffer = buffer << paddingBits;  // shift existing bits to MSB positions
```

### 7.5 BitStreamReader.h / BitStreamReader.cpp

**`readBit()`:** Mirror of writeBit.
```cpp
int bit = (buffer >> 7) & 1;  // extract MSB
buffer = buffer << 1;          // shift for next read
```
Reads MSB first because that's the order bits were written.

**Returns -1 on EOF** — this is the signal to stop reading.

### 7.6 HuffmanCompressor.h / HuffmanCompressor.cpp

**The orchestrator.** Contains instances of `HuffmanTree` and `FrequencyAnalyzer` as private members. Creates `BitStreamWriter/Reader` locally in compress/decompress methods.

**`compress()` flow:**
1. Open file → `ifstream` with `ios::binary` flag
2. Read all content → `stringstream << inFile.rdbuf()` trick
3. `analyzer.calculateFrequency(fileContent)`
4. `huffTree.buildTree(freqMap)`
5. `huffTree.generateCodes()`
6. Build encoded string by concatenating codes for each char
7. Write header (tree size + tree + char count)
8. `BitStreamWriter::writeBitString(encodedBits)` + `flush()`

**`decompress()` flow:**
1. Read 4 bytes → tree size
2. Read treeSize bytes → deserialize tree
3. Read 4 bytes → total character count
4. Create `BitStreamReader`, start from root
5. Loop: read bit → go left(0) or right(1) → if leaf, output char, reset to root
6. Stop when `charsDecoded == totalChars`

**Stats calculation:**
```cpp
ratio = (1.0 - (compressedSize / originalSize)) * 100.0;
```
A ratio of 36% means the compressed file is 36% smaller.

### 7.7 main.cpp

**Menu loop:** `while(true)` with `switch(choice)` and `case 0` for exit via `return 0`.

**Input validation:**
```cpp
if (!(std::cin >> choice)) {
    std::cin.clear();                             // clear error flags
    std::cin.ignore(numeric_limits<streamsize>::max(), '\n');  // discard bad input
}
```
This prevents infinite loops when user types a letter instead of a number.

**Quote stripping:** If user pastes a path like `"input.txt"`, the surrounding quotes are removed. Practical UX touch.

**Default filenames:**
- Compress: `input.txt` → `input.huff`
- Decompress: `input.huff` → `input_decoded.txt`

---

## 8. HOW COMPRESSION ACTUALLY SAVES SPACE

### The Math
Original: Each character = 8 bits (ASCII)
After Huffman: Each character = variable bits based on frequency

For text "aaabbc" (6 chars):
```
Original:  6 × 8 = 48 bits

Huffman tree gives: a=0, b=10, c=11
Encoded: 0 0 0 10 10 11 = 10 bits
With header overhead: ~30 bits for tree

Total compressed: ~40 bits vs 48 bits original
```

For larger files, the header overhead becomes negligible and savings are significant. Our test achieved **36.56% compression** on a 4715-byte file.

### When compression is LESS effective
- Very small files (header overhead > savings)
- Files with uniform character distribution (all chars equally frequent → all codes same length ≈ 8 bits)
- Already compressed files (.zip, .jpg) — no redundancy left to exploit

---

## 9. EDGE CASES HANDLED

| Edge Case | Where Handled | How |
|-----------|--------------|-----|
| Empty file | `compress()` | Returns false with error message |
| File not found | `compress()`, `decompress()` | Checks `is_open()`, returns false |
| Single unique character (e.g., "aaaa") | `buildTree()` | Creates dummy parent node |
| Special characters (`\n`, `\t`, space) | Display functions | Mapped to readable strings |
| Non-numeric menu input | `main.cpp` | `cin.clear()` + `cin.ignore()` |
| Quoted file paths | `main.cpp` | Strips surrounding `"` characters |
| Corrupted compressed file | `decompress()` | Checks `gcount()` after reads |
| Padding bits at end | File format | Character count tells when to stop decoding |

---

## 10. OOP CONCEPTS DEMONSTRATED

| Concept | Where |
|---------|-------|
| **Encapsulation** | Private members in every class, public interface methods |
| **Abstraction** | `HuffmanCompressor::compress()` hides all complexity behind one call |
| **Constructor Overloading** | `HuffmanNode` has two constructors (leaf vs internal) |
| **Composition** | `HuffmanCompressor` HAS-A `HuffmanTree` and HAS-A `FrequencyAnalyzer` |
| **Const Correctness** | Display methods marked `const`, parameters passed as `const&` |
| **RAII** | `shared_ptr` manages memory automatically |
| **Separation of Concerns** | Each class handles exactly one thing |
| **Header/Implementation Split** | `.h` for declaration, `.cpp` for definition |

---

## 11. COMPLEXITY ANALYSIS

| Operation | Time | Space |
|-----------|------|-------|
| Frequency counting | O(n) | O(k) where k = unique chars |
| Building Huffman tree | O(k log k) | O(k) |
| Generating codes | O(k) | O(k) |
| Encoding text | O(n) | O(n) for encoded string |
| Writing compressed file | O(n) | O(1) buffer |
| Decompression | O(n × h) | O(k) where h = tree height |

**Overall compression: O(n + k log k) ≈ O(n log n)**
**Overall decompression: O(n × h)** where h is bounded by k, typically O(n log k)

---

## 12. COMPILATION AND EXECUTION

### Compile Command
```bash
g++ -std=c++17 -o compress main.cpp HuffmanTree.cpp FrequencyAnalyzer.cpp BitStreamWriter.cpp BitStreamReader.cpp HuffmanCompressor.cpp
```

**Why C++17?** We use `std::shared_ptr`, structured bindings compatibility, and `uint32_t` which are well-supported in C++17.

**Why list specific .cpp files?** We don't compile `.h` files — they're included by `#include` directives. Only `.cpp` files produce object code.

**Why no HuffmanNode.cpp?** It's header-only. All methods are defined inline in the class declaration.

### Run
```bash
./compress        # Linux/Mac
.\compress.exe    # Windows
```

---

## 13. VERIFIED TEST RESULTS

```
Original Size     : 4715 bytes
Compressed Size   : 2991 bytes
Compression Ratio : 36.56%
Unique Characters : 95
Decompression     : PASS (byte-for-byte identical)
```

The sample `input.txt` contains English paragraphs, special characters (`@#$%^&*()`), numbers (0-9), uppercase and lowercase letters, and pangrams — covering all ASCII edge cases.

---

## 14. POSSIBLE VIVA QUESTIONS AND ANSWERS

**Q: Why is Huffman coding called a greedy algorithm?**
A: Because at each step, we greedily pick the two nodes with the lowest frequency to combine. This local optimal choice leads to a globally optimal solution (proven by the Huffman optimality theorem).

**Q: How do you ensure no code is a prefix of another?**
A: Characters exist only at leaf nodes. Since no leaf is an ancestor of another leaf, no code can be a prefix of another code. This is called a prefix-free code.

**Q: Why use shared_ptr instead of raw pointers?**
A: To avoid memory leaks. When the tree is destroyed, shared_ptr automatically deallocates all nodes when their reference count reaches zero. No need for explicit `delete`.

**Q: How does the decompressor know where the tree ends and data begins?**
A: The first 4 bytes of the compressed file store the tree header size. The decompressor reads exactly that many bytes for the tree, then reads the next 4 bytes for the character count, and everything after is encoded data.

**Q: What happens if the encoded bits don't fill the last byte completely?**
A: We pad with zero bits. The decompressor doesn't get confused because it uses the stored character count to stop — it never tries to decode padding bits as characters.

**Q: Why is the priority queue comparison inverted?**
A: C++ `priority_queue` is a max-heap by default (largest element on top). We need a min-heap (smallest on top) for Huffman. By returning `a->freq > b->freq`, we tell the queue that higher frequency means LOWER priority, effectively inverting it to a min-heap.

**Q: Can this compress images or videos?**
A: Technically yes, since it works on raw bytes. But it won't be effective because those formats are already compressed. Huffman works best on data with non-uniform frequency distributions, like text.

**Q: What's the worst-case compression ratio?**
A: If all characters have equal frequency, each gets a code of roughly log2(k) bits. If k=256 (all ASCII chars equally likely), codes are ~8 bits each — same as original. With header overhead, the "compressed" file would actually be LARGER.

**Q: Explain the bit manipulation in writeBit.**
A: `buffer = (buffer << 1) | (bit & 1)`. The left shift moves existing bits one position left, making room at the rightmost position. The OR operation places the new bit there. `bit & 1` is a safety mask ensuring only 0 or 1 is used.

**Q: Why serialize using preorder traversal specifically?**
A: Preorder (root→left→right) lets us reconstruct the tree top-down. When deserializing, we read the root marker first, then recursively build the left subtree (consuming its markers), then the right subtree. No additional data structures or delimiters needed.

---

*This document covers the complete project. Read it, understand the flow, trace through the algorithms with small examples, and you'll be able to explain every line in your viva confidently.*
