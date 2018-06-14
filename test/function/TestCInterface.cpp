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
#include <string.h>
#include <stdlib.h>

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
        char buffer[11];
        gwFile file = NULL;
        int len;

        ASSERT_NO_THROW(file = gwOpenFile(fs, fileName, GW_CREAT|GW_RDWR));
        EXPECT_TRUE(gwFileExists(fs, fileName));

        for (int i=0; i<5; i++) {
            ASSERT_NO_THROW(len = gwRead(fs, file, buffer, 10));
            EXPECT_EQ(10, len);
        }
        ASSERT_NO_THROW(gwFlush(fs, file));
        ASSERT_NO_THROW(gwCloseFile(fs, file));
    }




    /**
     * sequence write repetition of alphabet.
     * @param numOfFiles
     * @param fileSize
     * @param prefix
     */
    void TestSequenceWrite(gopherwoodFS fs, int numOfFiles, tSize fileSize, char* prefix) {
        int fileCounter = 0;
        tSize writeCounter = 0;
        int length = -1;
        char fileName[sizeof(prefix) + sizeof(int)];

        for(fileCounter=0; fileCounter<numOfFiles; fileCounter++){      //For loop creating files.
            strcpy(fileName, prefix);
            char fileNum[sizeof(int)];                          //Create file name.
            sprintf(fileNum, "%d", fileCounter + 1);
            strcat(fileName, fileNum);

            gwFile nFile = NULL;
            ASSERT_NO_THROW(nFile = gwOpenFile(fs, fileName, GW_CREAT|GW_RDWR|GW_SEQACC));    //Create file.
            char input[1];
            writeCounter = 0;

            while(writeCounter<fileSize){                       //For loop writing content.
                sprintf(input, "%d", (writeCounter%10));
                ASSERT_NO_THROW(length = gwWrite(fs, nFile, input, sizeof(input)));
                EXPECT_EQ(sizeof(input), length);
                writeCounter += length;
            }
            ASSERT_NO_THROW(gwCloseFile(fs, nFile));
            printf("%s: %d\n", fileName, writeCounter);//!!!
        }
    }

    /**
     * random write int numOfFiles, tSize fileSize
     * @param numOfFiles
     * @param fileSize
     * @param prefix
     */
    void TestRandomWrite(gopherwoodFS fs, int numOfFiles, tSize fileSize, char* prefix) {
        int fileCounter = 0;
        tSize writeCounter = 0;
        int length = -1;
        char fileName[sizeof(prefix) + sizeof(int)];
        for(fileCounter=0; fileCounter<numOfFiles; fileCounter++){      //For loop creating files.
            strcpy(fileName, prefix);
            char fileNum[sizeof(int)];                          //Create file name.
            sprintf(fileNum, "%d", fileCounter + 1);
            strcat(fileName, fileNum);

            gwFile nFile = NULL;
            ASSERT_NO_THROW(nFile = gwOpenFile(fs, fileName, GW_CREAT|GW_RDWR|GW_RNDACC));    //Create file.
            char input[1];
            writeCounter = 0;

            while(writeCounter<fileSize){                   //While loop writing content.
                if(writeCounter % 55 == 0) {                                //When write counter is divisible by 50,
                    ASSERT_NO_THROW(gwSeek(fs, nFile, 10, SEEK_CUR));      //seek forward 26.
                    writeCounter += 10;
                }
                sprintf(input, "%d", (writeCounter%10));
                ASSERT_NO_THROW(length = gwWrite(fs, nFile, input, sizeof(input)));
                EXPECT_EQ(sizeof(input), length);
                writeCounter += length;
            }
            ASSERT_NO_THROW(gwCloseFile(fs, nFile));
            printf("%s: %d\n", fileName, writeCounter);
        }
    }

    /**
     *
     * @param fs
     * @param numOfFiles
     * @param fileSize
     */
    void TestSequenceRead(gopherwoodFS fs, int numOfFiles, tSize fileSize, char* prefix) {
        int fileCounter = 0;
        int length = -1;
        char* buffer = (char*)malloc(sizeof(char));
        tSize totalLength = 0;
        char fileName[sizeof(prefix) + sizeof(int)];

        for(fileCounter=0; fileCounter<numOfFiles; fileCounter ++) {    //For loop iterate all files.
            totalLength = 0;
            length = -1;
            strcpy(fileName, prefix);               //Using current index of file as fileName.
            char fileNum[sizeof(int)];
            sprintf(fileNum, "%d", fileCounter + 1);
            strcat(fileName, fileNum);

            gwFile nFile = NULL;
            ASSERT_NO_THROW(nFile = gwOpenFile(fs, fileName, GW_RDWR|GW_SEQACC));

            while(length != 0) {                                                        //Reach Eof will return 0.
                ASSERT_NO_THROW(length = gwRead(fs, nFile, buffer, sizeof(char)));
                totalLength += length;
            }
            printf("%s: %d\n", fileName, totalLength);
            EXPECT_EQ(totalLength, fileSize);
            ASSERT_NO_THROW(gwCloseFile(fs, nFile));
        }
    }

    /**
     *
     * @param fs
     * @param numOfFiles
     * @param fileSize
     */
    void TestRandomRead(gopherwoodFS fs, int numOfFiles, tSize fileSize, char* prefix) {
        int fileCounter = 0;
        int length = -1;
        char* buffer;
        tSize  totalLength = 0;
        tSize numSeek = 0;
        char fileName[sizeof(prefix) + sizeof(int)];

        for(fileCounter=0; fileCounter<numOfFiles; fileCounter ++) {    //For loop iterate all files.
            totalLength = 0;
            tOffset pos = fileSize/4;
            strcpy(fileName, prefix);               //Using current index of file as fileName.
            char fileNum[sizeof(int)];
            sprintf(fileNum, "%d", fileCounter + 1);
            strcat(fileName, fileNum);

            gwFile nFile = NULL;
            ASSERT_NO_THROW(nFile = gwOpenFile(fs, fileName, GW_RDWR|GW_RNDACC));
            while(length != 0) {                                            //Reach Eof will return 0.
                if(totalLength >= fileSize/2) {
                    if(numSeek == 0) {                      //When first time reach half of the file
                        gwSeek(fs, nFile, pos, SEEK_SET);   //seek back to 1/4 of the file.
                        numSeek ++;
                    }
                    if(numSeek == 1) {                      //When second time reach half of the file
                        gwSeek(fs, nFile, pos, SEEK_CUR);   //seek forward 1/4 of the file.
                    }
                }
                ASSERT_NO_THROW(length = gwRead(fs, nFile, buffer, 4096));
                totalLength += length;
            }
            printf("%d: %d", fileCounter, totalLength);
            ASSERT_NO_THROW(gwCloseFile(fs, nFile));
        }
    }

