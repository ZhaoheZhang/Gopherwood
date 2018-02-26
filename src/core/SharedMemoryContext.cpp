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
#include "SharedMemoryContext.h"

namespace Gopherwood {
namespace Internal {

SharedMemoryContext::SharedMemoryContext(std::string dir, shared_ptr<mapped_region> region,
        shared_ptr<named_semaphore> semaphore) :
        workDir(dir), mShareMem(region), mSemaphore(semaphore) {
    void* addr = region->get_address();
    header = static_cast<ShareMemHeader*>(addr);
    buckets = static_cast<ShareMemBucket*>((void*)((char*)addr+sizeof(ShareMemHeader)));
}

void SharedMemoryContext::reset() {
    std::memset(mShareMem->get_address(), 0, mShareMem->get_size());
}

std::vector<int32_t> SharedMemoryContext::acquireBlock(FileId fileId)
{
    std::vector<int32_t> res;
    getMutex();

    int numBlocksToAcquire = calcBlockAcquireNum();

    /* pick up from free buckets */
    for(int32_t i=0; i<header->numBlocks; i++)
    {
        if(buckets[i].isFreeBucket())
        {
            buckets[i].setBucketActive();
            res.push_back(i);
            numBlocksToAcquire--;
        }
    }

    /* TODO: pick up from Used buckets and evict them */
    if (numBlocksToAcquire > 0)
    {
    }

    assert(numBlocksToAcquire == 0);

    releaseMutex();
    return res;
}

int SharedMemoryContext::calcBlockAcquireNum()
{
    return 1;
}

/* TODO: Use timed_wait() */
void SharedMemoryContext::getMutex()
{
    mSemaphore->wait();
    header->enter();
}

void SharedMemoryContext::releaseMutex()
{
    header->exit();
    mSemaphore->post();
}

}
}