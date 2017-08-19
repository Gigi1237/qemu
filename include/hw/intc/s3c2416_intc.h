#ifndef S3C2416_INTC_H
#define S3C2416_INTC_H

#include "hw/sysbus.h"

#define TYPE_S3C2416_INTC "S3C2416-intc"
#define S3C2416_INTC(obj) OBJECT_CHECK(S3C2416_intc_state, (obj), TYPE_S3C2416_INTC)

typedef struct {
    SysBusDevice parent_obj;

    MemoryRegion iomem;
    uint32_t SRCPND[2];
    uint32_t INTMOD[2];
    uint32_t INTMSK[2];
    uint32_t INTPND[2];
    uint32_t INTOFFSET[2];

    uint32_t SUBSRCPND;
    uint32_t INTSUBMSK;

    qemu_irq irq;
    qemu_irq fiq;
} S3C2416_intc_state;

#endif
