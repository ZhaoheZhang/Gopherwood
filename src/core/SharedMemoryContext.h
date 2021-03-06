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
#ifndef _GOPHERWOOD_CORE_SHAREDMEMORYCONTEXT_H_
#define _GOPHERWOOD_CORE_SHAREDMEMORYCONTEXT_H_

#include "platform.h"
#include "common/Memory.h"
#include "core/BlockStatus.h"
#include "core/SharedMemoryObj.h"

#include <boost/interprocess/mapped_region.hpp>

namespace Gopherwood {
namespace Internal {

using namespace boost::interprocess;

/**
 * SharedMemoryContext
 *
 * @desc SharedMemoryContext is an instance of mapped region for the unified metadata storage
 * Gopherwood is an embedded POSIX-like filesystem which do not have an overall status
 * coordinator. Each block operation will sync with SharedMemoryContext and archive to Manifest
 * Log file.
 * With this principle in mind, we shaped the Shared Memory region to:
 * 1. ShareMemHeader -- Contains SharedMemory information and statistics
 * 2. ShareMemBucket -- The bucket status
 * 3. ShareMemActiveStatus -- Track all ActiveStatus instances
 */
class SharedMemoryContext {
public:
    SharedMemoryContext(std::string dir, shared_ptr<mapped_region> region, int lockFD, bool reset);

    /* Regist/Unregist an ActiveStatus instance */
    int16_t registFile(int pid, FileId fileId, bool isWrite, bool isDelete);
    int16_t registAdmin(int pid);
    int unregistFile(int16_t activeId, int pid, bool *shouldDestroy);
    int unregistAdmin(int16_t activeId, int pid);

    /* support functions */
    int calcDynamicQuotaNum();
    bool isFileOpening(FileId fileId);

    /* bucket allocate/free/update */
    std::vector<int32_t> acquireFreeBucket(int16_t activeId, int num, FileId fileId, bool isWrite);
    void releaseBuckets(std::list<Block> &blocks);
    int activateBucket(FileId fileId, Block &block, int16_t activeId, bool isWrite);
    std::vector<Block> inactivateBuckets(std::vector<Block> &blocks, FileId fileId, int16_t activeId, bool isWrite);
    void updateActiveFileInfo(std::vector<Block> &blocks, FileId fileId);
    void deleteBlocks(std::vector<Block> &blocks, FileId fileId);
    void updateBucketDataSize(int32_t bucketId, int64_t size, FileId fileId, int16_t activeId);

    /* evict/load logic related APIs*/
    BlockInfo markBucketEvicting(int16_t activeId);
    int evictBucketFinishAndTryAcquire(int32_t bucketId, int16_t activeId, FileId fileId, int isWrite);
    int evictBucketFinishAndTryFree(int32_t bucketId, int16_t activeId);
    bool markBucketLoading(int32_t bucketId, int32_t blockId, int16_t activeId, FileId fileId);
    void markLoadFinish(int32_t bucketId, int16_t activeId, FileId fileId);
    bool isBlockLoading(FileId fileId, int32_t blockId);

    void reset();
    void lock();
    void unlock();

    /* getter & setter */
    int32_t getFreeBucketNum();
    int32_t getActiveBucketNum();
    int32_t getUsedBucketNum();
    int32_t getEvictingBucketNum();
    int32_t getLoadingBucketNum();
    int32_t getFileActiveStatusNum();
    int32_t getAdminActiveStatusNum();

    std::string &getWorkDir();
    int32_t getNumMaxActiveStatus();
    int64_t getBucketDataSize(int32_t bucketId, FileId fileId, int32_t blockId);

    ~SharedMemoryContext();

private:
    void printStatistics();

    std::string workDir;
    shared_ptr<mapped_region> mShareMem;
    int mLockFD;
    ShareMemHeader *header;
    ShareMemBucket *buckets;
    ShareMemActiveStatus *activeStatus;
};

}
}

#endif //_GOPHERWOOD_CORE_SHAREDMEMORYCONTEXT_H_
