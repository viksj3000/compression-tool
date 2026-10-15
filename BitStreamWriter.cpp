#include "BitStreamWriter.h"

// ============================================================
// BitStreamWriter Implementation
// ============================================================

// Initialize with empty buffer (no bits accumulated yet)
BitStreamWriter::BitStreamWriter(std::ofstream& outFile)
    : outputFile(outFile), buffer(0), bitCount(0) {}

// Write a single bit to the buffer
// How it works:
//   - We shift the buffer left by 1 to make room for the new bit
//   - We OR the new bit into the rightmost position
//   - When we've accumulated 8 bits, write the byte to file and reset
//
// Example: Writing bits 1, 0, 1, 1
//   buffer starts at: 00000000
//   after bit 1:      00000001 (shifted left, OR with 1)
//   after bit 0:      00000010 (shifted left, OR with 0)
//   after bit 1:      00000101 (shifted left, OR with 1)
//   after bit 1:      00001011 (shifted left, OR with 1)
//   ... continues until 8 bits, then writes byte to file

void BitStreamWriter::writeBit(int bit) {
    // Shift buffer left to make room and add the new bit
    buffer = (buffer << 1) | (bit & 1);
    bitCount++;

    // When we have 8 bits (one full byte), write it to the file
    if (bitCount == 8) {
        outputFile.put(static_cast<char>(buffer));
        buffer = 0;
        bitCount = 0;
    }
}

// Write a full byte to the file
void BitStreamWriter::writeByte(unsigned char byte) {
    // If there are pending bits in buffer, write each bit of this byte individually
    if (bitCount > 0) {
        for (int i = 7; i >= 0; i--) {
            writeBit((byte >> i) & 1);
        }
    } else {
        outputFile.put(static_cast<char>(byte));
    }
}

// Write a string of '0's and '1's as actual bits
// This is used to write the encoded text where each character
// has been replaced by its Huffman code string
void BitStreamWriter::writeBitString(const std::string& bitString) {
    for (char c : bitString) {
        writeBit(c - '0');  // convert '0'/'1' character to integer 0/1
    }
}

// Flush remaining bits with zero-padding
// If we have 5 bits in buffer, we add 3 zeros to complete the byte
// Returns the number of padding bits added (needed for decompression)
int BitStreamWriter::flush() {
    int paddingBits = 0;

    if (bitCount > 0) {
        paddingBits = 8 - bitCount;
        // Shift remaining bits to the left and pad with zeros
        buffer = buffer << paddingBits;
        outputFile.put(static_cast<char>(buffer));
        buffer = 0;
        bitCount = 0;
    }

    return paddingBits;
}
