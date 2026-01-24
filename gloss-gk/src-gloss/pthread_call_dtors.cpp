#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include "syscalls.h"
#include "deferred.h"
#include "cmpxchg.h"

#ifndef PTHREAD_DESTRUCTOR_ITERATIONS
#define PTHREAD_DESTRUCTOR_ITERATIONS 16
#endif

void _pthread_call_dtors()
{
	size_t ndtors = 0;
	dtor_t *dtors = nullptr;
	void **vals = nullptr;

	// first get required number
	__syscall_get_pthread_dtors_params p { .len = &ndtors, .dtors = dtors, .vals = vals };
	deferred_call(__syscall_get_pthread_dtors, &p);
	
	if(ndtors > 0)
	{
		// allocate space for them
		dtors = (dtor_t *)malloc(ndtors * sizeof(dtor_t));
		vals = (void **)malloc(ndtors * sizeof(void *));

		if(dtors && vals)
		{
			/* Run destructors for the current thread until we have no more to run */
			for(int i = 0; i < PTHREAD_DESTRUCTOR_ITERATIONS; i++)
			{
				// keep reallocating buffer on the assumption ndtors changed
				while(true)
				{
					p.len = &ndtors;
					p.dtors = dtors;
					p.vals = vals;

					if(deferred_call(__syscall_get_pthread_dtors, &p) == 0)
						break;

					if(ndtors == 0)
					{
						if(dtors) free(dtors);
						if(vals) free(vals);
						dtors = nullptr;
						vals = nullptr;
						break;
					}

					dtors = (dtor_t *)realloc(dtors, ndtors * sizeof(dtor_t));
					vals = (void **)realloc(vals, ndtors * sizeof(void *));

					if(!dtors || !vals)
						break;
				}

				if(dtors && vals && ndtors)
				{
					bool has_non_null = false;
					for(unsigned int cdtor = 0; cdtor < ndtors; cdtor++)
					{
						if(dtors[cdtor] && vals[cdtor])
						{
							has_non_null = true;
							dtors[cdtor](vals[cdtor]);
						}
					}
					if(!has_non_null)
						break;
				}
				else
				{
					break;
				}
			}
		}

		if(dtors) free(dtors);
		if(vals) free(vals);
	}
}
