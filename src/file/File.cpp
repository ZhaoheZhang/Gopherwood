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
#include "file/File.h"
#include "client/gopherwood.h"
#include "common/Exception.h"
#include "common/ExceptionInternal.h"

namespace Gopherwood {
namespace Internal {

File::File(FileId id, std::string fileName, int flags, int fd, shared_ptr<ActiveStatus> status) :
        id(id), name(fileName), flags(flags), localFD(fd), mStatus(status) {
    if ((flags & GW_WRONLY) || (flags & GW_RDWR)){
        mOutStream = shared_ptr<OutputStream>(new OutputStream(localFD, status));
    }
    else{
        mOutStream = NULL;
    }

    if ((flags & GW_RDONLY) || (flags & GW_RDWR)) {
        mInStream = shared_ptr<InputStream>(new InputStream(localFD, status));
    }
    else{
        mInStream = NULL;
    }
}

void File::read(char *buffer, int64_t length) {
    mInStream->read(buffer, length);
}

void File::write(const char *buffer, int64_t length) {
    mOutStream->write(buffer, length);
}

void File::seek(int64_t pos, int mode) {
    int64_t eof = mStatus->getEof();
    int64_t targetPos = -1;

    if (mode == SEEK_SET) {
        targetPos = pos;
    } else if (mode == SEEK_CUR){
        targetPos = mStatus->getPosition() + pos;
    } else if (mode == SEEK_END){
        targetPos = eof + pos;
    }

    if (targetPos >= 0 && targetPos <= eof){
        mStatus->setPosition(targetPos);
    } else{
        THROW(GopherwoodInvalidParmException,
              "[File::seek] target offset %ld exceed Eof %ld",
              targetPos, eof);
    }
}

void File::close() {
    mStatus->close();

    if (mOutStream){
        mOutStream->close();
    }

    if (mInStream){
        mInStream->close();
    }
}

File::~File() {
}

}
}
