/********************************************************************
 * Copyright (c) 2013 - 2014, Pivotal Inc.
 * All rights reserved.
 *
 * Author: Zhanwei Wang
 ********************************************************************/
/********************************************************************
 * 2014 -
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

#ifndef _GOPHERWOOD_CORE_INPUTSTREAMINTER_H_
#define _GOPHERWOOD_CORE_INPUTSTREAMINTER_H_

#include <string>
#include <thread>

namespace Gopherwood {
    namespace Internal {

        using namespace ::std;

        class FileSystemInter;

/**
 * A input stream used read data from hdfs.
 */
        class InputStreamInter {
        public:
            virtual ~InputStreamInter() {
            }

            /**
         * Open a file to read
         * @param fs gopherwood file system.
         * @param fileName the name of the file to be read.
         * @param verifyChecksum verify the checksum.
         */
//            virtual void open(shared_ptr<FileSystemInter> fs, const char *fileName, bool verifyChecksum = true);

            /**
             * To read data from gopherwood.
             * @param buf the buffer used to filled.
             * @param size buffer size.
             * @return return the number of bytes filled in the buffer, it may less than size.
             */
            virtual int32_t read(char *buf, int32_t size)=0;

            /**
             * To read data from gopherwood, block until get the given size of bytes.
             * @param buf the buffer used to filled.
             * @param size the number of bytes to be read.
             */
            virtual void readFully(char *buf, int64_t size)=0;

            /**
             * Get how many bytes can be read without blocking.
             * @return The number of bytes can be read without blocking.
             */
            virtual int64_t available()=0;

            /**
             * To move the file point to the given position.
             * @param pos the given position.
             */
            virtual int64_t seek(int64_t pos)=0;

            /**
             * To get the current file point position.
             * @return the position of current file point.
             */
            virtual int64_t tell()=0;

            /**
             * Close the stream.
             */
            virtual void close()=0;

            /**
             * Output a readable string of this input stream.
             */
            virtual string toString() = 0;

            virtual void deleteFile()=0;

            virtual std::shared_ptr<FileStatus> getFileStatus()=0;
        };

    }
}
#endif /* _GOPHERWOOD_CORE_INPUTSTREAMINTER_H_ */