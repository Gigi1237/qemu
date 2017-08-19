#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu-common.h"
#include "qemu/log.h"
#include "hw/sysbus.h"

#define TYPE_S3C2416_SYSC "s3c2416-sysc"
#define S3C2416_SYSC(obj) OBJECT_CHECK(s3c2416_sysc_state, (obj), TYPE_S3C2416_SYSC)

typedef struct {
    SysBusDevice parent_obj;

    MemoryRegion iomem;

    uint32_t LOCKCON0;
    uint32_t LOCKCON1;
    uint32_t OSCSET;
    uint32_t MPLLCON;
    uint32_t EPLLCON;
    uint32_t EPLLCON_K;
    uint32_t CLKSRC;
    uint32_t CLKDIV0;
    uint32_t CLKDIV1;
    uint32_t CLKDIV2;
    uint32_t HCLKCON;
    uint32_t PCLKCON;
    uint32_t SCLKCON;
    uint32_t PWRMODE;
    uint32_t SWRST;
    uint32_t BUSPRI0;
    uint32_t PWRCFG;
    uint32_t RSTCON;
    uint32_t RSTSTAT;
    uint32_t WKUPSTAT;
    uint32_t INFORM0;
    uint32_t INFORM1;
    uint32_t INFORM2;
    uint32_t INFORM3;
    uint32_t PHYCTRL;
    uint32_t PHYPWR;
    uint32_t URSTCON;
    uint32_t UCLKCON;
} s3c2416_sysc_state;


static uint64_t s3c2416_sysc_read(void *opaque, hwaddr offset,
    unsigned size)
{
    s3c2416_sysc_state *s = (s3c2416_sysc_state*)opaque;
    uint32_t val;

    switch (offset)
    {
        // LOCKCON0
    case 0x0:
        val = s->LOCKCON0;
        break;

        // LOCKCON1
    case 0x4:
        val = s->LOCKCON1;
        break;

        // OSCSET
    case 0x8:
        val = s->OSCSET;
        break;

        // MPLLCON
    case 0x10:
        val = s->MPLLCON;
        break;

        // EPLLCON
    case 0x18:
        val = s->EPLLCON;
        break;

        // EPLLCON_K
    case 0x1C:
        val = s->EPLLCON_K;
        break;

        // CLKSRC
    case 0x20:
        val = s->CLKSRC;
        break;

        // CLKDIV0
    case 0x24:
        val = s->CLKDIV0;
        break;

        // CLKDIV1
    case 0x28:
        val = s->CLKDIV1;
        break;

        // CLKDIV2
    case 0x2C:
        val = s->CLKDIV2;
        break;

        // HCLKCON
    case 0x30:
        val = s->HCLKCON;
        break;

        // PCLKCON
    case 0x34:
        val = s->PCLKCON;
        break;

        // SCLKCON
    case 0x38:
        val = s->SCLKCON;
        break;

        // PWRMODE
    case 0x40:
        val = s->PWRMODE;
        break;

        // SWRST
    case 0x44:
        val = s->SWRST;
        break;

        // BUSPRI0
    case 0x50:
        val = s->BUSPRI0;
        break;

        // PWRCFG
    case 0x60:
        val = s->PWRCFG;
        break;

        // RSTCON
    case 0x64:
        val = s->RSTCON;
        break;

        // RSTSTAT
    case 0x68:
        val = s->RSTSTAT;
        break;

        // WKUPSTAT
    case 0x6C:
        val = s->WKUPSTAT;
        break;

        // INFORM0
    case 0x70:
        val = s->INFORM0;
        break;

        // INFORM1
    case 0x74:
        val = s->INFORM1;
        break;

        // INFORM2
    case 0x78:
        val = s->INFORM2;
        break;

        // INFORM3
    case 0x7C:
        val = s->INFORM3;
        break;

        // PHYCTRL
    case 0x80:
        val = s->PHYCTRL;
        break;

        // PHYPWR
    case 0x84:
        val = s->PHYPWR;
        break;

        // URSTCON
    case 0x88:
        val = s->URSTCON;
        break;

        // UCLKCON
    case 0x8C:
        val = s->UCLKCON;
        break;

    default:
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset %"HWADDR_PRIx"\n",
                      __func__, offset);
        val = 0;
        break;
    }

    return val;
};

