#include "exception.h"
#include "logger.h"
#include "spawn_process.h"
#include "gtest/gtest.h"
#include "util.h"

#include <sys/types.h>
#include <unistd.h>
#include <string>

using namespace logger;
using namespace std;

const char *BASEDIR = "/tmp";

class CommonTest : public testing::Test {
    
    protected:
        virtual void SetUp() {
            Logger::SetLogFile("/tmp/gtest.log");
            Logger::SetLogLevel("DEBUG");
            
            LOG(INFO) << "************************************************";
            LOG(INFO) << "            Common Test Start Up";
            LOG(INFO) << "************************************************";
        }
    
        virtual void TearDown() {
            LOG(INFO) << "CommonTest TearDown";
            stringstream out;
            
            out << "rm -rf " << testDir();
            LOG(DEBUG) << "CMD: " << out.str();
            system(out.str().c_str());
        }
    
        ~CommonTest() {
            LOG(INFO) << "CommonTest dtor";
        }
        
        string testDir(string filename = "") {
            stringstream out;
            out << BASEDIR;
            out << "/dir." << getpid();
            
            if(filename.length())
                out << "/" << filename;
                
            LOG(DEBUG) << "Test base dir: " << out.str();
            
            return out.str();
        }
        
        bool pathExists(string dir) {
            struct stat st;
            return stat(dir.c_str(),&st) == 0;
        }
};

/* Test NOOP */
TEST_F(CommonTest, NOOP) {
}

/* Mkpath no path */
// Test mkpath when just a file name is specified.  No directory created.
TEST_F(CommonTest, mkpathNoPath) {
    EXPECT_TRUE(mkpath("testfile.txt"));
    EXPECT_FALSE(pathExists(testDir()));
}

/* Mkpath path */
// Test make path
TEST_F(CommonTest, MkpathFull) {
    // Test when doesn't exists
    EXPECT_TRUE(mkpath(testDir("testfile.txt")));
    EXPECT_TRUE(pathExists(testDir()));
    
    // Dir should exists already
    EXPECT_TRUE(mkpath(testDir("testfile.txt")));
    EXPECT_TRUE(pathExists(testDir()));
    
    // Test multiple directory creates
    EXPECT_TRUE(mkpath(testDir("a/a/a/a/a/testfile.txt")));
    EXPECT_TRUE(pathExists(testDir("a/a/a/a/a")));
}

