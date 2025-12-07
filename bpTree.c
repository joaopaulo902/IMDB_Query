//
// Created by joaop on 07/12/2025.
// Fixed and improved version
//

#include "bpTree.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <limits.h> // for LLONG_MIN
#include "sysManager.h"
#include <errno.h>

// helper compare: return -1,0,1
static int key_cmp(const Key *a, const Key *b) {
    if (a->value < b->value) return -1;
    if (a->value > b->value) return  1;
    if (a->id < b->id) return -1;
    if (a->id > b->id) return  1;
    return 0;
}

enum { HEADER_SIZE = sizeof(NodeHeader) };

// compute capacity based on PAGE_SIZE and sizes.
// For internal node we will store child_count = n+1 int64 + n keys (key: 4+8)
static const size_t KEY_SIZE = sizeof(int32_t) + sizeof(int64_t); // 12
static const size_t PTR_SIZE = sizeof(int64_t);

// compute maximums
static size_t max_leaf_entries(void) {
    size_t avail = NODE_PAGE_SIZE - HEADER_SIZE;
    size_t entry_size = KEY_SIZE + sizeof(int64_t); // key + value(offset)
    return avail / entry_size;
}

static size_t max_internal_keys(void) {
    size_t avail = NODE_PAGE_SIZE - HEADER_SIZE;
    if (avail <= PTR_SIZE)
        return 0;
    return (avail - PTR_SIZE) / (KEY_SIZE + PTR_SIZE);
}

// write header helper
static void write_bpheader(BPTree *T) {
    assert(T && T->f);
    if (fseeko(T->f, 0, SEEK_SET) != 0) { perror("fseeko write header"); exit(1); }
    if (fwrite(&T->hdr, sizeof(BPHeader), 1, T->f) != 1) { perror("fwrite header"); exit(1); }
    fflush(T->f);
}

// --- low-level page IO ----------------

static void write_page(BPTree *T, int64_t page_offset, const uint8_t *buf) {
    assert(T && T->f);
    if (fseeko(T->f, page_offset, SEEK_SET) != 0) {
        perror("fseeko write_page");
        exit(1);
    }
    if (fwrite(buf, 1, NODE_PAGE_SIZE, T->f) != NODE_PAGE_SIZE) {
        perror("fwrite write_page");
        exit(1);
    }
    fflush(T->f);
}

static void read_page(BPTree *T, int64_t page_offset, uint8_t *buf) {
    assert(T && T->f);
    if (fseeko(T->f, page_offset, SEEK_SET) != 0) {
        perror("fseeko read_page");
        exit(1);
    }
    size_t got = fread(buf, 1, NODE_PAGE_SIZE, T->f);
    if (got != NODE_PAGE_SIZE) {
        if (ferror(T->f)) {
            perror("fread read_page");
            exit(1);
        }
        // If file is shorter, zero the rest (new pages)
        if (got < NODE_PAGE_SIZE) memset(buf + got, 0, NODE_PAGE_SIZE - got);
    }
}

// allocate new page (append) -> returns page_offset
static int64_t alloc_page(BPTree *T) {
    assert(T && T->f);
    if (fseeko(T->f, 0, SEEK_END) != 0) { perror("fseeko alloc"); exit(1); }
    int64_t end = ftello(T->f);
    uint8_t *buf = calloc(1, NODE_PAGE_SIZE);
    if (!buf) { perror("calloc"); exit(1); }
    if (fwrite(buf, 1, NODE_PAGE_SIZE, T->f) != NODE_PAGE_SIZE) { perror("fwrite alloc"); free(buf); exit(1); }
    fflush(T->f);
    free(buf);

    // update header node count and write header immediately
    T->hdr.node_count++;
    write_bpheader(T);

    return end; // offset of new page
}

