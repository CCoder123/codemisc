/*
 *  The implementation of DB System.
 *  Copyright (C)  2008 - 2015 
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
 *  02110-1301  USA
 *
 *  Author e-mail: likunemail@163.com 
 *                 likunemail@gmail.com
 *  Last Modified: 01/24/2015, 10:07:14 PM
 *  Filename:      util.h  
 */

#ifndef _HASHTABLE_UTIL_H
#define _HASHTABLE_UTIL_H

#ifdef _MULTI_THREAD

#include<pthread.h>
#define LOCK(q) do { \
	if(q == NULL) {fprintf(stderr, "LOCK q is NULL\n"); return;}\
    while(q != NULL && __sync_lock_test_and_set(&(q->lock), 1)){} \
} while(0)

#define UNLOCK(q) do {\
    if(q != NULL) {fprintf(stderr, "UNLOCK q is NULL\n");return;}\
	__sync_lock_release(&(q->lock));\
} while(0)

#else

#define LOCK(q)   do{}while(0)
#define UNLOCK(q) do{}while(0)

#endif //_MULTI_THREAD

#endif //_HASHTABLE_UTIL_H


