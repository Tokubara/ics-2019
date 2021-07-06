#include "nemu.h"
#include "device/map.h"

uint8_t pmem[PMEM_SIZE] PG_ALIGN = {};

static IOMap pmem_map = {
  .name = "pmem", // 猜测原因是外设有内存映射, 名字会为外设
  .space = pmem,
  .callback = NULL
};

// x86中base=0
void register_pmem(paddr_t base) {
  pmem_map.low = base;
  pmem_map.high = base + PMEM_SIZE - 1;

  Log("Add '%s' at [0x%08x, 0x%08x]", pmem_map.name, pmem_map.low, pmem_map.high);
}

IOMap* fetch_mmio_map(paddr_t addr);

/* Memory accessing interfaces */

// len只能取{1..4}
uint32_t paddr_read(paddr_t addr, int len) {
  if (map_inside(&pmem_map, addr)) {
    uint32_t offset = addr - pmem_map.low;
    return *(uint32_t *)(pmem + offset) & (~0u >> ((4 - len) << 3));
  }
  else {
    return map_read(addr, len, fetch_mmio_map(addr));
  }
}

void paddr_write(paddr_t addr, uint32_t data, int len) {
  if (map_inside(&pmem_map, addr)) {
    uint32_t offset = addr - pmem_map.low;
    memcpy(pmem + offset, &data, len);
  }
  else {
    return map_write(addr, data, len, fetch_mmio_map(addr));
  }
}

static inline void* get_host_addr_by_addr(paddr_t st_addr, size_t len) {
  void* host_addr;
  if (map_inside(&pmem_map, st_addr) && map_inside(&pmem_map, st_addr+len-1)) {
    host_addr = pmem + (st_addr - pmem_map.low);
  } else {
    host_addr = get_mmio_host_addr_by_addr(st_addr, st_addr+len-1);
  }
  return host_addr;
}

// movsb, 失败返回-1, 这样movsb的执行函数调用rtl_exit
size_t pmem_cpy(paddr_t dest_addr, paddr_t src_addr, size_t len) {
  void* dest = get_host_addr_by_addr(dest_addr, len);
  if (!dest) {
    return 0;
  }
  void* src = get_host_addr_by_addr(src_addr, len);
  if (!src) {
    return 0;
  }
  memcpy(dest, src, len);
  return len;
}

bool is_valid_addr(paddr_t addr) { return (addr >= pmem_map.low && addr <= pmem_map.high); }
