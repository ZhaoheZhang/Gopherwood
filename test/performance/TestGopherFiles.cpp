/********************************************************************
 * 2016 -
 * open source under Apache License Version 2.0
 ********************************************************************/
/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "common/Configuration.h"
#include "client/gopherwood.h"
#include "common/DateTime.h"
#include "common/Exception.h"
#include "common/ExceptionInternal.h"
#include "gtest/gtest.h"

using namespace Gopherwood;
using namespace Gopherwood::Internal;
using Gopherwood::Internal::Configuration;

class TestCInterface: public ::testing::Test {
public:
    TestCInterface()
    {
        try {
            sprintf(workDir, "/data/gopherwood");

            GWContextConfig config;
            config.blockSize = 10;
            config.numBlocks = 50;
            config.numPreDefinedConcurrency = 10;
            config.severity = LOGSEV_INFO;
            fs =  gwCreateContext(workDir, &config);
        } catch (...) {

        }
    }

    ~TestCInterface() {
        try {
            gwDestroyContext(fs);
        } catch (...) {
        }
    }

    /**
     * write function for testing.
     * @param fileName
     * Use fileName to open file, close the file after writing.
     */

    void testWrite(char* fileName) {
        char input[] = "0123456789";
        gwFile file = NULL;
        int len;

        ASSERT_NO_THROW(file = gwOpenFile(fs, fileName, GW_CREAT|GW_RDWR));
        EXPECT_TRUE(gwFileExists(fs, fileName));

        for (int i=0; i<5; i++) {
            ASSERT_NO_THROW(len = gwWrite(fs, file, input, 10));
            EXPECT_EQ(10, len);
        }
        ASSERT_NO_THROW(gwFlush(fs, file));
        ASSERT_NO_THROW(gwCloseFile(fs, file));
    }

    /**
     * read function for testing.
     * @param fileName
     * Use fileName to open file, close the file after reading.
     */
    void testRead(char* fileName) {
        char output[11];
        gwFile file = NULL;
        int len;

        ASSERT_NO_THROW(file = gwOpenFile(fs, fileName, GW_CREAT|GW_RDWR));
        EXPECT_TRUE(gwFileExists(fs, fileName));

        for (int i=0; i<5; i++) {
            ASSERT_NO_THROW(len = gwRead(fs, file, output, 10));
            EXPECT_EQ(10, len);
        }
        ASSERT_NO_THROW(gwFlush(fs, file));
        ASSERT_NO_THROW(gwCloseFile(fs, file));
    }

protected:
    char workDir[40];
    gopherwoodFS fs;

};