protected:
    char workDir[40];
    gopherwoodFS fs;

};


TEST_F(TestCInterface, Test_Format_Context) {
    ASSERT_NO_THROW(gwFormatContext(workDir));
}

TEST_F(TestCInterface, Test_FAIL_Exist_NULL_FileName) {
    EXPECT_FALSE(gwFileExists(fs, NULL));
}

TEST_F(TestCInterface, Test_FAIL_Exist_NULL_FileSystem) {
    char fileName[] = "TestCInterface/Test_FAIL_Exist_NULL_FileSystem";
    EXPECT_FALSE(gwFileExists(NULL, fileName));
}

//Tests for OpenFile
TEST_F(TestCInterface, Test_SUCCESS_Open) {
    char fileName[] = "TestCInterface/Test_SUCCESS_Open";
    char input[] = "0123456789";

    gwFile file = NULL;
    int len;

    ASSERT_NO_THROW(file = gwOpenFile(fs, fileName, GW_CREAT|GW_RDWR));
    for (int i=0; i<5; i++) {
        ASSERT_NO_THROW(len = gwWrite(fs, file, input, 10));
        EXPECT_EQ(10, len);
    }
    EXPECT_TRUE(gwFileExists(fs, fileName));

    ASSERT_NO_THROW(gwCloseFile(fs, file));
    ASSERT_NO_THROW(gwDeleteFile(fs, fileName));
}