// serialize Node -> page buffer
static void node_serialize(BPTree *T, Node *node, uint8_t *pagebuf) {
    assert(node != NULL);
    assert(pagebuf != NULL);

    memset(pagebuf, 0, NODE_PAGE_SIZE);
    memcpy(pagebuf, &node->hdr, sizeof(node->hdr));
    uint8_t *p = pagebuf + HEADER_SIZE;
    if (node->hdr.is_leaf) {
        // leaf: for i in 0..n-1: key(year,id) then value (int64 offset)
        for (int i=0;i<node->hdr.n;i++) {
            memcpy(p, &node->keys[i].value, sizeof(int32_t)); p += sizeof(int32_t);
            memcpy(p, &node->keys[i].id, sizeof(int64_t)); p += sizeof(int64_t);
            memcpy(p, &node->ptrs[i], sizeof(int64_t)); p += sizeof(int64_t);
        }
    } else {
        // internal: child_ptr0, then for i: key_i, child_ptr[i+1]
        // write child ptr 0
        memcpy(p, &node->ptrs[0], PTR_SIZE); p += PTR_SIZE;
        for (int i=0;i<node->hdr.n;i++) {
            memcpy(p, &node->keys[i].value, sizeof(int32_t)); p += sizeof(int32_t);
            memcpy(p, &node->keys[i].id, sizeof(int64_t)); p += sizeof(int64_t);
            memcpy(p, &node->ptrs[i+1], PTR_SIZE); p += PTR_SIZE;
        }
    }
}

// deserialize page buffer -> node (assumes node->keys and ptrs allocated for capacity)
static void node_deserialize(BPTree *T, Node *node, const uint8_t *pagebuf) {
    NodeHeader hdr;
    memcpy(&hdr, pagebuf, sizeof(NodeHeader));
    node->hdr = hdr;
    const uint8_t *p = pagebuf + HEADER_SIZE;
    if (node->hdr.is_leaf) {
        for (int i=0;i<node->hdr.n;i++) {
            memcpy(&node->keys[i].value, p, sizeof(int32_t)); p += sizeof(int32_t);
            memcpy(&node->keys[i].id, p, sizeof(int64_t)); p += sizeof(int64_t);
            memcpy(&node->ptrs[i], p, sizeof(int64_t)); p += sizeof(int64_t);
        }
    }
    else {
        memcpy(&node->ptrs[0], p, PTR_SIZE); p += PTR_SIZE;
        for (int i=0;i<node->hdr.n;i++) {
            memcpy(&node->keys[i].value, p, sizeof(int32_t)); p += sizeof(int32_t);
            memcpy(&node->keys[i].id, p, sizeof(int64_t)); p += sizeof(int64_t);
            memcpy(&node->ptrs[i+1], p, PTR_SIZE); p += PTR_SIZE;
        }
    }
}

// allocate in-memory Node with capacity
static Node *node_create(BPTree *T, int is_leaf) {
    Node *n = calloc(1, sizeof(Node));
    if (!n) { perror("calloc node"); exit(1); }
    n->hdr.is_leaf = is_leaf ? 1 : 0;
    n->hdr.n = 0;
    n->hdr.parent = (int64_t)-1;
    n->hdr.next = (int64_t)-1;

    size_t key_cap = is_leaf ? (T->leaf_capacity + 2) : (T->internal_capacity + 2);
    size_t ptr_cap = is_leaf ? (T->leaf_capacity + 2) : (T->internal_capacity + 3);
    n->keys = calloc(key_cap, sizeof(Key));
    n->ptrs = calloc(ptr_cap, sizeof(int64_t));
    if (!n->keys || !n->ptrs) { perror("calloc node arrays"); exit(1); }
    return n;
}

static void node_free(Node *n) {
    if (!n) return;
    free(n->keys);
    free(n->ptrs);
    free(n);
}

static Node *read_node(BPTree *T, int64_t page_offset) {
    uint8_t *buf = malloc(NODE_PAGE_SIZE);
    if (!buf) {
        perror("malloc read buf");
        exit(1);
    }

    read_page(T, page_offset, buf);
    NodeHeader hdr;
    memcpy(&hdr, buf, sizeof(NodeHeader));
    Node *n = node_create(T, hdr.is_leaf ? 1 : 0);
    node_deserialize(T, n, buf);
    free(buf);
    return n;
}

