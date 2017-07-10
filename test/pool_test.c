#include <stdio.h>
#include <string.h>
#include <valgrind/memcheck.h>

#include "pool.h"
#include "test.h"

#define POOL_SIZE 100
#define TEST_SIZE 14
struct test_elt {
	char a[TEST_SIZE];
};
#define WRITE_TEST_ELT(e) memset((e), 'q', sizeof(struct test_elt))

size_t pool_used;
#define pool_is_full(p) (pool_used == 0)
#define pool_is_empty(p) (pool_used == (p)->ecount)

/* Functional */
void test_single_alloc_free(struct test_suite *ts);
void test_alloc_all(struct test_suite *ts);
void test_alloc_free_random(struct test_suite *ts);
void test_check_alignement(struct test_suite *ts);

/* Valgrind memory errors */
void test_overflow_block(struct test_suite *ts);
void test_leak_block(struct test_suite *ts);
void test_write_freed_block(struct test_suite *ts);
void test_double_free(struct test_suite *ts);

void
test_single_alloc_free(struct test_suite *ts) {
	int ret;
	struct pool p;
	void *block;

	tests_header(ts, "Single alloc and free\n");
	pool_used = 0;

	test_print(ts, "Init pool...\n");
	ret = pool_init(&p, POOL_SIZE, sizeof(struct test_elt), sizeof(int));
	test_ok_print(ts, "pool_init success", ret == 0);
	test_ok_print(ts, "pool full", pool_is_full(&p) == 1);
	test_ok_print(ts, "pool not empty", pool_is_empty(&p) == 0);

	test_print(ts, "Alloc a block...\n");
	block = pool_alloc(&p);
	test_ok_print(ts, "pool_alloc sucess", block != NULL);
	if (block == NULL) {
		test_print(ts, "block not allocated, abort test case\n");
		goto clean;
	}
	pool_used++;

	test_ok_print(ts, "pool not full", pool_is_full(&p) == 0);
	test_ok_print(ts, "pool not empty", pool_is_empty(&p) == 0);

	test_print(ts, "Write block...\n");
	WRITE_TEST_ELT(block);

	test_print(ts, "Free block...\n");
	pool_free(&p, block);
	pool_used--;
	test_ok_print(ts, "pool full", pool_is_full(&p) == 1);
	test_ok_print(ts, "pool not empty", pool_is_empty(&p) == 0);

clean:
	test_print(ts, "Clean pool...\n");
	pool_clean(&p);
}

/* TODO count number of allocation .... */
void
test_alloc_all(struct test_suite *ts) {
	struct pool p;
	void *block[POOL_SIZE]; /* store each element adress */
	size_t i;

	tests_header(ts, "Alloc all\n");
	pool_used = 0;

	test_print(ts, "Init pool...\n");
	pool_init(&p, POOL_SIZE, sizeof(struct test_elt), sizeof(int));

	test_print(ts, "Alloc and write each memory block...\n");
	for (i = 0; i < POOL_SIZE; i++) {
		test_ok(ts, pool_is_empty(&p) == 0);
		block[i] = pool_alloc(&p);
		test_ok(ts, block[i] != NULL);
		if (block[i] == NULL) {
			continue;
		}
		pool_used++;
		test_ok(ts, pool_is_full(&p) == 0);

		WRITE_TEST_ELT(block[i]);
	}

	test_ok_print(ts, "All element allocated", pool_used == p.ecount);

	test_ok_print(ts, "pool empty", pool_is_empty(&p) == 1);
	test_ok_print(ts, "alloc null", pool_alloc(&p) == NULL);

	test_print(ts, "Free all blocks...\n");
	for (i = 0; i < POOL_SIZE; i++) {
		if (block[i] == NULL) {
			continue;
		}
		pool_used--;
		pool_free(&p, block[i]);
	}
	test_ok_print(ts, "pool full", pool_is_full(&p) == 1);

	test_print(ts, "Clean pool...\n");
	pool_clean(&p);
}

void
test_alloc_free_random(struct test_suite *ts) {
	struct pool p;
	void *block[POOL_SIZE]; /* store each element adress */
	unsigned int indexes[] = {10, 0, 50, 6, 90};
	unsigned int indexes_len = 5;
	unsigned int i;

	tests_header(ts, "Random alloc and free\n");
	pool_used = 0;

	test_print(ts, "Init pool...\n");
	pool_init(&p, POOL_SIZE, sizeof(struct test_elt), sizeof(int));

	test_print(ts, "Alloc and write in each memory block...\n");
	for (i = 0; i < POOL_SIZE; i++) {
		block[i] = pool_alloc(&p);
		test_ok(ts, block[i] != NULL);
		if (block[i] == NULL) {
			continue;
		}
		pool_used++;
		WRITE_TEST_ELT(block[i]);
	}

	test_print(ts, "Free some choosen blocks...\n");
	for (i = 0; i < indexes_len; i++) {
		if (block[indexes[i]] == NULL) {
			test_print(ts, "block %u not allocated, continue\n", indexes[i]);
			continue;
		}
		pool_free(&p, block[indexes[i]]);
		pool_used--;
	}

	test_print(ts, "Re-alloc previously freed blocks and write in it...\n");
	for (i = 0; i < indexes_len; i++) {
		block[indexes[i]] = pool_alloc(&p);
		test_ok(ts, block[indexes[i]] != NULL);
		if (block[indexes[i]] == NULL) {
			test_print(ts, "block %u not allocated, continue\n", indexes[i]);
			continue;
		}
		pool_used++;
		WRITE_TEST_ELT(block[indexes[i]]);
	}

	test_print(ts, "Free all blocks...\n");
	for (i = 0; i < POOL_SIZE; i++) {
		if (block[i] == NULL) {
			test_print(ts, "block %u not allocated, continue free loop\n", i);
			continue;
		}
		pool_free(&p, block[i]);
		pool_used--;
	}

	test_print(ts, "Clean pool...\n");
	pool_clean(&p);
}