static void s3c2416_sysc_write(void *opaque, hwaddr offset,
    uint64_t val, unsigned size)
{
    s3c2416_sysc_state *s = (s3c2416_sysc_state*)opaque;

    switch (offset)
    {
        // LOCKCON0
    case 0x0:
        s->LOCKCON0 = val;
        break;

        // LOCKCON1
    case 0x4:
        s->LOCKCON1 = val;
        break;

        // OSCSET
    case 0x8:
        s->OSCSET = val;
        break;

        // MPLLCON
    case 0x10:
        s->MPLLCON = val;
        break;

        // EPLLCON
    case 0x18:
        s->EPLLCON = val;
        break;

        // EPLLCON_K
    case 0x1C:
        s->EPLLCON_K = val;
        break;

        // CLKSRC
    case 0x20:
        s->CLKSRC = val;
        break;

        // CLKDIV0
    case 0x24:
        s->CLKDIV0 = val;
        break;

        // CLKDIV1
    case 0x28:
        s->CLKDIV1 = val;
        break;

        // CLKDIV2
    case 0x2C:
        s->CLKDIV2 = val;
        break;

        // HCLKCON
    case 0x30:
        s->HCLKCON = val;
        break;

        // PCLKCON
    case 0x34:
        s->PCLKCON = val;
        break;

        // SCLKCON
    case 0x38:
        s->SCLKCON = val;
        break;

        // PWRMODE
    case 0x40:
        s->PWRMODE = val;
        break;

        // SWRST
    case 0x44:
        s->SWRST = val;
        break;

        // BUSPRI0
    case 0x50:
        s->BUSPRI0 = val;
        break;

        // PWRCFG
    case 0x60:
        s->PWRCFG = val;
        break;

        // RSTCON
    case 0x64:
        s->RSTCON = val;
        break;

        // WKUPSTAT
    case 0x6C:
        s->WKUPSTAT = val;
        break;

        // INFORM0
    case 0x70:
        s->INFORM0 = val;
        break;

        // INFORM1
    case 0x74:
        s->INFORM1 = val;
        break;

        // INFORM2
    case 0x78:
        s->INFORM2 = val;
        break;

        // INFORM3
    case 0x7C:
        s->INFORM3 = val;
        break;

        // PHYCTRL
    case 0x80:
        s->PHYCTRL = val;
        break;

        // PHYPWR
    case 0x84:
        s->PHYPWR = val;
        break;

        // URSTCON
    case 0x88:
        s->URSTCON = val;
        break;

        // UCLKCON
    case 0x8C:
        s->UCLKCON = val;
        break;

    default:
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset %"HWADDR_PRIx"\n",
                      __func__, offset);
        break;
    }
};


static const MemoryRegionOps s3c2416_sysc_ops = {
    .read = s3c2416_sysc_read,
    .write = s3c2416_sysc_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};


static void s3c2416_sysc_init(Object *obj)
{
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);
    s3c2416_sysc_state *s = S3C2416_SYSC(obj);

    memory_region_init_io(&s->iomem, OBJECT(s), &s3c2416_sysc_ops, s, "s3c2416_sysc", 0x00001000);
    sysbus_init_mmio(sbd, &s->iomem);

    s->LOCKCON0 = 0xFFFF;
    s->LOCKCON1 = 0xFFFF;
    s->OSCSET = 0x8000;
    s->MPLLCON = 0x18540C0;
    s->EPLLCON = 0x1200102;
    s->EPLLCON_K = 0x0;
    s->CLKSRC = 0x0;
    s->CLKDIV0 = 0xC;
    s->CLKDIV1 = 0x0;
    s->CLKDIV2 = 0x0;
    s->HCLKCON = 0xFFFFFFFF;
    s->PCLKCON = 0xFFFFFFFF;
    s->SCLKCON = 0xFFFFDFFF;
    s->PWRMODE = 0x0;
    s->SWRST = 0x0;
    s->BUSPRI0 = 0x0;
    s->PWRCFG = 0x0;
    s->RSTCON = 0x60101;
    s->RSTSTAT = 0x1;
    s->WKUPSTAT = 0x0;
    s->INFORM0 = 0x0;
    s->INFORM1 = 0x0;
    s->INFORM2 = 0x0;
    s->INFORM3 = 0x0;
    s->PHYCTRL = 0x0;
    s->PHYPWR = 0x0;
    s->URSTCON = 0x0;
    s->UCLKCON = 0x0;
};

static const TypeInfo s3c2416_sysc_type = {
    .name = TYPE_S3C2416_SYSC,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(s3c2416_sysc_state),
    .instance_init = s3c2416_sysc_init
};

static void s3c2416_sysc_register(void)
{
    type_register_static(&s3c2416_sysc_type);
}

type_init(s3c2416_sysc_register);
