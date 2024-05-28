#include "src/mm/buddy.h"
#include <gtest/gtest.h>

class BuddyAllocatorTest : public ::testing::Test {};

TEST_F(BuddyAllocatorTest, AllocateAndFree) {
  auto ptr1 = BuddyAllocator::alloc_sz(32);
  ASSERT_NE(ptr1.address, nullptr) << "Allocation of 32 bytes failed";

  auto ptr2 = BuddyAllocator::alloc_sz(64);
  ASSERT_NE(ptr2.address, nullptr) << "Allocation of 64 bytes failed";

  auto ptr3 = BuddyAllocator::alloc_sz(128);
  ASSERT_NE(ptr3.address, nullptr) << "Allocation of 128 bytes failed";

  BuddyAllocator::free(ptr1);
  BuddyAllocator::free(ptr2);
  BuddyAllocator::free(ptr3);

  auto ptr4 = BuddyAllocator::alloc_sz(32);
  ASSERT_NE(ptr4.address, nullptr) << "Re-allocation of 32 bytes failed";

  BuddyAllocator::free(ptr4);
}

TEST_F(BuddyAllocatorTest, AllocateExactBufferSize) {
  BuddyAllocator::alloc(0);

  auto res = BuddyAllocator::alloc(0);
  ASSERT_EQ(res.address, nullptr)
      << "Allocation from empty allocator should have failed";
}

TEST_F(BuddyAllocatorTest, ExhaustAllocator) {
  std::vector<AllocationResult> pointers;
  while (true) {
    auto ptr = BuddyAllocator::alloc_sz(32);
    if (ptr.address == nullptr)
      break;
    pointers.push_back(ptr);
  }

  ASSERT_EQ(BuddyAllocator::alloc_sz(32).address, nullptr)
      << "Allocator should be exhausted";

  for (auto &ptr : pointers) {
    BuddyAllocator::free(ptr);
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
