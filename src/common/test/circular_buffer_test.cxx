#include "circular_buffer.h"
#include "logger.h"
#include "util.h"
#include "gtest/gtest.h"

#include <sys/types.h>
#include <unistd.h>
#include <string>

using namespace logger;
using namespace std;

class CircularBufferTest : public testing::Test {

    protected:
        virtual void SetUp() {
            Logger::SetLogFile("/tmp/gtest.log");
            Logger::SetLogLevel("DEBUG");

            LOG(INFO) << "************************************************";
            LOG(INFO) << "            CircularBufferTest Start Up";
            LOG(INFO) << "************************************************";
        }

        virtual void TearDown() {
            LOG(INFO) << "CircularBufferTest TearDown";
        }

        void const printRawBytes(stringstream& out, const char* buffer, const size_t numBytes) {
            for (size_t ii = 0; ii < numBytes;ii++)
                out << setfill('0') << setw(2) << hex << byteToUnsignedInt(buffer[ii]);
        }

};

/* Test  CircularBuffer constructor */
TEST_F(CircularBufferTest, CTR) {
    CircularBuffer circularBuffer(10);
    EXPECT_EQ(circularBuffer.capacity(), 10);
    EXPECT_EQ(circularBuffer.available(), 10);
    EXPECT_EQ(circularBuffer.size(), 0);
}

TEST_F(CircularBufferTest, Write) {
    CircularBuffer circularBuffer(10);
    char writeBuffer[10];
    memset(writeBuffer, 7, 10);

    size_t bytes_written = circularBuffer.write(writeBuffer, 10);
    EXPECT_EQ(bytes_written, 10);
    EXPECT_EQ(circularBuffer.available(), 0);
    EXPECT_EQ(circularBuffer.size(), 10);

    bytes_written = circularBuffer.write(writeBuffer, 1);
    EXPECT_EQ(bytes_written, 0);
    EXPECT_EQ(circularBuffer.available(), 0);
    EXPECT_EQ(circularBuffer.size(), 10);
}

TEST_F(CircularBufferTest, Read) {
    CircularBuffer circularBuffer(10);
    char writeBuffer[10];
    memset(writeBuffer, 7, 10);

    char readBuffer[10];
    memset(readBuffer, 0, 10);

    size_t bytes_written = circularBuffer.write(writeBuffer, 10);
    EXPECT_EQ(bytes_written, 10);
    EXPECT_EQ(circularBuffer.available(), 0);
    EXPECT_EQ(circularBuffer.size(), 10);

    size_t bytes_read = circularBuffer.read(readBuffer, 10);
    EXPECT_EQ(bytes_read, 10);
    EXPECT_EQ(circularBuffer.available(), 10);
    EXPECT_EQ(circularBuffer.size(), 0);

    bytes_read = circularBuffer.read(readBuffer, 10);
    EXPECT_EQ(bytes_read, 0);
    EXPECT_EQ(circularBuffer.available(), 10);
    EXPECT_EQ(circularBuffer.size(), 0);
    EXPECT_FALSE(memcmp(readBuffer, writeBuffer, 10));

    stringstream out;
    printRawBytes(out, writeBuffer, 10);
    LOG(INFO) << "writeBuffer = " << out.str();
    out.str("");
    printRawBytes(out, readBuffer, 10);
    LOG(INFO) << "readBuffer = " << out.str();

    memset(writeBuffer, 77, 10);
    memset(readBuffer, 0, 10);

    bytes_written = circularBuffer.write(writeBuffer, 10);
    EXPECT_EQ(bytes_written, 10);
    EXPECT_EQ(circularBuffer.available(), 0);
    EXPECT_EQ(circularBuffer.size(), 10);

    bytes_read = circularBuffer.read(readBuffer, 10);
    EXPECT_EQ(bytes_read, 10);
    EXPECT_EQ(circularBuffer.available(), 10);
    EXPECT_EQ(circularBuffer.size(), 0);

    bytes_read = circularBuffer.read(readBuffer, 10);
    EXPECT_EQ(bytes_read, 0);
    EXPECT_EQ(circularBuffer.available(), 10);
    EXPECT_EQ(circularBuffer.size(), 0);
    EXPECT_FALSE(memcmp(readBuffer, writeBuffer, 10));

    out.str("");
    printRawBytes(out, writeBuffer, 10);
    LOG(INFO) << "writeBuffer = " << out.str();
    out.str("");
    printRawBytes(out, readBuffer, 10);
    LOG(INFO) << "readBuffer = " << out.str();
}