TEST_F(TestCInterface, Test_FAIL_Open_NULL_FileName) {
    EXPECT_FALSE(gwOpenFile(fs, NULL, GW_CREAT|GW_RDWR));
}

TEST_F(TestCInterface, Test_FAIL_Open_NULL_FileSystem) {
    char fileName[] = "TestCInterface/Test_FAIL_Open_NULL_FileSystem";
    EXPECT_EQ(NULL, gwOpenFile(NULL, fileName, GW_CREAT|GW_RDWR));
}

//Tests for Read
TEST_F(TestCInterface, Test_SUCCESS_Read) {
    char fileName[] = "TestCInterface/Test_SUCCESS_Read";
    this->testWrite(fileName);      //call built-in read and write.
    this->testRead(fileName);
    EXPECT_TRUE(gwFileExists(fs, fileName));
    ASSERT_NO_THROW(gwDeleteFile(fs, fileName));
}

TEST_F(TestCInterface, Test_FAIL_Read_NULL_FileSystem) {
    gwFile file = NULL;
    char fileName[] = "TestCInterface/Test_FAIL_Read_NULL_FileSystem";
    char buffer[11];
    ASSERT_NO_THROW(file = gwOpenFile(fs, fileName, GW_CREAT | GW_RDWR));
    EXPECT_EQ(-1, gwRead(NULL, file, buffer, 10));
}

TEST_F(TestCInterface, Test_FAIL_Read_NULL_File) {
    char buffer[11];
    EXPECT_EQ(-1, gwRead(fs, NULL, buffer, 10));
}

TEST_F(TestCInterface, Test_FAIL_Read_NULL_Buffer) {
    gwFile file = NULL;
    char fileName[] = "TestCInterface/Test_FAIL_Read_NULL_Buffer";
    ASSERT_NO_THROW(file = gwOpenFile(fs, fileName, GW_CREAT|GW_RDWR));
    EXPECT_TRUE(gwFileExists(fs, fileName));
    EXPECT_EQ(-1, gwRead(fs, file, NULL, 10));


    ASSERT_NO_THROW(gwCloseFile(fs, file));
    ASSERT_NO_THROW(gwDeleteFile(fs, fileName));
}


TEST_F(TestCInterface, Test_FAIL_Read_Negative_Length) {
    gwFile file = NULL;
    char fileName[] = "TestCInterface/Test_FAIL_Read_Negative_Length";
    char buffer[11];
    ASSERT_NO_THROW(file = gwOpenFile(fs, fileName, GW_CREAT|GW_RDWR));
    EXPECT_TRUE(gwFileExists(fs, fileName));
    EXPECT_EQ(-1, gwRead(fs, file, buffer, -10));

    ASSERT_NO_THROW(gwCloseFile(fs, file));
    ASSERT_NO_THROW(gwDeleteFile(fs, fileName));
}

//Tests for Seek
TEST_F(TestCInterface, Test_FAIL_Seek_NULL_FileSystem) {
    gwFile file = NULL;
    tOffset offset = 1;
    int mode = 5;
    EXPECT_EQ(-1, gwSeek(NULL, file, offset, mode));
}

TEST_F(TestCInterface, Test_FAIL_Seek_NULL_File) {
    tOffset offset = 1;
    int mode = 5;
    EXPECT_EQ(-1, gwSeek(fs, NULL, offset, mode));
}

