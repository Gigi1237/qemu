#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu-common.h"
#include "qemu/log.h"
#include "hw/sysbus.h"

#define TYPE_S3C2416_MEMC "s3c2416-memc"
#define S3C2416_MEMC(obj) OBJECT_CHECK(s3c2416_memc_state, (obj), TYPE_S3C2416_MEMC)

typedef struct {
    SysBusDevice parent_obj;

    MemoryRegion iomem;
    
    uint32_t BANKCFG;
    uint32_t BANKCON1;
    uint32_t BANKCON2;
    uint32_t BANKCON3;
    uint32_t REFRESH;
    uint32_t TIMEOUT;
} s3c2416_memc_state;


static uint64_t s3c2416_memc_read(void *opaque, hwaddr offset,
    unsigned size)
{
    s3c2416_memc_state *s = (s3c2416_memc_state*)opaque;
    uint32_t val;
    
    switch (offset)
    {
    // BANKCFG
    case 0x0:
        val = s->BANKCFG;
        break;

    // BANKCON1
    case 0x4:
        val = s->BANKCON1;
        break;

    // BANKCON2
    case 0x8:
        val = s->BANKCON2;
        break;

    // BANKCON3
    case 0xC:
        val = s->BANKCON3;
        break;

    // REFRESH
    case 0x10:
        val = s->REFRESH;
        break;

    // TIMEOUT
    case 0x14:
        val = s->TIMEOUT;
        break;

        default:
        qemu_log_mask(LOG_GUEST_ERROR, "s3c2416_memc: Attempted write to invalid register! off: 0x%llx\n", offset);
        break;
    }
    
    return val;
};

static void s3c2416_memc_write(void *opaque, hwaddr offset,
    uint64_t val, unsigned size)
{
    s3c2416_memc_state *s = (s3c2416_memc_state*)opaque;
    
    switch (offset)
    {
    // BANKCFG
    case 0x0:
        s->BANKCFG = val;
        break;

    // BANKCON1
    case 0x4:
        s->BANKCON1 = val;
        break;

    // BANKCON2
    case 0x8:
        s->BANKCON2 = val;
        break;

    // BANKCON3
    case 0xC:
        s->BANKCON3 = val;
        break;

    // REFRESH
    case 0x10:
        s->REFRESH = val;
        break;

    // TIMEOUT
    case 0x14:
        s->TIMEOUT = val;
        break;

    default:
        qemu_log_mask(LOG_GUEST_ERROR, "s3c2416_memc: Attempted write to invalid register! off: 0x%llx\n", offset);
        break;
    }
};
 

static const MemoryRegionOps s3c2416_memc_ops = {
    .read = s3c2416_memc_read,
    .write = s3c2416_memc_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};


static void s3c2416_memc_init(Object *obj)
{
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);
    s3c2416_memc_state *s = S3C2416_MEMC(obj);

    memory_region_init_io(&s->iomem, OBJECT(s), &s3c2416_memc_ops, s, "s3c2416_memc", 0x00001000);
    sysbus_init_mmio(sbd, &s->iomem);

    s->BANKCFG = 0x99F0D;
    s->BANKCON1 = 0x8;
    s->BANKCON2 = 0x8;
    s->BANKCON3 = 0x8;
    s->REFRESH = 0x20;
    s->TIMEOUT = 0x0;
};

static const TypeInfo s3c2416_memc_type = {
    .name = TYPE_S3C2416_MEMC,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(s3c2416_memc_state),
    .instance_init = s3c2416_memc_init
};

static void s3c2416_memc_register(void)
{
    type_register_static(&s3c2416_memc_type);
}

type_init(s3c2416_memc_register);
