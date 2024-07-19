#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>     // For std::remove and std::rename
#include <cstdlib>    // For std::rand and std::srand
#include <ctime>      // For std::time
#include <fcntl.h>    // For open
#include <unistd.h>   // For fsync and close
#include <sys/types.h>
#include <sys/stat.h>
#include <functional>
#include <cstring> // For memcpy
#include <cstdint> // For uint16_t
#include <cassert>  // For assert

using namespace std;

// Define constants
const int BNODE_NODE = 1; // internal nodes without values
const int BNODE_LEAF = 2; // leaf nodes with values
const int HEADER = 4; // Header size
const int BTREE_PAGE_SIZE = 4096;
const int BTREE_MAX_KEY_SIZE = 1000;
const int BTREE_MAX_VAL_SIZE = 3000;

class BNode {
public:
    vector<char> data;

    // Constructor to initialize the data vector with a given size
    BNode(size_t size = BTREE_PAGE_SIZE) : data(size) {}

    // Helper functions to access and manipulate the header
    uint16_t btype() const {
        return *reinterpret_cast<const uint16_t*>(data.data());
    }

    uint16_t nkeys() const {
        return *reinterpret_cast<const uint16_t*>(data.data() + 2);
    }

    void setHeader(uint16_t btype, uint16_t nkeys) {
        *reinterpret_cast<uint16_t*>(data.data()) = btype;
        *reinterpret_cast<uint16_t*>(data.data() + 2) = nkeys;
    }

    // Helper functions to access and manipulate pointers
    uint64_t getPtr(uint16_t idx) const {
        assert(idx < nkeys());
        size_t pos = HEADER + 8 * idx;
        return *reinterpret_cast<const uint64_t*>(data.data() + pos);
    }

    void setPtr(uint16_t idx, uint64_t val) {
        assert(idx < nkeys());
        size_t pos = HEADER + 8 * idx;
        *reinterpret_cast<uint64_t*>(data.data() + pos) = val;
    }

    // Offset helper functions
    uint16_t offsetPos(uint16_t idx) const {
        assert(1 <= idx && idx <= nkeys());
        return HEADER + 8 * nkeys() + 2 * (idx - 1);
    }

    uint16_t getOffset(uint16_t idx) const {
        return *reinterpret_cast<const uint16_t*>(data.data() + offsetPos(idx));
    }

    void setOffset(uint16_t idx, uint16_t offset) {
        *reinterpret_cast<uint16_t*>(data.data() + offsetPos(idx)) = offset;
    }

    uint16_t kvPos(uint16_t idx) const{
        assert(idx < nkeys());
        return HEADER + 8*nkeys() + 2*nkeys() + getOffset(idx);
    }

    // Function to get the key at a given index
    std::vector<char> getKey(uint16_t idx) const {
        assert(idx < nkeys());
        uint16_t pos = kvPos(idx);
        uint16_t klen = *reinterpret_cast<const uint16_t*>(&data[pos]);
        std::vector<char> key(klen);
        std::memcpy(key.data(), &data[pos + 4], klen);
        return key;
    }

    // Function to get the value at a given index
    std::vector<char> getVal(uint16_t idx) const {
        assert(idx < nkeys());
        uint16_t pos = kvPos(idx);
        uint16_t klen = *reinterpret_cast<const uint16_t*>(&data[pos]);
        uint16_t vlen = *reinterpret_cast<const uint16_t*>(&data[pos + 2]);
        std::vector<char> value(vlen);
        std::memcpy(value.data(), &data[pos + 4 + klen], vlen);
        return value;
    }

    // Determine node size
    uint16_t nbytes(){
        return kvPos(nkeys());
    }
};

class BTree {
public:
    uint64_t root;

    // Callbacks for managing on-disk pages
    std::function<BNode(uint64_t)> get; // Dereference a pointer
    std::function<uint64_t(BNode)> create; // Allocate a new page
    std::function<void(uint64_t)> del; // Deallocate a page

    // Constructor
    BTree(uint64_t root,
          std::function<BNode(uint64_t)> get,
          std::function<uint64_t(BNode)> create,
          std::function<void(uint64_t)> del)
        : root(root), get(get), create(create), del(del) {
        int node1max = HEADER + 8 + 2 + 4 + BTREE_MAX_KEY_SIZE + BTREE_MAX_VAL_SIZE;
        assert(node1max <= BTREE_PAGE_SIZE);
    }
};
