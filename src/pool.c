#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "pool.h"

#define MAX(a, b) ((a) < (b) ? (b) : (a))

#define _MASK(a) ((a) - 1)
#define ROUNDUP(v, a) (((uintptr_t)(v) + _MASK(a)) & ~(_MASK(a)))

static inline void *
_pool_aligned_malloc(size_t size_bytes, size_t align) {
	void *p, **p_al;

	/* store the adress of the block in first aligned (void *) */
	p = (void *)malloc(size_bytes + align - 1 + sizeof(void *));
	if (p == NULL)
		return NULL;

	p_al = (void **)ROUNDUP(p + sizeof(void *), align);
	*(p_al - 1) = p;

	return p_al;
}

static inline void
_pool_aligned_free(void *p) {
	free(((void **)p) - 1);
}

/* align is supposed to be a power of two */
int
pool_init(struct pool *p, size_t ecount, size_t esize, size_t align) {
	size_t area_len;
	struct pool_free *e;

	assert(p != NULL);

	esize = MAX(esize, sizeof(struct pool_free));
	esize = ROUNDUP(esize, align);

	area_len = ecount * esize;
	p->area = _pool_aligned_malloc(area_len, align);
	if (p->area == NULL)
		return 1;

	p->area_len = area_len;

	/* Fill freelist */
	p->free = p->area;

	e = p->area;
	while ((uintptr_t)e + esize < (uintptr_t)(p->area) + p->area_len) {
		e->next = (void *)((uintptr_t)e + esize);
		e = e->next;
	}
	e->next = NULL;

	p->ecount = ecount;
	p->esize = esize;

	return 0;
}

void
pool_clean(struct pool *p) {
	_pool_aligned_free(p->area);
}

void *
pool_alloc(struct pool *p) {
	void *e;

	if (p->free == NULL)
		return NULL;

	e = p->free;
	p->free = (p->free)->next;

	return e;
}

void
pool_free(struct pool *p, void *e) {
	struct pool_free *free_e;

	if (e == NULL)
		return;

	free_e = (struct pool_free *)e;
	free_e->next = p->free;
	p->free = free_e;
}
