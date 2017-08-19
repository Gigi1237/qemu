#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu-common.h"
#include "qemu/log.h"
#include "hw/sysbus.h"

#define TYPE_S3C2416_WTCON "s3c2416-wtcon"
#define S3C2416_WTCON(obj) OBJECT_CHECK(s3c2416_wtcon_state, (obj), TYPE_S3C2416_WTCON)

typedef struct {
    SysBusDevice parent_obj;

    MemoryRegion iomem;
    
    uint32_t WTCON;
    uint32_t WTDAT;
    uint32_t WTCNT;
} s3c2416_wtcon_state;


static uint64_t s3c2416_wtcon_read(void *opaque, hwaddr offset,
    unsigned size)
{
    s3c2416_wtcon_state *s = (s3c2416_wtcon_state*)opaque;
    uint32_t val;
    
    switch (offset)
    {
    // WTCON
    case 0x0:
        val = s->WTCON;
        break;

    // WTDAT
    case 0x4:
        val = s->WTDAT;
        break;

    // WTCNT
    case 0x8:
        val = s->WTCNT;
        break;

    default:
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset %"HWADDR_PRIx"\n",
                      __func__, offset);
        val = 0;
        break;
    }
    
    return val;
};

static void s3c2416_wtcon_write(void *opaque, hwaddr offset,
    uint64_t val, unsigned size)
{
    s3c2416_wtcon_state *s = (s3c2416_wtcon_state*)opaque;
    
    switch (offset)
    {
    // WTCON
    case 0x0:
        s->WTCON = val;
        break;

    // WTDAT
    case 0x4:
        s->WTDAT = val;
        break;

    // WTCNT
    case 0x8:
        s->WTCNT = val;
        break;

    default:
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset %"HWADDR_PRIx"\n",
                      __func__, offset);
        break;
    }
};
 

static const MemoryRegionOps s3c2416_wtcon_ops = {
    .read = s3c2416_wtcon_read,
    .write = s3c2416_wtcon_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};


static void s3c2416_wtcon_init(Object *obj)
{
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);
    s3c2416_wtcon_state *s = S3C2416_WTCON(obj);

    memory_region_init_io(&s->iomem, OBJECT(s), &s3c2416_wtcon_ops, s, "s3c2416_wtcon", 0x00001000);
    sysbus_init_mmio(sbd, &s->iomem);

    s->WTCON = 0x8021;
    s->WTDAT = 0x8000;
    s->WTCNT = 0x8000;
};

static const TypeInfo s3c2416_wtcon_type = {
    .name = TYPE_S3C2416_WTCON,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(s3c2416_wtcon_state),
    .instance_init = s3c2416_wtcon_init
};

static void s3c2416_wtcon_register(void)
{
    type_register_static(&s3c2416_wtcon_type);
}

type_init(s3c2416_wtcon_register);
