/* TODO
*   - find shortest distance between i and j given a distance matrix = DONE
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define DISCONNECT (UINT32_MAX / 2)

typedef uint32_t priority;

typedef union {
    void* ptr;
    uint64_t u64;
    uint8_t u8[8];
} eight_bytes;

typedef struct PQEntry PQEntry;
struct PQEntry{
    priority prio;
    eight_bytes data;
};

typedef struct PrioQueue PrioQueue;
struct PrioQueue {
    PQEntry* entries;
    size_t count;
    size_t MAX_SIZE;
};


PrioQueue
pq_create(const size_t max_size) {

    PQEntry* data_ptr = calloc(max_size, sizeof(PQEntry));

    return (PrioQueue) {
        .entries = data_ptr,
        .count = 0,
        .MAX_SIZE = max_size
    };
}


void
pq_free(PrioQueue pq) {
    
    free(pq.entries);
}


void
pq_print(const PrioQueue pq[static 1]) {

    if (!pq->count) {

        return;
    }
    
    const size_t stop = pq->count - 1;
    for (size_t i = 0; i < stop; i++) {

        printf("%i ", pq->entries[i].prio);
    }
    
    printf("%i", pq->entries[stop].prio);
}


size_t
pq_insert(const eight_bytes d, const priority p,
          PrioQueue pq[static 1]) {

    if (pq->count == pq->MAX_SIZE) {

        exit(EXIT_FAILURE);
    }
    
    size_t pos = pq->count;
    PQEntry new_entry = {.prio = p, .data = d};
    pq->entries[pos] = new_entry;
    
    while (pos != 0 && pq->entries[(pos - 1) >> 1].prio > p) {
        
        const size_t parent = (pos - 1) >> 1;
        PQEntry temp = pq->entries[parent];
        pq->entries[parent] = new_entry;
        pq->entries[pos] = temp;
        pos = parent;
    }
    
    pq->count++;
    return pos;
}


PQEntry
pq_pop(PrioQueue pq[static 1]) {
    
    if (!pq->count) {

        exit(EXIT_FAILURE);
    }

    const PQEntry popped = pq->entries[0];
    const PQEntry bubble = pq->entries[pq->count - 1];
    pq->entries[0] = bubble;
    pq->count--;
    
    size_t pos = 0;
    size_t left = 2 * pos + 1;
    size_t right = 2 * pos + 2;

    while (left < pq->count) {
        
        size_t swap = pos;

        if (right >= pq->count) {
            
            if (pq->entries[left].prio > bubble.prio) {

                swap = left;
            }

        } else if (pq->entries[right].prio < pq->entries[left].prio) {
            
            if (pq->entries[right].prio < bubble.prio) {

                swap = right;
            }

        } else {

            if (pq->entries[left].prio < bubble.prio) {

                swap = left;
            }
        }
    
        if (swap != pos) {

            const PQEntry temp = pq->entries[pos];
            pq->entries[pos] = pq->entries[swap];
            pq->entries[swap] = temp;
            
            pos = swap;
            left = 2 * pos + 1;
            right = 2 * pos + 2;

        } else {

            break;
        }
    }

    return popped;
}


priority
dijkstra_path(const size_t N, size_t path[static N],
              const size_t x, const size_t y,
              const priority D[static N*N]) {
    
    const priority NOEDGE = DISCONNECT;
    
    // Keeps track of distances from x to other nodes.
    priority* distances = calloc(N, sizeof(priority));
    if (!distances) {

        exit(EXIT_FAILURE);
    }

    // Keeps track of the direction of the shortest path from x.
    size_t* prev = calloc(N, sizeof(size_t));
    if (!prev) {

        free(distances);
        exit(EXIT_FAILURE);
    }
    
    for (size_t i = 0; i < N; i++) {
        
        distances[i] = NOEDGE;
        prev[i] = N;
    }

    PrioQueue pq = pq_create(N);
    if (!pq.entries) {
        
        free(distances);
        free(prev);
        exit(EXIT_FAILURE);
    }

    pq_insert((eight_bytes){.u64 = x}, 0, &pq);
    distances[x] = 0;
    
    while (pq.count) {
        
        PQEntry top = pq_pop(&pq);
        size_t cur_node = top.data.u64;

        for (size_t i = 0; i < N; i++) {
            
            priority dist = D[cur_node * N + i];

            if (dist > distances[i] || cur_node == i) {

                continue;
            }

            priority new_dist = dist + distances[cur_node];
        
            if (new_dist < distances[i]) {

                distances[i] = new_dist;
                pq_insert((eight_bytes){.u64 = i}, new_dist, &pq);
                prev[i] = cur_node;
            }
        }
    }
    
    priority shortest_path = distances[y];
    
    if (shortest_path != NOEDGE) {

        size_t cur_node = y;
        size_t i = N - 1;

        for (; cur_node != x; cur_node = prev[cur_node], --i) {

            path[i] = cur_node;
        }

        path[0] = x;

        for (size_t j = 1; j < N - i; j++) {

            path[j] = path[j + i];
        }
    }

    pq_free(pq);
    free(prev);
    free(distances);
    return shortest_path;
}


int
main() {

    #define LEN 11

    const priority N = DISCONNECT;

    /*
    *     1
    *    / \
    *   0---3---8
    *   |\ /|  /|
    *   4-2 5-7 |
    *    \|    \|
    *     6-----9
    *      \   /
    *       \ /
    *       10
    */   
    const priority D[LEN*LEN] = {
         0,  3,  4, 12,  3,  N,  N,  N,  N,  N,  N,
         3,  0,  N,  4,  N,  N,  N,  N,  N,  N,  N,
         4,  N,  0,  8,  2,  N,  5,  N,  N,  N,  N,
        12,  4,  8,  0,  N,  2,  N,  N,  1,  N,  N,
         3,  N,  2,  N,  0,  N,  8,  N,  N,  N,  N,
         N,  N,  N,  2,  N,  0,  N,  4,  N,  N,  N,
         N,  N,  5,  N,  8,  N,  0,  8,  N,  5, 10,
         N,  N,  N,  N,  N,  4,  8,  0,  3,  2,  N,
         N,  N,  N,  N,  N,  N,  N,  3,  0,  6,  N,
         N,  N,  N,  N,  N,  N,  5,  2,  6,  0,  4,
         N,  N,  N,  N,  N,  N, 10,  N,  N,  4,  0
    };

    size_t path[LEN] = {0};
    const size_t start = 4;
    const size_t end = 9;
    
    printf("Distance: %u\n", dijkstra_path(LEN, path, start, end, D));
    printf("Path: ");
    for (size_t i = 0; path[i] != end && i < LEN; i++) {
        
        printf("%zu -> ", path[i]);
    }
    printf("%zu\n", end);

    exit(EXIT_SUCCESS);
}
