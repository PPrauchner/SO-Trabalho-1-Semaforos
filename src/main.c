/*
 * Harness of the producer-consumer experiment.
 *
 * Responsibilities:
 * - Read the sync_mode from argv and run one experiment under it.
 * - Time only the concurrent phase and aggregate the checksums after the join.
 * - Print a single, parseable result line.
 */

#define _POSIX_C_SOURCE 200809L

#include "buffer.h"

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N_PRODUCERS 4
#define N_CONSUMERS 4
#define N_ITEMS 100000

#define ITEMS_PER_PRODUCER (N_ITEMS / N_PRODUCERS)
#define ITEMS_PER_CONSUMER (N_ITEMS / N_CONSUMERS)

/* Per-thread state: no field is shared, so nothing here needs a semaphore. */
typedef struct {
    Buffer *buffer;
    int index;
    int64_t checksum;
    int64_t item_count;
} ThreadState;

static void fail(const char *message)
{
    fprintf(stderr, "prodcons: %s\n", message);
    exit(EXIT_FAILURE);
}

/* Items come from the producer index and a local counter: no shared state. */
static int make_item(int producer_index, int local_counter)
{
    return producer_index * ITEMS_PER_PRODUCER + local_counter + 1;
}

static void *producer_main(void *argument)
{
    ThreadState *state = (ThreadState *)argument;

    for (int counter = 0; counter < ITEMS_PER_PRODUCER; counter++) {
        int item = make_item(state->index, counter);
        buffer_put(state->buffer, item);
        state->checksum += item;
        state->item_count++;
    }
    return NULL;
}

static void *consumer_main(void *argument)
{
    ThreadState *state = (ThreadState *)argument;

    for (int counter = 0; counter < ITEMS_PER_CONSUMER; counter++) {
        state->checksum += buffer_take(state->buffer);
        state->item_count++;
    }
    return NULL;
}

static double elapsed_ms(const struct timespec *start, const struct timespec *end)
{
    return (double)(end->tv_sec - start->tv_sec) * 1000.0
         + (double)(end->tv_nsec - start->tv_nsec) / 1000000.0;
}

static SyncMode parse_sync_mode(const char *name)
{
    if (strcmp(name, "full") == 0) {
        return SYNC_MODE_FULL;
    }
    fprintf(stderr, "prodcons: unknown sync_mode '%s' (expected: full)\n", name);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    Buffer buffer;
    pthread_t producers[N_PRODUCERS];
    pthread_t consumers[N_CONSUMERS];
    ThreadState producer_states[N_PRODUCERS];
    ThreadState consumer_states[N_CONSUMERS];
    struct timespec start;
    struct timespec end;
    int64_t produced_checksum = 0;
    int64_t consumed_checksum = 0;
    int64_t produced_items = 0;
    int64_t consumed_items = 0;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <sync_mode>\n", argv[0]);
        return EXIT_FAILURE;
    }

    SyncMode sync_mode = parse_sync_mode(argv[1]);

    if (buffer_init(&buffer, sync_mode) != 0) {
        fail("sem_init failed");
    }

    for (int i = 0; i < N_PRODUCERS; i++) {
        producer_states[i] = (ThreadState){ &buffer, i, 0, 0 };
    }
    for (int i = 0; i < N_CONSUMERS; i++) {
        consumer_states[i] = (ThreadState){ &buffer, i, 0, 0 };
    }

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < N_PRODUCERS; i++) {
        if (pthread_create(&producers[i], NULL, producer_main, &producer_states[i]) != 0) {
            fail("pthread_create failed for a producer");
        }
    }
    for (int i = 0; i < N_CONSUMERS; i++) {
        if (pthread_create(&consumers[i], NULL, consumer_main, &consumer_states[i]) != 0) {
            fail("pthread_create failed for a consumer");
        }
    }
    for (int i = 0; i < N_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }
    for (int i = 0; i < N_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    for (int i = 0; i < N_PRODUCERS; i++) {
        produced_checksum += producer_states[i].checksum;
        produced_items += producer_states[i].item_count;
    }
    for (int i = 0; i < N_CONSUMERS; i++) {
        consumed_checksum += consumer_states[i].checksum;
        consumed_items += consumer_states[i].item_count;
    }

    printf("sync_mode=%s produced_checksum=%lld consumed_checksum=%lld "
           "difference=%lld lost_items=%lld time_ms=%.3f\n",
           argv[1],
           (long long)produced_checksum,
           (long long)consumed_checksum,
           (long long)(produced_checksum - consumed_checksum),
           (long long)(produced_items - consumed_items),
           elapsed_ms(&start, &end));

    buffer_destroy(&buffer);
    return EXIT_SUCCESS;
}
