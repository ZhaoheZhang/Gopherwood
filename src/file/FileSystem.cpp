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
#include "FileSystem.h"
#include "common/Configuration.h"
#include "common/hash.h"
#include "core/ActiveStatus.h"

namespace Gopherwood {
namespace Internal {

FileSystem::FileSystem(const char *workDir) :
        workDir(workDir) {
    std::stringstream ss;
    ss << workDir << '/' << "-" << Configuration::LOCAL_SPACE_FILE;
    std::string filePath = ss.str();

    int flags = O_CREAT | O_RDWR;

    mLocalSpaceFile = open(filePath.c_str(), flags, 0644);

    mSharedMemoryContext = SharedMemoryManager::getInstance()->buildSharedMemoryContext(workDir);
    mActiveStatusContext = shared_ptr<ActiveStatusContext>(new ActiveStatusContext(mSharedMemoryContext));
}

FileId FileSystem::makeFileId(const std::string filePath)
{
    FileId id;

    /* hash the path to size_t */
    id.hashcode = StringHasher(filePath);

    /* if the hashcode collates with other files,
     * assign an identical collision id
     * TODO: Not implemented, need to check existing files */
    id.collisionId = 0;

    return id;
}

File* FileSystem::CreateFile(const char *fileName, int flags)
{
    FileId fileId;
    shared_ptr<ActiveStatus> status;

    fileId = makeFileId(std::string(fileName));
    status = mActiveStatusContext->initFileActiveStatus(fileId);

    std::string name(fileName);
    return new File(fileId, name, flags, mLocalSpaceFile, status);
}

File* FileSystem::OpenFile(const char *fileName, int flags)
{
    return NULL;
}

FileSystem::~FileSystem() {

}

}
}
