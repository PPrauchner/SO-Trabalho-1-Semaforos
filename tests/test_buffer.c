/*
 * Deterministic single-thread tests for the public buffer_* API.
 *
 * Responsibilities:
 * - Exercise the buffer contract under SYNC_MODE_FULL, without threads.
 * - Separate a data-structure defect from a concurrency defect: if these
 *   fail, the synchronisation is not the suspect.
 *
 * Only the public API is used — indices, slots and semaphore values are
 * implementation and stay untouched here. Every call sequence keeps the
 * buffer between empty and BUFFER_SIZE, so no call can ever block.
 */

#include "../src/buffer.h"

#include <stdio.h>
#include <stdlib.h>

/* Reports the failing test on stderr and aborts the run with status 1. */
static void check(int condition, const char *test_name, const char *detail)
{
    if (!condition) {
        fprintf(stderr, "FAIL %s: %s\n", test_name, detail);
        exit(1);
    }
}

/* Initialises a buffer in FULL mode, failing the run if that is impossible. */
static void setup(Buffer *buffer, const char *test_name)
{
    check(buffer_init(buffer, SYNC_MODE_FULL) == 0, test_name,
          "buffer_init returned -1");
}

/* Items leave the buffer in the same order they entered it. */
static void test_fifo_order(void)
{
    const char *name = "test_fifo_order";
    Buffer buffer;
    int i;

    setup(&buffer, name);

    for (i = 1; i <= 5; i++) {
        buffer_put(&buffer, i);
    }
    for (i = 1; i <= 5; i++) {
        check(buffer_take(&buffer) == i, name, "item out of order");
    }

    buffer_destroy(&buffer);
    printf("PASS %s\n", name);
}

/*
 * The circular indices keep the sequence correct after crossing the end of
 * the array more than once.
 */
static void test_wrap_around(void)
{
    const char *name = "test_wrap_around";
    const int total = 3 * BUFFER_SIZE + 3;
    Buffer buffer;
    int i;

    setup(&buffer, name);

    /* Half a buffer stays resident, so every index wrap happens mid-stream. */
    for (i = 0; i < BUFFER_SIZE / 2; i++) {
        buffer_put(&buffer, i);
    }
    for (i = BUFFER_SIZE / 2; i < total; i++) {
        buffer_put(&buffer, i);
        check(buffer_take(&buffer) == i - BUFFER_SIZE / 2, name,
              "item out of order after wrap");
    }
    for (i = total - BUFFER_SIZE / 2; i < total; i++) {
        check(buffer_take(&buffer) == i, name, "resident item lost");
    }

    buffer_destroy(&buffer);
    printf("PASS %s\n", name);
}

/* Filling the buffer to capacity and draining it loses no item and duplicates none. */
static void test_capacity(void)
{
    const char *name = "test_capacity";
    Buffer buffer;
    int i;

    setup(&buffer, name);

    for (i = 0; i < BUFFER_SIZE; i++) {
        buffer_put(&buffer, 100 + i);
    }
    for (i = 0; i < BUFFER_SIZE; i++) {
        check(buffer_take(&buffer) == 100 + i, name,
              "item lost or duplicated at full capacity");
    }

    /* A second full cycle proves the buffer is empty and reusable. */
    for (i = 0; i < BUFFER_SIZE; i++) {
        buffer_put(&buffer, 200 + i);
    }
    for (i = 0; i < BUFFER_SIZE; i++) {
        check(buffer_take(&buffer) == 200 + i, name,
              "buffer not empty after draining");
    }

    buffer_destroy(&buffer);
    printf("PASS %s\n", name);
}

int main(void)
{
    test_fifo_order();
    test_wrap_around();
    test_capacity();
    return 0;
}
