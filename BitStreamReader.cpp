#include "BitStreamReader.h"

// ============================================================
// BitStreamReader Implementation
// ============================================================

// Initialize with no bits in buffer
BitStreamReader::BitStreamReader(std::ifstream& inFile)
    : inputFile(inFile), buffer(0), bitCount(0) {}

// Read a single bit from the file
// How it works:
//   - If buffer is empty, read a new byte from file
//   - Extract the most significant bit (leftmost bit)
//   - Shift buffer left to prepare for next read
//
// We read bits from MSB to LSB (left to right) because that's
// the same order BitStreamWriter wrote them

int BitStreamReader::readBit() {
    // If buffer is empty, read the next byte from file
    if (bitCount == 0) {
        char ch;
        if (!inputFile.get(ch)) {
            return -1;  // end of file reached
        }
        buffer = static_cast<unsigned char>(ch);
        bitCount = 8;
    }

    // Extract the most significant bit
    // Example: buffer = 10110100, MSB is 1
    // (buffer >> 7) shifts right by 7, leaving only the MSB
    int bit = (buffer >> 7) & 1;

    // Shift buffer left to move next bit into MSB position
    buffer = buffer << 1;
    bitCount--;

    return bit;
}

// Read a full byte (8 bits) from the file
int BitStreamReader::readByte() {
    int result = 0;

    for (int i = 0; i < 8; i++) {
        int bit = readBit();
        if (bit == -1) return -1;  // EOF
        result = (result << 1) | bit;
    }

    return result;
}

// Check if there's more data to read
bool BitStreamReader::hasMore() {
    // Check if there are bits remaining in buffer
    if (bitCount > 0) return true;

    // Check if file has more data
    return inputFile.peek() != EOF;
}
