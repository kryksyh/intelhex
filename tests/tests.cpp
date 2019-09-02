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

#include "catch.hpp"
#include "intelhex.h"

using namespace IntelHexNS;

TEST_CASE("Can load file", "Loading")
{
    auto hex = IntelHex();
    REQUIRE(hex.load("incorrect path") == Result::FILE_NOT_FOUND);
    REQUIRE(hex.load("/tmp/S2000R-DZ_V1-00_10-10-2018_11-56.hex") == Result::SUCCESS);
}

TEST_CASE("Loads", "Loads")
{
    auto hex   = IntelHex();
    auto input = R"(
:10010000214601360121470136007EFE09D2190140
:100110002146017E17C20001FF5F16002148011928
:10012000194E79234623965778239EDA3F01B2CAA7
:100130003F0156702B5E712B722B732146013421C7
:00000001FF
)";
    REQUIRE(hex.loads(input) == Result::SUCCESS);
}

TEST_CASE("Can save file", "Saving")
{
    auto hex = IntelHex();
    REQUIRE(hex.load("/tmp/S2000R-DZ_V1-00_10-10-2018_11-56.hex") == Result::SUCCESS);
    // REQUIRE(saveFile("////incorrect path") == Result::NO_SUCH_FILE);
    REQUIRE(hex.save("/tmp/S2000R-DZ_V1-00_10-10-2018_11-56_out.hex") == Result::SUCCESS);
}

TEST_CASE("Reading file", "Reading")
{
    auto hex = IntelHex();
    REQUIRE(hex.load("/tmp/S2000R-DZ_V1-00_10-10-2018_11-56.hex") == Result::SUCCESS);
    volatile uint8_t byte;
    for (uint32_t address = 0; address < 0x1FFFF; address++) {
        byte = hex.get(address);
    }
}

TEST_CASE("Modifying file", "Modify")
{
    auto hex = IntelHex();
    REQUIRE(hex.load("/tmp/S2000R-DZ_V1-00_10-10-2018_11-56.hex") == Result::SUCCESS);
    for (uint32_t address = 0x9d004000; address < 0x9d00a000; address++) {
        hex[address] = 0xFF;
    }
    for (uint32_t address = 0x9d004000; address < 0x9d00a000; address++) {
        REQUIRE(hex[address] == 0xFF);
    }
}
