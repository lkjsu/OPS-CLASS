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
 * Driver code is in kern/tests/synchprobs.c We will replace that file. This
 * file is yours to modify as you see fit.
 *
 * You should implement your solution to the stoplight problem below. The
 * quadrant and direction mappings for reference: (although the problem is, of
 * course, stable under rotation)
 *
 *   |0 |
 * -     --
 *    01  1
 * 3  32
 * --    --
 *   | 2|
 *
 * As way to think about it, assuming cars drive on the right: a car entering
 * the intersection from direction X will enter intersection quadrant X first.
 * The semantics of the problem are that once a car enters any quadrant it has
 * to be somewhere in the intersection until it call leaveIntersection(),
 * which it should call while in the final quadrant.
 *
 * As an example, let's say a car approaches the intersection and needs to
 * pass through quadrants 0, 3 and 2. Once you call inQuadrant(0), the car is
 * considered in quadrant 0 until you call inQuadrant(3). After you call
 * inQuadrant(2), the car is considered in quadrant 2 until you call
 * leaveIntersection().
 *
 * You will probably want to write some helper functions to assist with the
 * mappings. Modular arithmetic can help, e.g. a car passing straight through
 * the intersection entering from direction X will leave to direction (X + 2)
 * % 4 and pass through quadrants X and (X + 3) % 4.  Boo-yah.
 *
 * Your solutions below should call the inQuadrant() and leaveIntersection()
 * functions in synchprobs.c to record their progress.
 */

#include <types.h>
#include <lib.h>
#include <thread.h>
#include <test.h>
#include <synch.h>

 int get_quadrant(uint32_t);
 int quadrant_change(int);
 struct semaphore *get_sem(int);
 struct semaphore* first_semaphore;
struct semaphore* second_semaphore;
struct semaphore *third_semaphore;
 struct semaphore *fourth_semaphore;

 struct lock* master_lock;

void
stoplight_init() {

    first_semaphore=sem_create("first_semaphore",1);
	second_semaphore=sem_create("second_semaphore",1);
	third_semaphore=sem_create("third_semaphore",1);
	fourth_semaphore=sem_create("fourth_semaphore",1);

  master_lock= lock_create("master_lock");

}
void stoplight_cleanup() {

	sem_destroy(first_semaphore);
	sem_destroy(second_semaphore);
	sem_destroy(third_semaphore);
	sem_destroy(fourth_semaphore);
	lock_destroy(master_lock);
}

void
turnright(uint32_t direction, uint32_t index)
{
  //lock_acquire(master_lock);
  //int current_quadrant=get_quadrant(direction);
 int current_quadrant=(int)(direction);
	P(get_sem(current_quadrant));
	inQuadrant(current_quadrant,index);

	//inQuadrant(current_quadrant,index);
	leaveIntersection(index);
	V(get_sem(current_quadrant));

	//lock_release(master_lock);
}
void
gostraight(uint32_t direction, uint32_t index)
{
	lock_acquire(master_lock);
	 //int current_quadrant=get_quadrant(direction);
	int current_quadrant=(int)(direction);
     P(get_sem(current_quadrant));
	inQuadrant(current_quadrant,index);
	int second_quadrant = quadrant_change(current_quadrant);
	P(get_sem(second_quadrant));
	lock_release(master_lock);
	lock_acquire(master_lock);
	inQuadrant(second_quadrant,index);
	V(get_sem(current_quadrant));

	leaveIntersection(index);
	V(get_sem(second_quadrant));
	lock_release(master_lock);

}
void
turnleft(uint32_t direction, uint32_t index)
{
	lock_acquire(master_lock);
	 //int current_quadrant=get_quadrant(direction);
	int current_quadrant=(int)(direction);
	P(get_sem(current_quadrant));
	inQuadrant(current_quadrant,index);

	int second_quadrant = quadrant_change(current_quadrant);
	P(get_sem(second_quadrant));
	lock_release(master_lock);
	lock_acquire(master_lock);
	inQuadrant(second_quadrant,index);
	V(get_sem(current_quadrant));
	int third_quadrant = quadrant_change(second_quadrant);
	P(get_sem(third_quadrant));
	lock_release(master_lock);
	lock_acquire(master_lock);
	inQuadrant(third_quadrant,index);
	V(get_sem(second_quadrant));
	leaveIntersection(index);
	V(get_sem(third_quadrant));

	lock_release(master_lock);
}

int quadrant_change(int direction){

    //int newdir = direction;

   return (direction+3)%4;
}

int get_quadrant(uint32_t direction){

	int newdir = (int) direction;

	return (newdir+2)%4;
}
 struct semaphore
 *get_sem(int direction){
	switch(direction){
		case 0:
			return first_semaphore;
			break;
		case 1:
			return second_semaphore;
			break;
		case 2:
			return third_semaphore;
			break;
		case 3:
			return fourth_semaphore;
			break;
		default:
			return NULL;
	}

 }
