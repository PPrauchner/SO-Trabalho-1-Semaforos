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

/* Initialises a buffer in the given mode, failing the run if that is impossible. */
static void setup_mode(Buffer *buffer, SyncMode sync_mode, const char *test_name)
{
    check(buffer_init(buffer, sync_mode) == 0, test_name,
          "buffer_init returned -1");
}

/* Initialises a buffer in FULL mode, failing the run if that is impossible. */
static void setup(Buffer *buffer, const char *test_name)
{
    setup_mode(buffer, SYNC_MODE_FULL, test_name);
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

/*
 * NO_MUTEX keeps the counting semaphores, so with a single thread — where the
 * absent mutual exclusion cannot matter — the buffer still behaves as a FIFO.
 */
static void test_no_mutex_is_fifo_single_threaded(void)
{
    const char *name = "test_no_mutex_is_fifo_single_threaded";
    Buffer buffer;
    int i;

    setup_mode(&buffer, SYNC_MODE_NO_MUTEX, name);

    for (i = 0; i < BUFFER_SIZE; i++) {
        buffer_put(&buffer, 10 + i);
    }
    for (i = 0; i < BUFFER_SIZE; i++) {
        check(buffer_take(&buffer) == 10 + i, name, "item out of order");
    }

    buffer_destroy(&buffer);
    printf("PASS %s\n", name);
}

/*
 * NONE uses no semaphore at all. With a single thread and at most BUFFER_SIZE
 * resident items — the only regime where the missing capacity control cannot
 * bite — the indices still walk the array in order.
 */
static void test_none_is_fifo_single_threaded(void)
{
    const char *name = "test_none_is_fifo_single_threaded";
    Buffer buffer;
    int i;

    setup_mode(&buffer, SYNC_MODE_NONE, name);

    for (i = 0; i < BUFFER_SIZE; i++) {
        buffer_put(&buffer, 20 + i);
    }
    for (i = 0; i < BUFFER_SIZE; i++) {
        check(buffer_take(&buffer) == 20 + i, name, "item out of order");
    }

    buffer_destroy(&buffer);
    printf("PASS %s\n", name);
}

int main(void)
{
    test_fifo_order();
    test_wrap_around();
    test_capacity();
    test_no_mutex_is_fifo_single_threaded();
    test_none_is_fifo_single_threaded();
    return 0;
}
