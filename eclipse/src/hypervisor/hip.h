#ifndef HIP_H
#define HIP_H 1

#include <stdint.h>

struct [[gnu::packed]] Hip final {
  uint8_t signature[4]; // 0x41564f4e -> "NOVA"
  uint16_t checksum;
  uint16_t length;
  uint64_t nova_start_address;
  uint64_t nova_end_address;
  uint64_t mbuf_start_address;
  uint64_t mbuf_end_address;
  uint64_t root_pd_start_address;
  uint64_t root_pd_end_address;
  uint64_t rsdp_address;
  uint64_t uefi_memory_map_address;
  uint32_t uefi_memory_map_size;
  uint16_t uefi_desc_size;
  uint16_t uefi_desc_version;
  uint64_t stc_freq;
  uint64_t num_capability_selectors;

  // FIXME (need better names):
  uint16_t sel_hst_arch;
  uint16_t sel_hst_nova;
  uint16_t sel_gst_arch;
  uint16_t sel_gst_nova;
  uint16_t cpu_num;
  uint16_t cpu_bsp;
  uint16_t int_pin;
  uint16_t int_msi;
  uint8_t mco_obj;
  uint8_t mco_hst;
  uint8_t mco_gst;
  uint8_t mco_dma;
  uint8_t mcp_pio;
  uint8_t mco_msr;
  uint16_t ki_max;
};

static_assert(sizeof(Hip) == 0x78);

#endif // HIP_H