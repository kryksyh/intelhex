
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