// write Node to disk at page_offset
static void write_node(BPTree *T, int64_t page_offset, Node *n) {
    uint8_t *buf = calloc(1, NODE_PAGE_SIZE);
    if (!buf) { perror("calloc write buf"); exit(1); }
    node_serialize(T, n, buf);
    write_page(T, page_offset, buf);
    free(buf);
}

// --- B+ tree operations ----------------

// initialize tree (open file, set capacities)
BPTree *bpt_open(const char *path) {
    BPTree *T = calloc(1, sizeof(BPTree));
    if (!T) {
        perror("calloc BPTree");
        exit(1);
    }

    // always compute capacities
    T->leaf_capacity     = max_leaf_entries();
    T->internal_capacity = max_internal_keys();

    // try open existing
    T->f = fopen(path, "r+b");
    if (T->f) {
        if (fseek(T->f, 0, SEEK_SET) != 0) {
            perror("fseek header read");
            exit(1);
        }
        if (fread(&T->hdr, sizeof(BPHeader), 1, T->f) != 1) {
            perror("fread header");
            exit(1);
        }

        if (T->hdr.page_size != NODE_PAGE_SIZE) {
            fprintf(stderr, "bpt_open: page_size mismatch (file=%u, expected=%d)\n", T->hdr.page_size, NODE_PAGE_SIZE);
            fclose(T->f);
            free(T);
            return NULL;
        }
        T->root_offset = (int64_t) T->hdr.root_offset;
        return T;
    }

    // create new
    T->f = fopen(path, "w+b");
    if (!T->f) { perror("fopen create"); exit(1); }

    memset(&T->hdr, 0, sizeof(BPHeader));
    T->hdr.page_size   = NODE_PAGE_SIZE;
    T->hdr.root_offset = sizeof(BPHeader);
    T->hdr.node_count  = 1;

    // write header
    fseek(T->f, 0, SEEK_SET);
    if (fwrite(&T->hdr, sizeof(BPHeader), 1, T->f) != 1) { perror("fwrite header create"); exit(1); }
    fflush(T->f);

    // create empty root page
    uint8_t *buf = calloc(1, NODE_PAGE_SIZE);
    if (!buf) { perror("calloc rootbuf"); exit(1); }
    NodeHeader root_hdr = { .is_leaf = 1, .n = 0, .parent = -1, .next = -1 };
    memcpy(buf, &root_hdr, sizeof(root_hdr));

    fseek(T->f, T->hdr.root_offset, SEEK_SET);
    if (fwrite(buf, 1, NODE_PAGE_SIZE, T->f) != NODE_PAGE_SIZE) { perror("fwrite root"); exit(1); }
    fflush(T->f);

    free(buf);

    T->root_offset = T->hdr.root_offset;
    return T;
}

void bpt_close(BPTree *T) {
    if (!T) return;
    if (T->f) {
        // ensure header saved
        write_bpheader(T);
        fclose(T->f);
    }
    free(T);
}

// helper: load node at page i into Node * (page_offset)
static Node *load_node_at(BPTree *T, int64_t page_offset) {
    // read page buf
    uint8_t *buf = malloc(NODE_PAGE_SIZE);
    if (!buf) { perror("malloc load buf"); exit(1); }
    read_page(T, page_offset, buf);
    NodeHeader hdr;
    memcpy(&hdr, buf, sizeof(NodeHeader));
    Node *n = node_create(T, hdr.is_leaf ? 1 : 0);
    node_deserialize(T, n, buf);
    free(buf);
    return n;
}

