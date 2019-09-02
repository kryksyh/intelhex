/**
 *  MIT License
 *
 *  Copyright (c) Dmitry Makarenko 2019
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#include <vector>
#include "std_compat.h"

namespace IntelHexNS {

enum class Result
{
    UNKNOWN,
    SUCCESS,
    FILE_NOT_FOUND,
    INCORRECT_FILE,
    UNSUPPORTED_FORMAT,
};

struct Block;

class IntelHex {
public:
    IntelHex();
    IntelHex(fs::path path);
    ~IntelHex();
    Result load(fs::path path);
    Result loads(const std::string &hex);
    Result save();
    Result save(const fs::path &path) const;
    uint8_t get(uint32_t address) const;
    uint8_t &operator[](uint32_t address);
    void erase(uint32_t address, uint32_t length);
    uint32_t maxAddress() const;
    uint32_t minAddress() const;
    uint32_t size() const;
    Result state() const;
    void fill(uint8_t fillChar);
    bool isSet(uint32_t address, uint8_t &val);

private:
    std::vector<Block *> m_blocks;
    mutable Result m_state;
    fs::path filename;
    Result parse(std::istream &input);
    uint8_t m_fillChar;
};

} // namespace IntelHexNS
