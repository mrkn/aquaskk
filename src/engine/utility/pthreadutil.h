/* -*- C++ -*-
 *
 * pthreadutil.h - C++ pthread wrapper.
 *
 *   Copyright (c) 2007-2008 Tomotaka SUWA, All rights reserved.
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

#ifndef pthreadutil_h
#define pthreadutil_h

#include <pthread.h>
#include <cerrno>
#include <cassert>

namespace pthread {
    class mutex {
	friend class lock;
	friend class condition;

	pthread_mutex_t handle_;

	mutex(const mutex&);
	mutex& operator=(const mutex&);

    public:
	mutex() {
	    pthread_mutex_init(&handle_, 0);
	};

	~mutex() {
	    pthread_mutex_destroy(&handle_);
	}
    };

    class suspend_cancel {
	void set(int state) {
	    int tmp;
	    pthread_setcancelstate(state, &tmp);
	}

	suspend_cancel(const suspend_cancel&);
	suspend_cancel& operator=(const suspend_cancel&);

    public:
	suspend_cancel() {
	    set(PTHREAD_CANCEL_DISABLE);
	}

	~suspend_cancel() {
	    set(PTHREAD_CANCEL_ENABLE);
	}
    };

    class lock {
	suspend_cancel shield_;
	mutex& target_;

	lock();
	lock(const lock&);
	lock& operator=(const lock&);

    public:
	lock(mutex& target) : target_(target) {
	    pthread_mutex_lock(&target_.handle_);
	}

	~lock() {
	    pthread_mutex_unlock(&target_.handle_);
	}
    };

    class condition {
	mutex mutex_;
	pthread_cond_t handle_;

	condition(const condition&);
	condition& operator=(const condition&);

	void trylock() {
	    assert(EBUSY == pthread_mutex_trylock(&mutex_.handle_) &&
		   "*** You MUST lock the pthread::condition object to avoid race conditions. ***");
	}

    public:
	condition() {
	    pthread_cond_init(&handle_, 0);
	}

	~condition() {
	    pthread_cond_destroy(&handle_);
	}

	operator mutex&() {
	    return mutex_;
	}

	void signal() {
	    trylock();

	    pthread_cond_signal(&handle_);
	}

	void broadcast() {
	    trylock();

	    pthread_cond_broadcast(&handle_);
	}

	bool wait() {
	    trylock();

	    return 0 == pthread_cond_wait(&handle_, &mutex_.handle_);
	}

	bool wait(long second, long nano_second = 0) {
	    trylock();

	    timespec abstime;
	    abstime.tv_sec = std::time(0) + second;
	    abstime.tv_nsec = nano_second;

	    return 0 == pthread_cond_timedwait(&handle_, &mutex_.handle_, &abstime);
	}
    };

    class task {
    public:
	virtual ~task() {}
	virtual bool run() = 0;
    };

    class timer {
	task* task_;
	long interval_;
	long startup_delay_;
	pthread_t thread_;

	static void* handler(void* param) {
	    timer* self = reinterpret_cast<timer*>(param);

	    self->run();

	    return 0;
	}

	void run() {
	    wait(startup_delay_);

	    while(task_->run()) {
		wait(interval_);
	    }
	}

	void wait(long second) {
	    if(!second) return;

	    timespec ts;
	    ts.tv_sec = second;
	    ts.tv_nsec = 0;

	    nanosleep(&ts, 0);
	}

	timer();
	timer(const timer&);
	timer& operator=(const timer&);

    public:
	timer(task* task, long interval, long startup_delay = 0)
	    : task_(task), interval_(interval), startup_delay_(startup_delay) {
	    if(pthread_create(&thread_, 0, timer::handler, this) != 0) {
		thread_ = 0;
	    }
	}

	~timer() {
	    if(thread_ && pthread_cancel(thread_) == 0) {
		pthread_join(thread_, 0);
	    }
	}
    };
};

#endif