// find leaf for given key; returns leaf page_offset and Node* (caller must free node)
static int64_t find_leaf_page(BPTree *T, const Key *k, Node **leaf_out) {
    if (!T || !T->f || !k || !leaf_out) return -1;
    *leaf_out = NULL;

    int64_t cur_off = T->root_offset;
    if (cur_off < 0) {
        fprintf(stderr, "find_leaf_page: invalid root offset: %lld\n", (long long)cur_off);
        return -1;
    }

    const int MAX_ITER = 2000000;  // safety: prevents infinite loops if corrupt
    int steps = 0;

    while (1) {
        if (++steps > MAX_ITER) {
            fprintf(stderr, "find_leaf_page: infinite loop detected — corrupt tree\n");
            return -1;
        }

        // ----------------------------
        // 1) Read page from disk
        // ----------------------------
        uint8_t pagebuf[NODE_PAGE_SIZE];
        read_page(T, cur_off, pagebuf);

        // ----------------------------
        // 2) Read header
        // ----------------------------
        NodeHeader hdr;
        memcpy(&hdr, pagebuf, sizeof(NodeHeader));

        // ----------------------------
        // 3) Validate node size (corruption check)
        // ----------------------------
        if (hdr.is_leaf) {
            if (hdr.n > T->leaf_capacity) {
                fprintf(stderr,
                        "find_leaf_page: corrupt leaf node at %lld: n=%u > leaf_capacity=%zu\n",
                        (long long)cur_off, hdr.n, T->leaf_capacity);
                return -1;
            }
        } else {
            if (hdr.n > T->internal_capacity) {
                fprintf(stderr,
                        "find_leaf_page: corrupt internal node at %lld: n=%u > internal_capacity=%zu\n",
                        (long long)cur_off, hdr.n, T->internal_capacity);
                return -1;
            }
        }

        // ----------------------------
        // 4) Deserialize node
        // ----------------------------
        Node *node = node_create(T, hdr.is_leaf ? 1 : 0);
        memcpy(&node->hdr, &hdr, sizeof(NodeHeader));
        node_deserialize(T, node, pagebuf);

        // ----------------------------
        // 5) Return leaf node
        // ----------------------------
        if (node->hdr.is_leaf) {
            *leaf_out = node;
            return cur_off;
        }

        // ----------------------------
        // 6) Internal node — choose child pointer
        // ----------------------------
        int i = 0;
        while (i < node->hdr.n && key_cmp(&node->keys[i], k) < 0)
            i++;

        int64_t child_off = node->ptrs[i];

        // ----------------------------
        // 7) Safety validation
        // ----------------------------
        if (child_off < 0) {
            fprintf(stderr,
                    "find_leaf_page: invalid child ptr %lld at node %lld\n",
                    (long long)child_off, (long long)cur_off);
            node_free(node);
            return -1;
        }

        if (child_off == cur_off) {
            fprintf(stderr,
                    "find_leaf_page: child pointer loops to itself (%lld) — corrupt\n",
                    (long long)cur_off);
            node_free(node);
            return -1;
        }

        // ----------------------------
        // 8) Descend to next page
        // ----------------------------
        node_free(node);
        cur_off = child_off;
    }
}

// insert into leaf without split; assumes leaf Node loaded and page_offset given
static void leaf_insert_nonfull(BPTree *T, Node *leaf, int64_t leaf_offset, const Key *k, int64_t value_offset) {
    // insert sorted by key
    int i = leaf->hdr.n - 1;
    while (i >= 0 && key_cmp(&leaf->keys[i], k) > 0) {
        leaf->keys[i+1] = leaf->keys[i];
        leaf->ptrs[i+1] = leaf->ptrs[i];
        i--;
    }
    leaf->keys[i+1] = *k;
    leaf->ptrs[i+1] = value_offset;
    leaf->hdr.n++;
    // write page
    write_node(T, leaf_offset, leaf);
}

