#ifndef POOL_H_
#define POOL_H_

#include <stdlib.h>

struct pool_free {
	struct pool_free *next;
};

struct pool {
	void *area;
	size_t area_len;

	size_t ecount;
	size_t esize;

	struct pool_free *free;
};
typedef struct pool pool_t;

int pool_init(struct pool	*p,
     size_t					ecount,
     size_t					esize,
     size_t					align);
void pool_clean(struct pool *p);

void * pool_alloc(struct pool *p);
void pool_free(struct pool	*p,
     void					*elt);


#endif /* POOL_H_ */
