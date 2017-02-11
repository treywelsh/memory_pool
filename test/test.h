#ifndef TEST_H_
#define TEST_H_

#include <stdio.h>
#include <limits.h>

struct test_suite {
	unsigned int tests_count;
	unsigned int tests_failed;
	FILE *s;
};


#define tests_init(ts, strm) do { \
		(ts)->tests_count = 0; \
		(ts)->tests_failed = 0; \
		(ts)->s = (strm); \
		if (setvbuf((strm), NULL, _IONBF, 0)) { \
			fprintf((strm), \
				 "WARN(test.h <tests_init>): setvbuf failed, messages will be buffered\n"); \
		} \
} while (0)


#define test_ok(ts, pass) do { \
		if ((ts)->tests_count > UINT_MAX - 1) { \
			fprintf((ts)->s, "WARN(test.h <test_ok>): integer overflow\n");  \
		} \
		((ts)->tests_count)++; \
		if (!(pass)) { \
			((ts)->tests_failed)++; \
		} \
} while (0)

#define test_ok_print(ts, str, pass) do { \
		char *pass_str; \
		test_ok((ts), (pass)); \
		if (pass) { \
			pass_str = "PASS"; \
		} else { \
			pass_str = "FAIL"; \
		} \
		fprintf((ts)->s, "%-40s %s\n", (str), pass_str); \
} while (0)

/* C99 */
#define test_print(ts, FMT, ...) do { \
		fprintf((ts)->s, \
			 "TEST(%s:%d <%s>): " FMT, \
			 __FILE__, __LINE__, __func__, ## __VA_ARGS__); \
} while (0)


#define tests_display_results(ts) \
	fprintf((ts)->s, \
	"===== %-6s : %-4u =====\n===== %-6s : %-4u =====\n", \
	"FAILED", (ts)->tests_failed, "TOTAL", (ts)->tests_count) \

#define tests_header(ts, name) \
	fprintf((ts)->s, \
	"=================================================\n          TEST %s=================================================\n", \
	(name)); \

#endif /* TEST_H_ */