// split leaf: leaf is full, create new leaf, move half entries to new leaf,
// update parent (caller should handle root split)
static int64_t leaf_split(BPTree *T, Node *leaf, int64_t leaf_offset, const Key *k, int64_t value_offset, Key *promoted) {
    // create new leaf page
    int64_t new_off = alloc_page(T);
    Node *new_leaf = node_create(T, 1);
    new_leaf->hdr.is_leaf = 1;
    new_leaf->hdr.parent = leaf->hdr.parent;
    new_leaf->hdr.next = leaf->hdr.next;
    leaf->hdr.next = new_off;

    // temp arrays to merge existing + new key then split
    int total = leaf->hdr.n + 1;
    Key *tmp_keys = malloc(sizeof(Key) * total);
    int64_t *tmp_vals = malloc(sizeof(int64_t) * total);
    if (!tmp_keys || !tmp_vals) { perror("malloc tmp in leaf_split"); exit(1); }

    int idx = 0, i = 0;
    // merge
    int inserted = 0;
    for (i = 0; i < leaf->hdr.n; i++) {
        if (!inserted && key_cmp(&leaf->keys[i], k) > 0) {
            tmp_keys[idx] = *k;
            tmp_vals[idx] = value_offset;
            idx++; inserted = 1;
        }
        tmp_keys[idx] = leaf->keys[i];
        tmp_vals[idx] = leaf->ptrs[i];
        idx++;
    }
    if (!inserted) { tmp_keys[idx] = *k; tmp_vals[idx] = value_offset; idx++; }
    assert(idx == total);

    // split point
    int cut = total/2;
    // fill left (leaf)
    leaf->hdr.n = 0;
    for (i = 0; i < cut; i++) {
        leaf->keys[i] = tmp_keys[i];
        leaf->ptrs[i] = tmp_vals[i];
        leaf->hdr.n++;
    }
    // fill right (new_leaf)
    new_leaf->hdr.n = 0;
    for (i = cut; i < total; i++) {
        new_leaf->keys[new_leaf->hdr.n] = tmp_keys[i];
        new_leaf->ptrs[new_leaf->hdr.n] = tmp_vals[i];
        new_leaf->hdr.n++;
    }

    // promoted key is first key of new_leaf
    *promoted = new_leaf->keys[0];

    // write both pages
    write_node(T, leaf_offset, leaf);
    write_node(T, new_off, new_leaf);

    // cleanup
    free(tmp_keys); free(tmp_vals);
    node_free(new_leaf);
    return new_off;
}

