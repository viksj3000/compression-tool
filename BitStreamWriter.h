#ifndef BITSTREAM_WRITER_H
#define BITSTREAM_WRITER_H

#include <fstream>
#include <string>

// ============================================================
// BitStreamWriter - Writes individual bits to a binary file
// ============================================================
// Normal file I/O works at the byte level (8 bits at a time).
// But Huffman codes can be any number of bits (like 3 bits or 5 bits).
// This class collects bits one by one into a buffer byte,
// and when 8 bits are collected, writes that byte to the file.
//
// At the end, if we have leftover bits (less than 8), we pad
// with zeros and write the count of padding bits so the decoder
// knows how many bits to ignore at the end.

class BitStreamWriter {
private:
    std::ofstream& outputFile;    // reference to the output file stream
    unsigned char buffer;         // accumulates bits until we have 8 (one full byte)
    int bitCount;                 // how many bits are currently in the buffer (0-7)

public:
    // Constructor takes a reference to an already-opened output file
    BitStreamWriter(std::ofstream& outFile);

    // Write a single bit (0 or 1) to the buffer
    // When buffer is full (8 bits), it automatically writes the byte to file
    void writeBit(int bit);

    // Write a full byte directly to the file (flushes buffer first if needed)
    void writeByte(unsigned char byte);

    // Write a string of '0' and '1' characters as actual bits
    void writeBitString(const std::string& bitString);

    // Flush remaining bits in buffer to file with zero-padding
    // Returns the number of padding bits added (0-7)
    // This MUST be called after all data is written
    int flush();
};

#endif // BITSTREAM_WRITER_H
