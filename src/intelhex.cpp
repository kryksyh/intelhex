#include "intelhex.h"
#include <fstream>
#include <iostream>
#include <string.h>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

enum class RecordType
{
    Data                   = 0,
    EndOfFile              = 1,
    ExtendedSegmentAddress = 2,
    StartSegmentAddress    = 3,
    ExtendedLinearAddress  = 4,
    StartLinearAddress     = 5
};

uint8_t from_hex(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    else if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    else
        return 0;
}

template<typename T>
static T from_hex(const char *str)
{
    if constexpr (std::is_same_v<uint8_t, T>) {
        return (from_hex(str[0]) << 4) + from_hex(str[1]);
    }
    else if constexpr (std::is_same_v<uint16_t, T>) {
        T res(0);
        res = from_hex<uint8_t>(str) << 8;
        str += 2;
        res |= from_hex<uint8_t>(str);
        return res;
    }
    else if constexpr (std::is_same_v<uint32_t, T>) {
        T res(0);
        res = from_hex<uint16_t>(str) << 16;
        str += 2;
        res |= from_hex<uint16_t>(str);
        return res;
    }
    return 0;
}

void to_hex(uint8_t byte, char *out)
{
    char hex[] = "0123456789ABCDEF";
    out[0]     = hex[byte >> 4];
    out[1]     = hex[byte & 0x0F];
}

bool isChecksumCorrect(std::string_view view)
{
    uint8_t cs = 0;
    for (const char *ptr = view.data() + 1; ptr < view.data() + view.size() - 1; ptr += 2) {
        cs += from_hex<uint8_t>(ptr);
    }
    return cs == 0;
}

struct Block {
public:
    Block()
        : m_base_address(0)
        , m_extended_address(0)
        , m_length(0)
        , m_allocated_length(0)
        , m_data(nullptr)
    {
    }
    ~Block() { free(m_data); }
    void add_bytes(uint8_t *data, uint16_t length)
    {
        if (m_length + length > m_allocated_length) {
            m_allocated_length += length * 2;
            m_data = (uint8_t *) realloc(m_data, m_allocated_length);
        }
        memcpy(&m_data[m_length], data, length);
        m_length += length;
    }
    void set_base_address(uint16_t address) { m_base_address = address; }
    void set_extended_address(uint16_t address) { m_extended_address = address; }

    uint32_t address() const { return (m_extended_address << 16) + m_base_address; }

    uint32_t length() const { return m_length; }

    uint8_t *data() const { return m_data; }

private:
    uint16_t m_base_address;
    uint16_t m_extended_address;
    uint32_t m_length;
    uint8_t *m_data;
    unsigned int m_allocated_length;
};

IntelHex::IntelHex()
{
    m_state = Result::INCORRECT_FILE;
    m_blocks.clear();
}

IntelHex::~IntelHex()
{
    for (auto block : m_blocks) {
        delete block;
    }
}

Result IntelHex::parse(std::istream &input)
{
    std::string line;
    uint16_t extended_address(0);
    Block *currentBlock = new Block();

    while (std::getline(input, line) && m_state == Result::UNKNOWN) {
        RecordType type;
        uint8_t length(0);
        uint16_t address(0);
        uint8_t buf[256];

        if (line.size() < 11)
            continue;

        if (line[0] != ':') {
            delete currentBlock;
            m_state = Result::INCORRECT_FILE;
            break;
        }

        std::string_view view(line);
        const char *data = view.data() + 1;

        length = from_hex<uint8_t>(data);
        data += 2;

        address = from_hex<uint16_t>(data);
        data += 4;

        type = static_cast<RecordType>(from_hex<uint8_t>(data));
        data += 2;

        for (int i = 0; i < length; i++) {
            buf[i] = from_hex<uint8_t>(data);
            data += 2;
        }

        if (!isChecksumCorrect(view)) {
            delete currentBlock;
            m_state = Result::INCORRECT_FILE;
            break;
        }

        switch (type) {
        case RecordType::Data:
            if (currentBlock->address() + currentBlock->length() !=
                    (extended_address << 16) + address ||
                (currentBlock->address() >> 16) != extended_address) {
                m_blocks.push_back(currentBlock);
                currentBlock = new Block();
                currentBlock->set_extended_address(extended_address);
                currentBlock->set_base_address(address);
            }
            currentBlock->add_bytes(buf, length);
            break;
        case RecordType::EndOfFile:
            m_blocks.push_back(currentBlock);
            m_state = Result::SUCCESS;
            break;
        case RecordType::StartLinearAddress:
            break;
        case RecordType::ExtendedLinearAddress:
            // extended address 2 bytes, always big endian
            if (length == 2) {
                extended_address = (buf[0] << 8) | buf[1];
            }
            else {
                m_state = Result::INCORRECT_FILE;
            }
            break;
        case RecordType::ExtendedSegmentAddress:
        case RecordType::StartSegmentAddress:
            m_state = Result::UNSUPPORTED_FORMAT;
        default:
            m_state = Result::INCORRECT_FILE;
            break;
        }
    }
    return m_state;
}