// insert key/value into tree (simplified, handles splits up to root)
void bpt_insert(BPTree *T, int32_t value, int64_t id, int64_t value_offset) {
    Key k = { .value = value, .id = id };

    // find leaf
    Node *leaf = NULL;
    int64_t leaf_off = find_leaf_page(T, &k, &leaf);
    if (leaf_off < 0 || leaf == NULL) {
        fprintf(stderr, "bpt_insert: find_leaf_page failed\n");
        if (leaf) node_free(leaf);
        return;
    }

    // if empty tree: leaf may be zeroed page (hdr.n==0). ensure is_leaf set
    if (leaf->hdr.n == 0 && leaf->hdr.is_leaf == 0) {
        // convert to leaf in-memory and persist below
        leaf->hdr.is_leaf = 1;
        leaf->hdr.parent = -1;
        leaf->hdr.next = -1;
        write_node(T, leaf_off, leaf);
    }

    // if key already present, ignore (or update value)
    for (int i=0;i<leaf->hdr.n;i++) {
        if (key_cmp(&leaf->keys[i], &k) == 0) {
            leaf->ptrs[i] = value_offset; // update
            write_node(T, leaf_off, leaf);
            node_free(leaf);
            return;
        }
    }

    // capacity
    if ((size_t)leaf->hdr.n < T->leaf_capacity) {
        leaf_insert_nonfull(T, leaf, leaf_off, &k, value_offset);
        node_free(leaf);
        return;
    }

    // need to split leaf
    Key promoted;
    int64_t new_leaf_off = leaf_split(T, leaf, leaf_off, &k, value_offset, &promoted);

    // now push promoted key up to parent; if parent == -1, create new root
    int64_t parent_off = leaf->hdr.parent;
    if (parent_off == -1) {
        // create new root page
        int64_t root_off = alloc_page(T);
        Node *root = node_create(T, 0);
        root->hdr.is_leaf = 0;
        root->hdr.n = 1;
        root->hdr.parent = -1;
        root->ptrs[0] = leaf_off;
        root->keys[0] = promoted;
        root->ptrs[1] = new_leaf_off;

        // update child parent pointers
        leaf->hdr.parent = root_off;
        Node *newleaf = read_node(T, new_leaf_off);
        newleaf->hdr.parent = root_off;

        // write nodes
        write_node(T, leaf_off, leaf);
        write_node(T, new_leaf_off, newleaf);
        write_node(T, root_off, root);

        // update tree root in-memory and on-disk header
        T->root_offset = root_off;
        T->hdr.root_offset = root_off;
        write_bpheader(T);

        node_free(root);
        node_free(newleaf);
    } else {
        // push up into parent: handle internal splits iteratively
        int64_t cur_off = parent_off;
        Key cur_promoted = promoted;
        int64_t cur_new_off = new_leaf_off;
        while (1) {
            Node *parent = read_node(T, cur_off);
            // find position to insert cur_promoted
            int pos = 0;
            while (pos < parent->hdr.n && key_cmp(&parent->keys[pos], &cur_promoted) <= 0) pos++;
            // shift keys/ptrs
            for (int j = parent->hdr.n; j > pos; j--) {
                parent->keys[j] = parent->keys[j-1];
            }
            for (int j = parent->hdr.n+1; j > pos+1; j--) {
                parent->ptrs[j] = parent->ptrs[j-1];
            }
            parent->keys[pos] = cur_promoted;
            parent->ptrs[pos+1] = cur_new_off;
            parent->hdr.n++;

            if ((size_t)parent->hdr.n <= T->internal_capacity) {
                // fits; write and done
                write_node(T, cur_off, parent);
                node_free(parent);
                break;
            } else {
                // split internal node
                int total = parent->hdr.n;
                int cut = total/2;
                int64_t new_off = alloc_page(T);
                Node *new_internal = node_create(T, 0);
                new_internal->hdr.is_leaf = 0;
                new_internal->hdr.parent = parent->hdr.parent;

                // Move keys/ptrs from cut+1 .. total-1 into new_internal
                int new_idx = 0;
                // new_internal->ptrs[0] = parent->ptrs[cut+1];
                new_internal->ptrs[0] = parent->ptrs[cut+1];

                // For all moved child pointers, update their parent fields
                // Move keys and ptrs
                for (int j = cut+1; j < total; j++) {
                    new_internal->keys[new_idx] = parent->keys[j];
                    new_internal->ptrs[new_idx+1] = parent->ptrs[j+1];

                    // update child's parent
                    int64_t child_off = new_internal->ptrs[new_idx+1];
                    if (child_off >= 0) {
                        Node *child = read_node(T, child_off);
                        child->hdr.parent = new_off;
                        write_node(T, child_off, child);
                        node_free(child);
                    }

                    new_idx++;
                }
                new_internal->hdr.n = new_idx;

                // The key to promote is parent->keys[cut]
                Key promote_key = parent->keys[cut];

                // shrink parent: keep ptrs[0..cut], keys[0..cut-1]
                // parent->ptrs[0..cut] remain; parent->hdr.n = cut
                parent->hdr.n = cut;

                // ensure child's parent pointers for remaining parent children are correct
                for (int j = 0; j <= parent->hdr.n; j++) {
                    int64_t child_off = parent->ptrs[j];
                    if (child_off >= 0) {
                        Node *child = read_node(T, child_off);
                        child->hdr.parent = cur_off;
                        write_node(T, child_off, child);
                        node_free(child);
                    }
                }

                // write nodes
                write_node(T, cur_off, parent);
                write_node(T, new_off, new_internal);

                // climb up
                cur_promoted = promote_key;
                cur_new_off = new_off;
                int64_t up = parent->hdr.parent;

                node_free(new_internal);
                node_free(parent);

                if (up == -1) {
                    // create new root
                    int64_t new_root_off = alloc_page(T);
                    Node *root = node_create(T, 0);
                    root->hdr.is_leaf = 0;
                    root->hdr.n = 1;
                    root->hdr.parent = -1;
                    root->ptrs[0] = cur_off;
                    root->keys[0] = cur_promoted;
                    root->ptrs[1] = cur_new_off;

                    // update children parent pointers
                    Node *left_child = read_node(T, cur_off);
                    left_child->hdr.parent = new_root_off;
                    write_node(T, cur_off, left_child);
                    node_free(left_child);

                    Node *right_child = read_node(T, cur_new_off);
                    right_child->hdr.parent = new_root_off;
                    write_node(T, cur_new_off, right_child);
                    node_free(right_child);

                    write_node(T, new_root_off, root);

                    T->root_offset = new_root_off;
                    T->hdr.root_offset = new_root_off;
                    write_bpheader(T);

                    node_free(root);
                    break;
                } else {
                    cur_off = up;
                    continue;
                }
            }
        } // climb while
    }

    node_free(leaf);
}

