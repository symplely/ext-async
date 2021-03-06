/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) Martin Schröder 2019                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Martin Schröder <m.schroeder2007@gmail.com>                 |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_VALGRIND
#include "valgrind/valgrind.h"
#endif

#include "php.h"
#include "zend.h"

#include "async/stack.h"

zend_bool async_fiber_stack_allocate(async_fiber_stack *stack, unsigned int size)
{
	size_t msize;

	stack->size = ((size_t) size + ASYNC_STACK_PAGESIZE - 1) / ASYNC_STACK_PAGESIZE * ASYNC_STACK_PAGESIZE;

#ifdef HAVE_MMAP

	void *pointer;

	msize = stack->size + ASYNC_FIBER_GUARDPAGES * ASYNC_STACK_PAGESIZE;
	pointer = mmap(0, msize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (pointer == (void *) -1) {
		pointer = mmap(0, msize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

		if (pointer == (void *) -1) {
			return 0;
		}
	}

#if ASYNC_FIBER_GUARDPAGES
	mprotect(pointer, ASYNC_FIBER_GUARDPAGES * ASYNC_STACK_PAGESIZE, PROT_NONE);
#endif

	stack->pointer = (void *)((char *) pointer + ASYNC_FIBER_GUARDPAGES * ASYNC_STACK_PAGESIZE);
#else
	stack->pointer = emalloc(stack->size);
	msize = stack->size;
#endif

	if (UNEXPECTED(!stack->pointer)) {
		return 0;
	}

#ifdef VALGRIND_STACK_REGISTER
	char * base;

	base = (char *) stack->pointer;
	stack->valgrind = VALGRIND_STACK_REGISTER(base, base + msize - ASYNC_FIBER_GUARDPAGES * ASYNC_STACK_PAGESIZE);
#endif

	return 1;
}

void async_fiber_stack_free(async_fiber_stack *stack)
{
	if (stack->pointer != NULL) {
#ifdef VALGRIND_STACK_DEREGISTER
		VALGRIND_STACK_DEREGISTER(stack->valgrind);
#endif

#ifdef HAVE_MMAP

		void *address;
		size_t len;

		address = (void *)((char *) stack->pointer - ASYNC_FIBER_GUARDPAGES * ASYNC_STACK_PAGESIZE);
		len = stack->size + ASYNC_FIBER_GUARDPAGES * ASYNC_STACK_PAGESIZE;

		munmap(address, len);
#else
		efree(stack->pointer);
#endif

		stack->pointer = NULL;
	}
}