Result IntelHex::load(fs::path path)
{
    std::ifstream infile(path);
    m_state = Result::UNKNOWN;

    m_blocks.clear();
    if (!infile.is_open()) {
        m_state = Result::FILE_NOT_FOUND;
    }
    else {
        m_state = parse(infile);
    }

    return m_state;
}

Result IntelHex::loads(const std::string &hex)
{
    m_state = Result::UNKNOWN;
    std::istringstream input(hex);
    return parse(input);
}

Result IntelHex::save()
{
    m_state = save(filename);
    return m_state;
}

uint8_t checksum(uint32_t size, std::array<uint8_t, 256> buf)
{
    uint8_t cs = 0;
    for (int i = 0; i < size; i++) {
        cs += buf[i];
    }
    cs = (~cs) + 1;
    return cs;
}

Result IntelHex::save(const fs::path &path) const
{
    std::ofstream outfile(path);
    std::string line;
    m_state = Result::UNKNOWN;

    if (!outfile.is_open())
        m_state = Result::FILE_NOT_FOUND;
    uint16_t extended_address = 0;
    std::array<uint8_t, 256> buf;
    std::array<char, buf.size()> str;
    str[0]              = ':';
    uint8_t line_length = 0;
    for (auto block : m_blocks) {
        uint16_t new_extended_address = block->address() >> 16;
        uint16_t base_address         = block->address();

        if (extended_address != new_extended_address) {
            extended_address = new_extended_address;
            //:02'00'00'04'00'01'F9
            buf[0]      = 2;
            buf[1]      = 0;
            buf[2]      = 0;
            buf[3]      = static_cast<uint8_t>(RecordType::ExtendedLinearAddress);
            buf[4]      = extended_address >> 8;
            buf[5]      = extended_address & 0xFF;
            line_length = 6;
            buf[6]      = checksum(line_length, buf);
            line_length = 7;

            for (int i = 0; i < line_length; i++) {
                to_hex(buf[i], &(str.data()[1 + i * 2]));
            }
            str[line_length * 2 + 1] = '\n';

            outfile.write(str.data(), line_length * 2 + 2);
        }
        //:20'FFE0'00'02680A6051607047426808604A60116041607047014880687047C0464C360020B0
        uint32_t write_pos = 0;
        while (block->length() > write_pos) {
            // Default line size
            uint8_t write_size = 0x20;

            // If current block is running out, writing what's left
            if (block->length() - write_pos < write_size)
                write_size = (block->length() - write_pos);

            // Record address, 16 bytes,
            uint16_t write_addr = (block->address() & 0xFFFF) + write_pos;

            // Line header: REC_SIZE REC_ADDR REC_TYPE
            buf[0] = write_size;
            buf[1] = write_addr >> 8;
            buf[2] = write_addr & 0xFF;
            buf[3] = static_cast<uint8_t>(RecordType::Data);

            // Advancing forward past header
            line_length = 4;

            // Copying just enough data to new buffer
            memcpy(&buf[line_length], &(block->data()[write_pos]), write_size);
            line_length += write_size;

            // Checksum it all over
            buf[line_length] = checksum(line_length, buf);
            line_length++;

            // Converting to ascii
            for (int i = 0; i < line_length; i++) {
                to_hex(buf[i], &(str.data()[1 + i * 2]));
            }

            str[line_length * 2 + 1] = '\n';
            outfile.write(str.data(), line_length * 2 + 2);

            // Advancing buffer position
            write_pos += write_size;
        }
        m_state = Result::SUCCESS;
    }
    outfile.close();
    return m_state;
}

uint8_t IntelHex::get(uint32_t address) const
{
    for (auto block : m_blocks) {
        if (block->address() <= address && block->address() + block->length() > address) {
            return block->data()[address - block->address()];
        }
    }
    return 0;
}

uint8_t &IntelHex::operator[](uint32_t address)
{
    for (auto block : m_blocks) {
        if (block->address() <= address && block->address() + block->length() > address) {
            return block->data()[address - block->address()];
        }
        else if (block->address() + block->length() == address) {
            uint8_t tmp = 0x00;
            block->add_bytes(&tmp, 1);
            return block->data()[address - block->address()];
        }
    }
    uint8_t tmp     = 0x00;
    Block *newBlock = new Block();
    newBlock->set_extended_address(address >> 16);
    newBlock->set_base_address(address & 0xFFFF);
    m_blocks.push_back(newBlock);
    newBlock->add_bytes(&tmp, 1);
    return newBlock->data()[address - newBlock->address()];
}

uint32_t IntelHex::maxAddress() const
{
    uint32_t max = 0;
    for (auto block : m_blocks) {
        uint32_t lmax = block->address() + block->length() - 1;
        if (max < lmax)
            max = lmax;
    }
    return max;
}

uint32_t IntelHex::minAddress() const
{
    uint32_t min = 0xFFFFFFFF;
    for (auto block : m_blocks) {
        uint32_t lmin = block->address() + block->length() - 1;
        if (min < lmin)
            min = lmin;
    }
    return min;
}