//Tests for Write
TEST_F(TestCInterface, Test_SUCCESS_Write) {
    char fileName[] = "TestCInterface/Test_SUCCESS_Write";
    this->testWrite(fileName);
    EXPECT_TRUE(gwFileExists(fs, fileName));
    ASSERT_NO_THROW(gwDeleteFile(fs, fileName));
}

TEST_F(TestCInterface, Test_FAIL_Write_NULL_FileSystem) {
    gwFile file = NULL;
    char fileName[] = "TestCInterface/Test_FAIL_Write_NULL_FileSystem";
    char input[] = "0123456789";
    ASSERT_NO_THROW(file = gwOpenFile(fs, fileName, GW_CREAT|GW_RDWR));
    EXPECT_EQ(-1, gwWrite(NULL, file, input, 10));

    ASSERT_NO_THROW(gwCloseFile(fs, file));
    ASSERT_NO_THROW(gwDeleteFile(fs, fileName));
}

TEST_F(TestCInterface, Test_FAIL_Write_NULL_File) {
    char input[] = "0123456789";
    EXPECT_EQ(-1, gwWrite(fs, NULL, input, 10));
}

TEST_F(TestCInterface, Test_FAIL_Write_Negative_Length) {
    gwFile file = NULL;
    char fileName[] = "TestCInterface/Test_FAIL_Write_Negative_Length";
    char input[] = "0123456789";
    ASSERT_NO_THROW(file = gwOpenFile(fs, fileName, GW_CREAT|GW_RDWR));
    EXPECT_EQ(-1, gwWrite(fs, file, input, -10));

    ASSERT_NO_THROW(gwCloseFile(fs, file));
    ASSERT_NO_THROW(gwDeleteFile(fs, fileName));
}

//Tests for Flush
TEST_F(TestCInterface, Test_FAIL_Flush_NULL_FileSystem) {
    gwFile file = NULL;
    EXPECT_EQ(-1, gwFlush(NULL, file));
}

TEST_F(TestCInterface, Test_FAIL_Flush_NULL_File) {
    EXPECT_EQ(-1, gwFlush(fs, NULL));
}

//Tests for CloseFile
TEST_F(TestCInterface, Test_SUCCESS_CloseFile) {
    char fileName[] = "TestCInterface/Test_SUCCESS_CloseFile";
    char input[] = "0123456789";
    gwFile file = NULL;
    int len;

    ASSERT_NO_THROW(file = gwOpenFile(fs, fileName, GW_CREAT|GW_RDWR));
    EXPECT_TRUE(gwFileExists(fs, fileName));
    ASSERT_NO_THROW(len = gwWrite(fs, file, input, 10));
    EXPECT_EQ(10, len);
    ASSERT_NO_THROW(gwCloseFile(fs, file));

    ASSERT_NO_THROW(gwDeleteFile(fs, fileName));
}

TEST_F(TestCInterface, Test_FAIL_CloseFile_NULL_FileSystem) {
    gwFile file = NULL;
    EXPECT_EQ(-1, gwCloseFile(NULL, file));
}

TEST_F(TestCInterface, Test_FAIL_CloseFile_NULL_File) {
    EXPECT_EQ(-1, gwCloseFile(fs, NULL));
}

//Tests for DeleteFile
TEST_F(TestCInterface, Test_SUCCESS_DeleteFile) {
    char fileName[] = "TestCInterface/Test_SUCCESS_DeleteFile";
    char input[] = "0123456789";
    gwFile file = NULL;
    int len;
    ASSERT_NO_THROW(file = gwOpenFile(fs, fileName, GW_CREAT|GW_RDWR));
    EXPECT_TRUE(gwFileExists(fs, fileName));
    ASSERT_NO_THROW(len = gwWrite(fs, file, input, 10));
    EXPECT_EQ(10, len);

    gwCloseFile(fs, file);
    gwDeleteFile(fs, fileName);

    EXPECT_FALSE(gwFileExists(fs, fileName));
}

