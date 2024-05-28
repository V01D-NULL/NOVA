#ifndef BUDDY_H
#define BUDDY_H 1

#include "types/size.h"
#include <stdint.h>

namespace {
static constexpr bool verbose = false;
static constexpr bool safety_checks = false;
} // namespace

// TODO: Implement this stub
extern "C" void memset(void *n, void *z, int zz) {}

enum FillMode : char {
  ZERO = 0,
  NONE = 1 // Don't fill
};

using AddressType = uint64_t *;
using Order = int;

struct AllocationResult {
  Order order;
  int buddy_index;
  AddressType address{nullptr};

  AllocationResult() = default;
  AllocationResult(uint64_t ord, int idx, AddressType addr)
      : order(ord), buddy_index(idx), address(addr) {}
};

class BuddyAllocator {
public:
  BuddyAllocator() = default;

  static AllocationResult alloc_sz(int sz, FillMode fill = FillMode::NONE) {
    Order order = helper.size_to_order(sz);
    if (order == -1) [[unlikely]] {
      return AllocationResult();
    }

    return alloc(order, fill);
  }

  static AllocationResult alloc(Order order, FillMode fill = FillMode::NONE) {
    int idx_begin, idx_end;

    if constexpr (safety_checks) {
      if (order > helper.min_order || order < helper.max_order) {
        // std::cout << "Bad order: " << order << "\n";
        return AllocationResult();
      }
    }

    // Step 1.
    // Calculate the range of nodes we need to check.
    if (order == 0) {
      idx_begin = idx_end = 0;
    } else {
      idx_begin = helper.nodes_at_order(order) - 1;
      idx_end = idx_begin + helper.nodes_at_order(order) - 1;
    }

    // Step 2.
    // Find a free node at this order.
    // Bit position of the node that was allocated (-1 indicates allocation
    // failure)
    int allocated_node_idx = -1;

    // Calculate the number of bit-shifts to get the left-most node of this
    // order
    int shift = (1 << order) - 1;
    for (int x = idx_begin; x <= idx_end; x++) {
      if (helper.test_and_set_bit(shift) == 1) {
        allocated_node_idx = x;
        break;
      }
      shift += 1;
    }

    // Step 3.
    // Allocate child and parent nodes
    if (allocated_node_idx == -1) {
      // std::cout << "Failed to allocate memory at order " << std::dec << order
      //           << '\n';
      return AllocationResult();
    }

    auto const set = [&](int idx) { helper.set_bit(idx); };
    helper.recurse_parent(set, allocated_node_idx);
    helper.recurse_children(set, allocated_node_idx);

    AddressType addr = reinterpret_cast<AddressType>(
        base + (allocated_node_idx * helper.order_to_size(order)));
    return AllocationResult(order, allocated_node_idx, addr);
  }

  // Tries to allocate a node at order 'order'.
  // Returns true if successful, otherwise false.
  static bool scan_order(int order) {
    int begin = 0;
    int end = helper.nodes_at_order(order);
    for (int x = begin; x <= end; x++) {
      if (helper.test_and_set_bit(x))
        return true;
    }

    return false;
  }

  static void free(AllocationResult alloc_result) {
    const auto [order, buddy_index, _] = alloc_result;
    free(order, buddy_index);
  }

  static void free(Order order, int allocated_node_idx) {
    /*
      1. clear this node
      2. If buddy of node is cleared, we can merge (i.e. clear the parent node)
      3. Ascend to parent order and repeat
    */

    auto const coalesce = [&](int idx) {
      bool buddy_node_in_use =
          order == 0 || helper.test_bit(helper.buddy_of(idx));

      if (!buddy_node_in_use) {
        helper.clear_bit(helper.parent_of(idx));
      }
    };

    helper.clear_bit(allocated_node_idx); // Free this node (address).
    helper.recurse_parent(coalesce, allocated_node_idx); // Coalesce
  }

private:
  class Helper {
    long bitmap[8191]{0};

  public:
    Helper() {}

    static constexpr int bitmap_block_size = sizeof(int) * 8;
    static constexpr int page_size = 4096;
    static constexpr int tree_depth = 12;

    // Represents the number of bit-shifts required to get the last bit (aka
    // number of bits in the bitmap) (i.e. the right-most node at the bottom of
    // the tree)
    static constexpr int last_bit = (1 << (tree_depth + 1)) - 1;
    static constexpr int first_bit = 0;

    static constexpr int min_order = tree_depth;
    static constexpr int max_order = 0;

  public:
    // Util functions
    //
    int order_to_size(int order) {
      return page_size * (1 << (min_order - order));
    }

    int size_to_order(int size) {
      int order = min_order;
      int current_size = page_size;

      while (current_size < size && order > max_order) {
        current_size <<= 1;
        --order;
      }

      if (current_size >= size) {
        return order;
      }

      // Requested size too large
      return -1;
    }

    int nodes_at_order(int order) { return 1 << order; }

    bool test_bit(int idx) {
      return (bitmap[idx / bitmap_block_size] &
              (1 << (idx % bitmap_block_size)));
    }

    void set_bit(int idx) {
      bitmap[idx / bitmap_block_size] |= (1 << (idx % bitmap_block_size));
    }

    void clear_bit(int idx) {
      bitmap[idx / bitmap_block_size] &= ~(1 << (idx % bitmap_block_size));
      // if (buddy_of(idx) == 0) {}
    }

    // Set bit if clear
    // Returns True if the bit could be set.
    // If not, it returns False
    bool test_and_set_bit(int idx) {
      if (test_bit(idx) == 0) {
        set_bit(idx);
        return true;
      }

      return false;
    }

    int buddy_of(int idx) { return ((idx - 1) ^ 1) + 1; }

    // Return the parent's bit index of a given bit index
    int parent_of(int idx) { return (idx - 1) / 2; }

    int left_child(int idx) { return (idx * 2) + 1; }

    int right_child(int idx) { return (idx * 2) + 2; }

    //
    // These functions traverse the parent and child nodes.
    // They also perform a callback 'f' which is set_bit() for allocation or
    // clear_bit() for deallocation
    //
    template <typename F> void recurse_parent(F f, int current_idx) {
      f(current_idx);
      if (current_idx == 0)
        return;

      recurse_parent(f, parent_of(current_idx));
    }

    template <typename F> void recurse_children(F f, int current_idx) {
      if (current_idx > this->last_bit)
        return;

      f(current_idx);
      recurse_children(f, left_child(current_idx));
      recurse_children(f, right_child(current_idx));
    }
  };

private:
  // clang-format off
  static inline Helper helper;
  static inline uint8_t static_memory[1 << (Helper::tree_depth + 12)] = {0}; // 1 << (12 + 12) = 16 MB
  static inline uintptr_t base = reinterpret_cast<uintptr_t>(static_memory);
  // clang-format on
};

#endif // BUDDY_H