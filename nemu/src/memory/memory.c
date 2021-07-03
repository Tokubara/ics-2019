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

// movsb, 失败返回-1, 这样movsb的执行函数调用rtl_exit
int pmem_cpy(paddr_t dest_addr, paddr_t src_addr, size_t len) {
  if (map_inside(&pmem_map, dest_addr) && map_inside(&pmem_map, dest_addr+len-1) && map_inside(&pmem_map, src_addr) && map_inside(&pmem_map, src_addr+len-1)) {
    uint8_t* dest = pmem + (dest_addr - pmem_map.low);
    uint8_t* src = pmem + (src_addr - pmem_map.low);
    memcpy(dest, src, len);
    return 0;
  } else {
    return -1;
  }
}

bool is_valid_addr(paddr_t addr) { return (addr >= pmem_map.low && addr <= pmem_map.high); }