TEST_F(TestCInterface, Test_FAIL_DeleteFile_UnClosed) {
    char fileName[] = "TestCInterface/Test_FAIL_DeleteFile_UnClosed";
    char input[] = "0123456789";
    char buffer[11];
    gwFile file = NULL;
    int len;
    ASSERT_NO_THROW(file = gwOpenFile(fs, fileName, GW_CREAT|GW_RDWR));
    EXPECT_TRUE(gwFileExists(fs, fileName));
    ASSERT_NO_THROW(len = gwWrite(fs, file, input, 10));
    ASSERT_NO_THROW(len = gwWrite(fs, file, input, 10));
    EXPECT_EQ(10, len);
    len = 0;

    gwCloseFile(fs, file);
    ASSERT_NO_THROW(file = gwOpenFile(fs, fileName, GW_CREAT|GW_RDWR));
    ASSERT_NO_THROW(len = gwRead(fs, file, buffer, 10));
    EXPECT_EQ(10, len);

    ASSERT_NO_THROW(gwDeleteFile(fs, fileName));
    ASSERT_NO_THROW(len = gwRead(fs, file, buffer, 10));
    EXPECT_EQ(10, len);
    EXPECT_TRUE(gwFileExists(fs, fileName));

    ASSERT_NO_THROW(gwCloseFile(fs, file));
    EXPECT_FALSE(gwFileExists(fs, fileName));
}

TEST_F(TestCInterface, Test_FAIL_DeleteFile_NULL_FileSystem) {
    char fileName[] = "TestCInterface/Test_FAIL_DeleteFile_NULL_FileSystem";
    gwFile file = NULL;
    file = gwOpenFile(fs, fileName, GW_CREAT|GW_RDWR);
    EXPECT_EQ(-1, gwDeleteFile(NULL, fileName));

    ASSERT_NO_THROW(gwCloseFile(fs, file));
    ASSERT_NO_THROW(gwDeleteFile(fs, fileName));
}

TEST_F(TestCInterface, Test_FAIL_DeleteFile_NULL_FileName) {
    EXPECT_EQ(-1, gwDeleteFile(fs, NULL));
}

//Tests for CancelFile
TEST_F(TestCInterface, Test_SUCCESS_CancelFile) {
    char fileName[] = "TestCInterface/Test_SUCCESS_CancelFile";
    char input[] = "0123456789";

    gwFile file = NULL;
    int len;

    ASSERT_NO_THROW(file = gwOpenFile(fs, fileName, GW_CREAT|GW_RDWR));
    for (int i=0; i<5; i++) {
        ASSERT_NO_THROW(len = gwWrite(fs, file, input, 10));
        EXPECT_EQ(10, len);
    }

    ASSERT_NO_THROW(gwCancelFile(fs, file));
    EXPECT_FALSE(gwFileExists(fs, fileName));
}

TEST_F(TestCInterface, Test_FAIL_CancelFile_NULL_FileSystem) {
    gwFile file = NULL;
    EXPECT_EQ(-1, gwCancelFile(NULL, file));
}

TEST_F(TestCInterface, Test_FAIL_CancelFile_NULL_File) {
    EXPECT_EQ(-1, gwCancelFile(fs, NULL));
}

//Tests for StatFile
TEST_F(TestCInterface, Test_SUCCESS_StateFile) {
    GWFileInfo fileInfo;
    fileInfo.fileSize = 10;
    fileInfo.maxQuota = 10;
    fileInfo.curQuota = 10;
    fileInfo.numBlocks = 10;
    fileInfo.numEvicted = 10;
    fileInfo.numLoaded = 10;
    fileInfo.numActivated = 10;

    gwFile file = NULL;
    char fileName[] = "TestCInterface/Test_SUCCESS_StateFile";
    ASSERT_NO_THROW(file = gwOpenFile(fs, fileName, GW_CREAT|GW_RDWR));
    EXPECT_TRUE(gwFileExists(fs, fileName));

    ASSERT_NO_THROW(gwStatFile(fs, file, &fileInfo));

    ASSERT_NO_THROW(gwCloseFile(fs, file));
    ASSERT_NO_THROW(gwDeleteFile(fs, fileName));
}

