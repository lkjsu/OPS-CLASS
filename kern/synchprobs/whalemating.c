/*
 * Copyright (c) 2001, 2002, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Driver code is in kern/tests/synchprobs.c We will
 * replace that file. This file is yours to modify as you see fit.
 *
 * You should implement your solution to the whalemating problem below.
 */

#include <types.h>
#include <lib.h>
#include <thread.h>
#include <test.h>
#include <synch.h>
#include <wchan.h>

/*
 * Called by the driver during initialization.
 */
 // static struct cv* male_cv= NULL;
 // static struct cv* female_cv= NULL;
 // static struct cv* matchmaker_cv= NULL;
 //  volatile int male_count;
 // volatile int female_count;
 // volatile int matchmaker_count;
  //static struct lock* male_lock= NULL;
 // static struct lock* female_lock=NULL;
  struct semaphore* male_semaphore;
	struct semaphore* female_semaphore;
	

	// struct wchan master_wchan;
	// struct spinlock master_s

void whalemating_init() {
	  // male_cv= cv_create("male_cv");
	  // female_cv= cv_create("female_cv");
	  // matchmaker_cv= cv_create("matchmaker_cv");
		male_semaphore=sem_create("male_sempahore",0);
		female_semaphore=sem_create("female_semaphore",0);
		// master_wchan = wchan_create
	  // master_lock=lock_create("master_lock");

}

/*
 * Called by the driver during teardown.
 */

void
whalemating_cleanup() {
  // cv_destroy(male_cv);
	// cv_destroy(female_cv);
	// cv_destroy(matchmaker_cv);

	sem_destroy(male_semaphore);
	sem_destroy(female_semaphore);
	// lock_destroy(male_lock);
	// lock_destroy (female_lock);
	// lock_destroy(master_lock);
	// return;
}

void
male(uint32_t index)
{
 male_start(index);
 P(male_semaphore);
	male_end(index);
}

void
female(uint32_t index)
{
	female_start(index);
	P(female_semaphore);
	female_end(index);
}

void
matchmaker(uint32_t index)
{
	matchmaker_start(index);
	V(female_semaphore);
	V(male_semaphore);
	matchmaker_end(index);
}