void
test_check_alignement(struct test_suite *ts) {
	struct pool p;
	void *block[POOL_SIZE]; /* store each element adress */
	unsigned int i;

	tests_header(ts, "Check alignement\n");
	pool_used = 0;

	test_print(ts, "Init pool with element size %zu...\n", sizeof(struct
		 test_elt));
	pool_init(&p, POOL_SIZE, sizeof(struct test_elt), sizeof(int));
	test_print(ts, "Final pool element size : %zu\n", p.esize);

	test_print(ts, "Alloc each memory block...\n");
	for (i = 0; i < POOL_SIZE; i++) {
		block[i] = pool_alloc(&p);
		if (block[i] == NULL) {
			test_print(ts, "block %u not allocated, continue alloc loop\n", i);
			continue;
		}
		pool_used++;
		test_ok(ts, (size_t)block[i] % sizeof(int) == 0);
	}

	test_print(ts, "Free all blocks...\n");
	for (i = 0; i < POOL_SIZE; i++) {
		if (block[i] == NULL) {
			test_print(ts, "block %u not allocated, continue free loop\n", i);
			continue;
		}
		pool_used--;
		pool_free(&p, block[i]);
	}
	test_ok_print(ts, "pool full", pool_is_full(&p) == 1);

	test_print(ts, "Clean pool...\n");
	pool_clean(&p);
}

void
test_overflow_block(struct test_suite *ts) {
	struct pool p;
	void *block[2];
	void *block_1_address;
	void *block_1_realloc_address;

	tests_header(ts, "Valgrind write outside of block\n");
	pool_used = 0;

	test_print(ts, "Init pool...\n");
	pool_init(&p, POOL_SIZE, sizeof(struct test_elt), sizeof(int));

	test_print(ts, "Alloc block 0...\n");
	block[0] = pool_alloc(&p);

	test_print(ts, "Alloc block 1...\n");
	block[1] = pool_alloc(&p);
	block_1_address = block[1];

	test_print(ts, "Release block 1...\n");
	pool_free(&p, block[1]);

	test_print(ts, "Write %u bytes to the block 0 of size %zu...\n", 17,
		 sizeof(struct
		 test_elt));
	memcpy(block[0], "aaaaaaaaaaaaaaaaaaaaaaaaaaa",  sizeof(struct test_elt) +
		 1);

	test_print(ts, "Re-alloc block 1...\n");
	block_1_realloc_address = pool_alloc(&p);

	test_ok_print(ts, "test if free list not corrupted\n", block_1_realloc_address == block_1_address);

	pool_free(&p, block_1_realloc_address);
	pool_free(&p, block[0]);

	test_print(ts, "Clean pool...\n");
	pool_clean(&p);
}

void
test_leak_block(struct test_suite *ts) {
	struct pool p;
	void *block;

	tests_header(ts, "Valgrind check leak\n");
	pool_used = 0;

	test_print(ts, "Init pool...\n");
	pool_init(&p, POOL_SIZE, sizeof(struct test_elt), sizeof(int));

	test_print(ts, "Alloc a block...\n");
	block = pool_alloc(&p);

	test_print(ts, "Write block...\n");
	WRITE_TEST_ELT(block);

	test_print(ts, "Enforce memory leak check...\n");
	VALGRIND_DO_LEAK_CHECK;

	test_print(ts, "Clean pool...\n");
	pool_clean(&p);
}

void
test_write_freed_block(struct test_suite *ts) {
	struct pool p;
	void *block;

	tests_header(ts, "Valgrind write in freed block\n");
	pool_used = 0;

	test_print(ts, "Init pool...\n");
	pool_init(&p, POOL_SIZE, sizeof(struct test_elt), sizeof(int));

	test_print(ts, "Alloc a block...\n");
	block = pool_alloc(&p);

	test_print(ts, "Free block...\n");
	pool_free(&p, block);

	test_print(ts, "Write block...\n");
	WRITE_TEST_ELT(block);

	test_print(ts, "Clean pool...\n");
	pool_clean(&p);
}

void
test_double_free(struct test_suite *ts) {
	struct pool p;
	void *block;

	tests_header(ts, "Valgrind double free\n");
	pool_used = 0;

	test_print(ts, "Init pool...\n");
	pool_init(&p, POOL_SIZE, sizeof(struct test_elt), sizeof(int));

	test_print(ts, "Alloc block...\n");
	block = pool_alloc(&p);

	test_print(ts, "Double free block...\n");
	pool_free(&p, block);
	pool_free(&p, block);

	test_print(ts, "Clean pool...\n");
	pool_clean(&p);
}

int
main(void) {
	struct test_suite ts;

	tests_init(&ts, stdout);

	/* Functional tests */
	test_single_alloc_free(&ts);
	tests_display_results(&ts);

	test_alloc_all(&ts);
	tests_display_results(&ts);

	test_alloc_free_random(&ts);
	tests_display_results(&ts);

	test_check_alignement(&ts);
	tests_display_results(&ts);

	/* Valgrind tests */
	test_overflow_block(&ts);
	test_leak_block(&ts);
	test_write_freed_block(&ts);
	test_double_free(&ts);

	return 0;
}