TEST_F(TestCInterface, Test_FAIL_StateFile_NULL_FileSystem) {
    GWFileInfo fileInfo;
    fileInfo.fileSize = 10;
    fileInfo.maxQuota = 10;
    fileInfo.curQuota = 10;
    fileInfo.numBlocks = 10;
    fileInfo.numEvicted = 10;
    fileInfo.numLoaded = 10;
    fileInfo.numActivated = 10;
    gwFile file = NULL;
    char fileName[] = "TestCInterface/Test_FAIL_StateFile_NULL_FileSystem";
    ASSERT_NO_THROW(file = gwOpenFile(fs, fileName, GW_CREAT|GW_RDWR));
    EXPECT_TRUE(gwFileExists(fs, fileName));

    EXPECT_EQ(-1, gwStatFile(NULL, file, &fileInfo));

    ASSERT_NO_THROW(gwCloseFile(fs, file));
    ASSERT_NO_THROW(gwDeleteFile(fs, fileName));
}

TEST_F(TestCInterface, Test_FAIL_StateFile_NULL_File) {
    GWFileInfo fileInfo;
    fileInfo.fileSize = 10;
    fileInfo.maxQuota = 10;
    fileInfo.curQuota = 10;
    fileInfo.numBlocks = 10;
    fileInfo.numEvicted = 10;
    fileInfo.numLoaded = 10;
    fileInfo.numActivated = 10;

    EXPECT_EQ(-1, gwStatFile(fs, NULL, &fileInfo));
}

TEST_F(TestCInterface, Test_FAIL_StateFile_NULL_FileInfo) {
    gwFile file = NULL;
    char fileName[] = "TestCInterface/Test_FAIL_StateFile_NULL_FileInfo";
    ASSERT_NO_THROW(file = gwOpenFile(fs, fileName, GW_CREAT|GW_RDWR));
    EXPECT_TRUE(gwFileExists(fs, fileName));

    EXPECT_EQ(-1, gwStatFile(fs, file, NULL));

    ASSERT_NO_THROW(gwCloseFile(fs, file));
    ASSERT_NO_THROW(gwDeleteFile(fs, fileName));
}

TEST_F(TestCInterface, Test_FAIL_StateFile_Invalid_FileInfo) {
    GWFileInfo fileInfo;
    fileInfo.fileSize = -1;
    fileInfo.maxQuota = 10;
    fileInfo.curQuota = 10;
    fileInfo.numBlocks = 10;
    fileInfo.numEvicted = 10;
    fileInfo.numLoaded = 10;
    fileInfo.numActivated = 10;
    gwFile file = NULL;

    char fileName[] = "TestCInterface/Test_FAIL_StateFile_Invalid_FileInfo";
    ASSERT_NO_THROW(file = gwOpenFile(fs, fileName, GW_CREAT|GW_RDWR));
    EXPECT_TRUE(gwFileExists(fs, fileName));

    EXPECT_EQ(-1, gwStatFile(fs, file, &fileInfo));

    ASSERT_NO_THROW(gwCloseFile(fs, file));
    ASSERT_NO_THROW(gwDeleteFile(fs, fileName));
}

//Tests for GetSysStat
TEST_F(TestCInterface, Test_SUCCESS_GetSysStat) {
    GWSysInfo sysInfo;
    sysInfo.numFreeBuckets = 10;
    sysInfo.numActiveBuckets = 100;
    sysInfo.numUsedBuckets = 10;
    sysInfo.numLoadingBuckets = 10;
    sysInfo.numEvictingBuckets = 10;
    sysInfo.numAdminActiveStatus = 1;
    sysInfo.numFileActiveStatus = 1;

    ASSERT_NO_THROW(gwGetSysStat(fs, &sysInfo));
}

