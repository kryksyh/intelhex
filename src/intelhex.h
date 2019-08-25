#include <filesystem>
#include <vector>

enum class Result
{
    UNKNOWN,
    SUCCESS,
    FILE_NOT_FOUND,
    INCORRECT_FILE,
    UNSUPPORTED_FORMAT,
};

class Block;

class IntelHex {
public:
    IntelHex();
    IntelHex(std::filesystem::path path);
    ~IntelHex();
    Result load(std::filesystem::path path);
    Result loads(const std::string &hex);
    Result save();
    Result save(const std::filesystem::__cxx11::path &path) const;
    uint8_t get(uint32_t address) const;
    uint8_t &operator[](uint32_t address);
    void erase(uint32_t address, uint32_t length);
    uint32_t maxAddress() const;
    uint32_t minAddress() const;
    Result state() const;

private:
    std::vector<Block *> m_blocks;
    mutable Result m_state;
    std::filesystem::path filename;
    Result parse(std::istream &input);
};
