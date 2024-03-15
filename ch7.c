/* TODO:
*   - breadth-first search given an adjacency matrix = DONE
*   - find connected components = DONE
*   - find a spanning tree = DONE
*/

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

struct Node {
    size_t parent;
    size_t value;
};

bool
search(const size_t x, const size_t start, const bool* adjmat,
       const size_t N) {

    if (x == start) {

        return true;
    }
    
    bool found = false;

    bool* visited = (bool*)calloc(N, sizeof(bool));
    if (!visited) {
        
        exit(EXIT_FAILURE);
    }

    size_t* frontier = (size_t*)malloc(N * sizeof(size_t));
    if (!frontier) {
    
        free(visited);
        exit(EXIT_FAILURE);
    }
    
    visited[start] = true;
    frontier[0] = start;

    size_t q_start = 0;
    size_t q_end = 1;

    while (q_start < q_end) {

        const size_t current_node = frontier[q_start];

        for (size_t i = 0; i < N; i++) {

            if (adjmat[current_node * N + i] && !visited[i]) {

                frontier[q_end] = i;
                q_end++;
                visited[i] = true;

                if (i == x) {

                    found = true;
                    break;
                }
            }
        }

        q_start++;
    }

    free(visited);
    free(frontier);
    return found;
}


size_t
connected(size_t con[], const size_t start, const bool* adjmat,
          const size_t N) {
    
    size_t count = 0;
    
    bool* visited = (bool*)calloc(N, sizeof(bool));
    if (!visited) {
        
        exit(EXIT_FAILURE);
    }

    size_t* frontier = (size_t*)malloc(N * sizeof(size_t));
    if (!frontier) {
    
        free(visited);
        exit(EXIT_FAILURE);
    }
    
    visited[start] = true;
    frontier[0] = start;

    size_t q_start = 0;
    size_t q_end = 1;

    while (q_start < q_end) {

        const size_t current_node = frontier[q_start];

        for (size_t i = 0; i < N; i++) {

            if (adjmat[current_node * N + i] && !visited[i]) {

                frontier[q_end] = i;
                q_end++;
                con[count] = i;
                count++;
                visited[i] = true;
                
            }
        }

        q_start++;
    }

    free(visited);
    free(frontier);
    return count;
}


size_t
spanning_tree(struct Node* tree, const size_t start, const bool* adjmat,
              const size_t N) {
    
    bool* visited = (bool*)calloc(N, sizeof(bool));
    if (!visited) {
        
        exit(EXIT_FAILURE);
    }

    size_t* frontier = (size_t*)malloc(N * sizeof(size_t));
    if (!frontier) {
    
        free(visited);
        exit(EXIT_FAILURE);
    }
    
    visited[start] = true;
    frontier[0] = start;
    tree[0] = (struct Node) {start, start};

    size_t q_start = 0;
    size_t q_end = 1;
    size_t tree_size = 1;

    while (q_start < q_end) {

        const size_t current_node = frontier[q_start];

        for (size_t i = 0; i < N; i++) {

            if (adjmat[current_node * N + i] && !visited[i]) {

                frontier[q_end] = i;
                q_end++;
                tree[tree_size] = (struct Node) {current_node, i};
                tree_size++;
                visited[i] = true;
                
            }
        }

        q_start++;
    }

    free(visited);
    free(frontier);
    return tree_size;
}


void
print_subtree(const size_t start, const size_t depth,
              const struct Node* tree, const size_t N) {
    
    bool has_children = false;

    for (size_t d = 0; d < depth; d++) {
        
        printf("  ");
    }

    printf("%zu", tree[start].value);
    
    for (size_t i = 0; i < N; i++) {

        if (tree[i].parent == tree[start].value && i != start) {

            if (!has_children) {

                has_children = true;
                printf(":\n");
            }

            print_subtree(i, depth + 1, tree, N);
        }
    }

    if (!has_children) {

        printf("\n");
    }
}


void
print_tree(const struct Node* tree, const size_t N) {

    size_t root_index = 0;
    bool root_found = false;

    for (size_t i = 0; i < N; i++) {
        
        if (tree[i].parent == tree[i].value) {
            
            root_index = i;
            root_found = true;
            break;
        }
    }

    if (!root_found) {
        
        exit(EXIT_FAILURE);
    }
    
    print_subtree(root_index, 0, tree, N);
}


int
main() {

    const bool G1[6 * 6] = {
        0, 1, 1, 0, 0, 0,
        1, 0, 1, 1, 0, 0,
        1, 1, 0, 0, 0, 0,
        0, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1,
        0, 0, 0, 0, 1, 0
    };

    size_t G1con[6] = {0};
    size_t G1count = connected(G1con, 2, G1, 6);
    struct Node G1tree[6] = {0};
    size_t G1tree_size = spanning_tree(G1tree, 2, G1, 6);

    printf("3 connected to 0: %s\n", (search(0, 3, G1, 6)) ? "true" : "false");
    printf("2 connected to 5: %s\n", (search(5, 2, G1, 6)) ? "true" : "false");
    printf("5 connected to 4: %s\n", (search(4, 5, G1, 6)) ? "true" : "false");
    printf("1 connected to 1: %s\n", (search(1, 1, G1, 6)) ? "true" : "false");

    printf("Connected to 2:\n");
    for (size_t i = 0; i < G1count; i++) {
        printf("%zu ", G1con[i]);
    }
    printf("\n");

    printf("Tree of 2:\n");
    for (size_t i = 0; i < G1tree_size; i++) {
        printf("%zu -> %zu\n", G1tree[i].value, G1tree[i].parent);
    }
    printf("\n");
    print_tree(G1tree, G1tree_size);

    exit(EXIT_SUCCESS);
}