TEST_F(TestCInterface, Test_FAIL_GetSysStat_NULL_FileSystem) {
    GWSysInfo sysInfo;
    sysInfo.numFreeBuckets = 10;
    sysInfo.numActiveBuckets = 100;
    sysInfo.numUsedBuckets = 10;
    sysInfo.numLoadingBuckets = 10;
    sysInfo.numEvictingBuckets = 10;
    sysInfo.numAdminActiveStatus = 1;
    sysInfo.numFileActiveStatus = 1;

    EXPECT_EQ(-1, gwGetSysStat(NULL, &sysInfo));
}

TEST_F(TestCInterface, Test_FAIL_GetSysStat_NULL_SysInfo){
    EXPECT_EQ(-1, gwGetSysStat(fs, NULL));
}

TEST_F(TestCInterface, Test_FAIL_EvictBlocks_Success) {
    int num = Configuration::NUMBER_OF_BLOCKS + 1;
    ASSERT_NO_THROW(gwEvictBlocks(fs, num));
}

TEST_F(TestCInterface, Test_FAIL_EvictBlocks_NULL_FileSystem) {
    int num = Configuration::NUMBER_OF_BLOCKS + 1;
    EXPECT_EQ(-1, gwEvictBlocks(NULL, num));
}

TEST_F(TestCInterface, Test_FAIL_EvictBlocks_Invalid_Num) {
    EXPECT_EQ(-1, gwEvictBlocks(fs, -99999));
}

TEST_F(TestCInterface, Test_FAIL_InvalidFlags) {
    char fileName[] = "TestCInterface/Test_FAIL_InvalidFlags";

    EXPECT_EQ(NULL, gwOpenFile(fs, fileName, GW_WRONLY|GW_RDWR));
    EXPECT_EQ(NULL, gwOpenFile(fs, fileName, GW_RDWR|GW_RDONCE));
    EXPECT_EQ(NULL, gwOpenFile(fs, fileName, GW_WRONLY|GW_RDONCE));
    EXPECT_EQ(NULL, gwOpenFile(fs, fileName, GW_SEQACC|GW_RNDACC));
}

TEST_F(TestCInterface, Test_SequenceWrite_SequenceRead) {
    char prefix[] = {"Sequence"};
    int numFiles = 5;
    tSize fileSize = 200;

    printf("Writing\n");
    TestSequenceWrite(fs, numFiles, fileSize, prefix);
    printf("Reading\n");
    TestSequenceRead(fs, numFiles, fileSize, prefix);
}

//TEST_F(TestCInterface, Test_RandomWrite_SequenceRead) {
//    char prefix[] = {"Random"};
//    int numFiles = 5;
//    tSize fileSize = 100;
//
//    printf("Writing\n");
//    TestRandomWrite(fs, numFiles, fileSize, prefix);
//    printf("Reading\n");
//    TestSequenceRead(fs, numFiles, fileSize, prefix);
//}

//TEST_F(TestCInterface, Test_SequenceWrite_RandomRead) {
//    char prefix[] = {"Sequence"};
//    int numFiles = 5;
//    tSize fileSize = 20;
//
//    TestSequenceWrite(fs, numFiles, fileSize, prefix);
//    TestRandomRead(fs, numFiles, fileSize, prefix);
//}
//
//TEST_F(TestCInterface, Test_RandomWrite_RandomRead) {
//    char prefix[] = {"Random"};
//    int numFiles = 5;
//    tSize fileSize = 100;
//
//    TestRandomWrite(fs, numFiles, fileSize, prefix);
//    TestRandomRead(fs, numFiles, fileSize, prefix);
//}