#define MAX_LEAF_RESULTS 10

#define RANGE_LIMIT 10

// Returns up to 10 ptr offsets in array.
// *count_out = number of values returned.
// *leaf_off_io = current leaf page offset for continuing the query.
// *pos_io = current index inside that leaf page.
int64_t* bpt_range_query_page(BPTree *T, int32_t ylo, int32_t yhi, int *count_out, int64_t *leaf_off_io, int *pos_io) {
    *count_out = 0;

    // output array
    int64_t *results = malloc(RANGE_LIMIT * sizeof(int64_t));
    if (!results) return NULL;

    Node *leaf = NULL;

    //----------------------------------------------------
    // 1. First call: find starting leaf
    //----------------------------------------------------
    if (*leaf_off_io == -1) {
        Key k_lo = { .value = ylo, .id = LLONG_MIN };
        *leaf_off_io = find_leaf_page(T, &k_lo, &leaf);
        *pos_io = 0;

        if (*leaf_off_io < 0 || leaf == NULL) {
            if (leaf)
                node_free(leaf);
            return results;
        }
    } else {
        //------------------------------------------------
        // 2. Subsequent call: continue from saved leaf
        //------------------------------------------------
        leaf = read_node(T, *leaf_off_io);
        if (!leaf) return results;
    }

    //----------------------------------------------------
    // 3. Scan leaves and gather at most 10 results
    //----------------------------------------------------
    while (*count_out < RANGE_LIMIT && leaf) {

        for (int i = *pos_io; i < leaf->hdr.n; i++) {

            int year = leaf->keys[i].value;

            if (year < ylo) continue;
            if (year > yhi) {
                // end of range
                node_free(leaf);
                *leaf_off_io = -1;
                return results;
            }

            // store record offset
            results[*count_out] = leaf->ptrs[i];
            (*count_out)++;

            // update internal leaf index
            *pos_io = i + 1;

            if (*count_out == RANGE_LIMIT) {
                node_free(leaf);
                return results;   // page full
            }
        }

        //------------------------------------------------
        // Reached end of this leaf → move to next
        //------------------------------------------------
        if (leaf->hdr.next == -1) {
            node_free(leaf);
            *leaf_off_io = -1; // no more leaves
            return results;
        }

        *leaf_off_io = leaf->hdr.next;
        node_free(leaf);
        leaf = read_node(T, *leaf_off_io);
        *pos_io = 0;
    }

    if (leaf) node_free(leaf);
    return results;
}


int print_value_cb(int64_t val, void *ctx) {
    return val;
}

int BP_tree_test(void) {
    // open index (creates if missing)
    BPTree *T = bpt_open(RATING_INDEX_FILE);
    if (!T) {
        fprintf(stderr, "BP_tree_test: failed to open index\n");
        return -1;
    }
    printf("PAGE_SIZE=%d; leaf_capacity=%zu internal_capacity=%zu\n", NODE_PAGE_SIZE, T->leaf_capacity, T->internal_capacity);

    int count;
    int64_t currentLeaf = -1;
    int currentIdx = 0;
    int64_t* returned_ids = bpt_range_query_page(T, 800, 1000, &count, &currentLeaf, &currentIdx);
    for (int i = 0; i < count; i++) {
        printf("%lld\n", returned_ids[i]);
    }
    free(returned_ids);

    bpt_close(T);
    return 0;
}
