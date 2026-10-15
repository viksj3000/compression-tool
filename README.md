# File Compression Tool - Huffman Coding

A command-line file compression and decompression tool built using **Huffman Coding Algorithm** in C++. This project demonstrates key Data Structures and Algorithms (DSA) concepts including binary trees, priority queues, greedy algorithms, and bit manipulation.

## 📌 About the Project

Huffman coding is a **lossless data compression** algorithm that assigns variable-length binary codes to characters based on their frequency of occurrence. Characters that appear more frequently get shorter codes, while less frequent characters get longer codes. This results in overall compression of the data.

### Key DSA Concepts Used

| Concept | Usage in Project |
|---------|-----------------|
| **Binary Tree** | Huffman tree stores characters at leaf nodes |
| **Priority Queue (Min-Heap)** | Used to build tree by always combining lowest frequency nodes |
| **Greedy Algorithm** | Huffman coding is a classic greedy approach |
| **Hash Map** | `std::map` for character-to-frequency and character-to-code mapping |
| **Recursion** | Tree traversal for code generation, serialization, and display |
| **Bit Manipulation** | BitStreamWriter/Reader handle bit-level file I/O |
| **File I/O** | Binary file reading and writing using `fstream` |
| **Smart Pointers** | `shared_ptr` for automatic memory management of tree nodes |

## 📂 Project Structure

```
compression-tool/
├── HuffmanNode.h            # Node class for Huffman tree
├── HuffmanTree.h            # Huffman tree class declaration
├── HuffmanTree.cpp          # Tree building, code generation, serialization
├── FrequencyAnalyzer.h      # Frequency counter class declaration
├── FrequencyAnalyzer.cpp    # Character frequency analysis
├── BitStreamWriter.h        # Bit-level file writer declaration
├── BitStreamWriter.cpp      # Writing individual bits to binary file
├── BitStreamReader.h        # Bit-level file reader declaration
├── BitStreamReader.cpp      # Reading individual bits from binary file
├── HuffmanCompressor.h      # Main compressor class declaration
├── HuffmanCompressor.cpp    # Compression and decompression logic
├── main.cpp                 # CLI menu and program entry point
├── input.txt                # Sample input file for testing
└── README.md                # This file
```

## 🔧 How to Compile

Make sure you have `g++` with C++17 support installed.

```bash
g++ -std=c++17 -o compress main.cpp HuffmanTree.cpp FrequencyAnalyzer.cpp BitStreamWriter.cpp BitStreamReader.cpp HuffmanCompressor.cpp
```

## 🚀 How to Run

```bash
./compress
```

This opens an interactive menu:

```
===== File Compression Tool (Huffman Coding) =====

  1. Compress a File
  2. Decompress a File
  3. View Frequency Table
  4. View Huffman Codes Table
  5. View Compression Stats
  6. View Huffman Tree Structure
  0. Exit

  Enter choice:
```

## 📖 How It Works

### Compression Process

1. **Read** the input text file
2. **Count** frequency of each character using `FrequencyAnalyzer`
3. **Build** Huffman tree using a min priority queue (`HuffmanTree`)
4. **Generate** binary codes for each character by traversing the tree
5. **Encode** the original text by replacing each character with its code
6. **Write** the compressed binary file using `BitStreamWriter`

### Compressed File Format

```
[4 bytes: tree header size] [Serialized Tree] [4 bytes: char count] [Encoded Binary Data]
```

- **Tree header size**: Tells decompressor how many bytes to read for the tree
- **Serialized tree**: The Huffman tree structure stored as a string
- **Char count**: Total characters in original file (to know when to stop decoding)
- **Encoded data**: The actual compressed binary data

### Decompression Process

1. **Read** the compressed file header
2. **Extract** and rebuild the Huffman tree from the header
3. **Read** encoded binary data bit by bit using `BitStreamReader`
4. **Traverse** the tree: bit 0 = go left, bit 1 = go right
5. At each leaf node, output the character and restart from root
6. **Write** the decoded text to the output file

## 📊 Example Output

### Compression Stats
```
  ---- Compression Stats ----
  Original File     : input.txt
  Compressed File   : input.huff
  Original Size     : 3500 bytes
  Compressed Size   : 2100 bytes
  Compression Ratio : 40.00%
  Characters Encoded: 3500
  Unique Characters : 75
  ---------------------------
```

### Huffman Codes Table
```
  ----------------------------------------
  Char     | Code        | Length
  ----------------------------------------
   SPACE   | 00          | 2 bits
   e       | 100         | 3 bits
   t       | 1010        | 4 bits
   a       | 1011        | 4 bits
   ...     | ...         | ...
  ----------------------------------------
```

## 🧪 Testing

1. Compress the sample input file:
   - Select option 1, enter `input.txt`
   - Output will be saved as `input.huff`

2. Decompress the compressed file:
   - Select option 2, enter `input.huff`
   - Output will be saved as `input_decoded.txt`

3. Verify the output matches the original:
   - Compare `input.txt` and `input_decoded.txt`
   - They should be identical

## ⚙️ Classes Overview

| Class | Responsibility |
|-------|---------------|
| `HuffmanNode` | Represents a single node (character, frequency, children) |
| `HuffmanTree` | Builds tree, generates codes, serializes/deserializes |
| `FrequencyAnalyzer` | Counts character frequencies, displays frequency table |
| `BitStreamWriter` | Writes individual bits to binary file with padding |
| `BitStreamReader` | Reads individual bits from binary file |
| `HuffmanCompressor` | Main controller - orchestrates compression/decompression |

## 📝 Notes

- Works with any text file containing ASCII characters
- Handles special characters, spaces, tabs, and newlines
- Uses `shared_ptr` for tree nodes to prevent memory leaks
- Compression ratio depends on character distribution in the file
- Files with many repeated characters compress better
- Very small files may actually increase in size due to tree header overhead

## 👤 Author

College DSA Project - File Compression using Huffman Coding
