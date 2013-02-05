/*
 *  Common routine to implement atexit-like functionality.
 */

#include <stddef.h>
#include <stdlib.h>
#include <reent.h>
#include <sys/lock.h>
#include "atexit.h"

/* Following reference forces __call_exitproc to be linked in */
void * __atexit_dummy = &__call_exitprocs;

/* Make this a weak reference to avoid pulling in malloc.  */
__LOCK_INIT_RECURSIVE(, __atexit_lock);

/*
 * Register a function to be performed at exit or on shared library unload.
 */

int
_DEFUN (__do_register_exitproc,
	(type, fn, arg, d),
	int type _AND
	void (*fn) (void) _AND
	void *arg _AND
	void *d)
{
  struct _on_exit_args * args;
  register struct _atexit *p;

#ifndef __SINGLE_THREAD__
  __lock_acquire_recursive(__atexit_lock);
#endif

  p = _GLOBAL_REENT->_atexit;
  if (p == NULL)
    {
      _GLOBAL_REENT->_atexit = p = (struct _atexit *)malloc (sizeof (*p));
      memset (p, 0, sizeof (*p));
    }
  if (p->_ind >= _ATEXIT_SIZE)
    return -1;

  if (type != __et_atexit)
    {
      args = &p->_on_exit_args;
      args->_fnargs[p->_ind] = arg;
      args->_fntypes |= (1 << p->_ind);
      args->_dso_handle[p->_ind] = d;
      if (type == __et_cxa)
	args->_is_cxa |= (1 << p->_ind);
    }
  p->_fns[p->_ind++] = fn;
#ifndef __SINGLE_THREAD__
  __lock_release_recursive(__atexit_lock);
#endif
  return 0;
}
