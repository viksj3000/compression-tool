#ifndef BITSTREAM_READER_H
#define BITSTREAM_READER_H

#include <fstream>

// ============================================================
// BitStreamReader - Reads individual bits from a binary file
// ============================================================
// This is the counterpart of BitStreamWriter. It reads bytes
// from the compressed file and returns them one bit at a time.
// During decompression, we traverse the Huffman tree bit by bit:
//   - bit 0 means go left
//   - bit 1 means go right
// When we reach a leaf node, we've decoded one character.

class BitStreamReader {
private:
    std::ifstream& inputFile;     // reference to the input file stream
    unsigned char buffer;         // current byte being read bit by bit
    int bitCount;                 // how many bits remain in current buffer (0-8)

public:
    // Constructor takes a reference to an already-opened input file
    BitStreamReader(std::ifstream& inFile);

    // Read a single bit from the file (returns 0 or 1)
    // Returns -1 if end of file is reached
    int readBit();

    // Read a full byte (8 bits) from the file
    // Returns -1 if end of file is reached
    int readByte();

    // Check if there are more bits to read
    bool hasMore();
};

#endif // BITSTREAM_READER_H
