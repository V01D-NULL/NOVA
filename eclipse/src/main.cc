#include "hypervisor/hip.h"
#include "mm/buddy.h"

void main([[maybe_unused]] struct Hip *hip) {
  BuddyAllocator::alloc_sz(4096);

  for (;;)
    ;
}