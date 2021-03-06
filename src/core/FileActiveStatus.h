/********************************************************************
 * 2017 -
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
#ifndef _GOPHERWOOD_CORE_ACTIVESTATUS_H_
#define _GOPHERWOOD_CORE_ACTIVESTATUS_H_

#include "client/gopherwood.h"
#include "platform.h"

#include "block/OssBlockWorker.h"
#include "common/LRUCache.cpp"
#include "common/ThreadPool.h"
#include "core/SharedMemoryContext.h"
#include "core/BaseActiveStatus.h"
#include "core/BlockStatus.h"
#include "core/Manifest.h"
#include "file/FileId.h"

namespace Gopherwood {
namespace Internal {

enum ActiveStatusType {
    writeFile = 1,
    readFile = 2,
    deleteFile = 3
};

/**
 * ActiveStatus
 *
 * @desc An ActiveStatus instance maintains file block status. Three main
 * function of this class are:
 * 1. Communicate with Shared Memory to acquire/release local buckets
 * 2. Maintain Manifest Log to syncronize file status with Shared Memory status
 * 3. Provide the file/block status to OutputStream/InputStream
 * @BlockArray The array to save all block status
 * @PreAllocatedBlocks To eliminate the Share Memory contention issue, each time
 * it need to acquire new buckets, a number of buckets will be pre-allocated.
 * @SharedMemoryContext The filesystem level Shared Memory instance to control
 * bucket operations.
 */
class FileActiveStatus : BaseActiveStatus{
public:
    FileActiveStatus(FileId fileId,
                 shared_ptr<SharedMemoryContext> sharedMemoryContext,
                 shared_ptr<ThreadPool> threadPool,
                 bool isCreate,
                 bool isSequence,
                 ActiveStatusType type,
                 int localSpaceFD
    );

    /*********** Getter and setters ***********/
    int64_t getEof();
    int64_t getPosition();
    void setPosition(int64_t pos);

    /**** The main entry point to adjust block status ****/
    BlockInfo getCurBlockInfo();
    void getStatistics(GWFileInfo *fileInfo);

    void flush();
    void close(bool isCancel);

    /* used as a Thread function */
    void loadBlock(BlockInfo info);

    ~FileActiveStatus();

private:
    void registInSharedMem();
    void unregistInSharedMem(bool isCancel);

    Block getCurBlock();
    int32_t getNumBlocks();
    int64_t getCurBlockOffset();
    int32_t getCurQuota();
    int32_t getNumAcquiredBuckets();
    std::string getManifestFileName(FileId fileId);
    bool isMyActiveBlock(int blockId);
    bool isBlockLoading(int blockId);

    /***** active status block manipulations *****/
    void catchUpManifestLogs();
    void adjustActiveBlock(int curBlockId);
    void acquireNewBlocks();
    void extendOneBlock();
    int activateBlock(int blockId);
    void updateCurBlockSize();
    void getSharedMemEof();

    void logEvictBlock(BlockInfo info);

    /****************** Fields *******************/
    FileId mFileId;
    shared_ptr<ThreadPool> mThreadPool;
    shared_ptr<Manifest> mManifest;
    shared_ptr<LRUCache<int, int>> mLRUCache;

    bool mIsWrite;
    bool mIsDelete;
    bool mIsSequence;
    bool mShouldDestroy;
    int64_t mPos;
    int64_t mEof;

    std::vector<Block> mBlockArray;
    std::list<Block> mPreAllocatedBuckets;
    std::vector<Block> mLoadingBuckets;
    std::mutex mLoadMutex;
};


}
}

#endif //_GOPHERWOOD_CORE_ACTIVESTATUS_H_
