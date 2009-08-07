/* -*- C++ -*-
 *
 * stringutil.h - string utilities
 *
 *   Copyright (c) 2009 Tomotaka SUWA, All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 * 
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the authors nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef	stringutil_h
#define stringutil_h

#include <sstream>
#include <numeric>

namespace string {
    class splitter {
        std::istringstream buf_;

        struct replace {
            std::string from_;
            std::string to_;
            unsigned index_;
            unsigned last_;
        
        public:
            replace(std::string from, std::string to)
                : from_(from), to_(to), index_(0), last_(from_.size() - 1) {}

            std::string& operator()(std::string& result, char ch) {
                if(ch != from_[index_]) {
                    result += from_.substr(0, index_);
                    result += ch;
                    index_ = 0;
                } else {
                    if(index_ == last_) {
                        result += to_;
                        index_ = 0;
                    } else {
                        ++ index_;
                    }
                }

                return result;
            }
        };

    public:
        void split(const std::string& str, const std::string& delimiter = ",") {
            std::string empty;
            buf_.clear();
            buf_.str(std::accumulate(str.begin(), str.end(),
                                     empty, replace(delimiter, " ")));
        }

        splitter& operator>>(std::string& str) {
            buf_ >> str;
            return *this;
        }

        operator bool() {
            return buf_;
        }
    };
};

#endif
