#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu-common.h"
#include "qemu/log.h"
#include "hw/sysbus.h"

#include "ui/console.h"
#include "ui/input.h"


#define TYPE_S3C2416_GPIO "s3c2416-gpio"
#define S3C2416_GPIO(obj) OBJECT_CHECK(s3c2416_gpio_state, (obj), TYPE_S3C2416_GPIO)

typedef struct {
    SysBusDevice parent_obj;

    MemoryRegion iomem;

    uint32_t GPACON;
    uint32_t GPADAT;
    uint32_t GPBCON;
    uint32_t GPBDAT;
    uint32_t GPBUDP;
    uint32_t GPBSEL;
    uint32_t GPCCON;
    uint32_t GPCDAT;
    uint32_t GPCUDP;
    uint32_t GPDCON;
    uint32_t GPDDAT;
    uint32_t GPDUDP;
    uint32_t GPECON;
    uint32_t GPEDAT;
    uint32_t GPEUDP;
    uint32_t GPESEL;
    uint32_t GPFCON;
    uint32_t GPFDAT;
    uint32_t GPFUDP;
    uint32_t GPGCON;
    uint32_t GPGDAT;
    uint32_t GPGUDP;
    uint32_t GPHCON;
    uint32_t GPHDAT;
    uint32_t GPHUDP;
    uint32_t GPJCON;
    uint32_t GPJDAT;
    uint32_t GPJUDP;
    uint32_t GPJSEL;
    uint32_t GPKCON;
    uint32_t GPKDAT;
    uint32_t GPKUDP;
    uint32_t GPLCON;
    uint32_t GPLDAT;
    uint32_t GPLUDP;
    uint32_t GPLSEL;
    uint32_t GPMCON;
    uint32_t GPMDAT;
    uint32_t GPMUDP;
    uint32_t MISCCR;
    uint32_t DCLKCON;
    uint32_t EXTINT0;
    uint32_t EXTINT1;
    uint32_t EXTINT2;
    uint32_t EINTFLT2;
    uint32_t EINTFLT3;
    uint32_t EINTMASK;
    uint32_t EINTPEND;
    uint32_t GSTATUS0;
    uint32_t GSTATUS1;
    uint32_t DSC0;
    uint32_t DSC1;
    uint32_t DSC2;
    uint32_t DSC3;
    uint32_t PDDMCON;
    uint32_t PDSMCON;
} s3c2416_gpio_state;


static uint64_t s3c2416_gpio_read(void *opaque, hwaddr offset,
    unsigned size)
{
    s3c2416_gpio_state *s = (s3c2416_gpio_state*)opaque;
    uint32_t val;

    switch (offset)
    {
        // GPACON
    case 0x0:
        val = s->GPACON;
        break;

        // GPADAT
    case 0x4:
        val = s->GPADAT;
        break;

        // GPBCON
    case 0x10:
        val = s->GPBCON;
        break;

        // GPBDAT
    case 0x14:
        val = s->GPBDAT;
        break;

        // GPBUDP
    case 0x18:
        val = s->GPBUDP;
        break;

        // GPBSEL
    case 0x1C:
        val = s->GPBSEL;
        break;

        // GPCCON
    case 0x20:
        val = s->GPCCON;
        break;

        // GPCDAT
    case 0x24:
        val = s->GPCDAT;
        break;

        // GPCUDP
    case 0x28:
        val = s->GPCUDP;
        break;

        // GPDCON
    case 0x30:
        val = s->GPDCON;
        break;

        // GPDDAT
    case 0x34:
        val = s->GPDDAT;
        break;

        // GPDUDP
    case 0x38:
        val = s->GPDUDP;
        break;

        // GPECON
    case 0x40:
        val = s->GPECON;
        break;

        // GPEDAT
    case 0x44:
        val = s->GPEDAT;
        break;

        // GPEUDP
    case 0x48:
        val = s->GPEUDP;
        break;

        // GPESEL
    case 0x4C:
        val = s->GPESEL;
        break;

        // GPFCON
    case 0x50:
        val = s->GPFCON;
        break;

        // GPFDAT
    case 0x54:
        val = s->GPFDAT;
        break;

        // GPFUDP
    case 0x58:
        val = s->GPFUDP;
        break;

        // GPGCON
    case 0x60:
        val = s->GPGCON;
        break;

        // GPGDAT
    case 0x64:
        val = s->GPGDAT;
        break;

        // GPGUDP
    case 0x68:
        val = s->GPGUDP;
        break;

        // GPHCON
    case 0x70:
        val = s->GPHCON;
        break;

        // GPHDAT
    case 0x74:
        val = s->GPHDAT;
        break;

        // GPHUDP
    case 0x78:
        val = s->GPHUDP;
        break;

        // GPJCON
    case 0xD0:
        val = s->GPJCON;
        break;

        // GPJDAT
    case 0xD4:
        val = s->GPJDAT;
        break;

        // GPJUDP
    case 0xD8:
        val = s->GPJUDP;
        break;

        // GPJSEL
    case 0xDC:
        val = s->GPJSEL;
        break;

        // GPKCON
    case 0xE0:
        val = s->GPKCON;
        break;

        // GPKDAT
    case 0xE4:
        val = s->GPKDAT;
        break;

        // GPKUDP
    case 0xE8:
        val = s->GPKUDP;
        break;

        // GPLCON
    case 0xF0:
        val = s->GPLCON;
        break;

        // GPLDAT
    case 0xF4:
        val = s->GPLDAT;
        break;

        // GPLUDP
    case 0xF8:
        val = s->GPLUDP;
        break;

        // GPLSEL
    case 0xFC:
        val = s->GPLSEL;
        break;

        // GPMCON
    case 0x100:
        val = s->GPMCON;
        break;

        // GPMDAT
    case 0x104:
        val = s->GPMDAT;
        break;

        // GPMUDP
    case 0x108:
        val = s->GPMUDP;
        break;

        // MISCCR
    case 0x80:
        val = s->MISCCR;
        break;

        // DCLKCON
    case 0x84:
        val = s->DCLKCON;
        break;

        // EXTINT0
    case 0x88:
        val = s->EXTINT0;
        break;

        // EXTINT1
    case 0x8C:
        val = s->EXTINT1;
        break;

        // EXTINT2
    case 0x90:
        val = s->EXTINT2;
        break;

        // EINTFLT2
    case 0x9C:
        val = s->EINTFLT2;
        break;

        // EINTFLT3
    case 0xA0:
        val = s->EINTFLT3;
        break;

        // EINTMASK
    case 0xA4:
        val = s->EINTMASK;
        break;

        // EINTPEND
    case 0xA8:
        val = s->EINTPEND;
        break;

        // GSTATUS0
    case 0xAC:
        val = s->GSTATUS0;
        break;

        // GSTATUS1
    case 0xB0:
        val = s->GSTATUS1;
        break;

        // DSC0
    case 0xC0:
        val = s->DSC0;
        break;

        // DSC1
    case 0xC4:
        val = s->DSC1;
        break;

        // DSC2
    case 0xC8:
        val = s->DSC2;
        break;

        // DSC3
    case 0x110:
        val = s->DSC3;
        break;

        // PDDMCON
    case 0x114:
        val = s->PDDMCON;
        break;

        // PDSMCON
    case 0x118:
        val = s->PDSMCON;
        break;

    default:
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset %"HWADDR_PRIx"\n",
                      __func__, offset);
        val = 0;
        break;
    }

    return val;
};

static void s3c2416_gpio_write(void *opaque, hwaddr offset,
    uint64_t val, unsigned size)
{
    s3c2416_gpio_state *s = (s3c2416_gpio_state*)opaque;

    switch (offset)
    {
        // GPACON
    case 0x0:
        s->GPACON = val;
        break;

        // GPADAT
    case 0x4:
        s->GPADAT = val;
        break;

        // GPBCON
    case 0x10:
        s->GPBCON = val;
        break;

        // GPBDAT
    case 0x14:
        s->GPBDAT = val;
        break;

        // GPBUDP
    case 0x18:
        s->GPBUDP = val;
        break;

        // GPBSEL
    case 0x1C:
        s->GPBSEL = val;
        break;

        // GPCCON
    case 0x20:
        s->GPCCON = val;
        break;

        // GPCDAT
    case 0x24:
        s->GPCDAT = val;
        break;

        // GPCUDP
    case 0x28:
        s->GPCUDP = val;
        break;

        // GPDCON
    case 0x30:
        s->GPDCON = val;
        break;

        // GPDDAT
    case 0x34:
        s->GPDDAT = val;
        break;

        // GPDUDP
    case 0x38:
        s->GPDUDP = val;
        break;

        // GPECON
    case 0x40:
        s->GPECON = val;
        break;

        // GPEDAT
    case 0x44:
        s->GPEDAT = val;
        break;

        // GPEUDP
    case 0x48:
        s->GPEUDP = val;
        break;

        // GPESEL
    case 0x4C:
        s->GPESEL = val;
        break;

        // GPFCON
    case 0x50:
        s->GPFCON = val;
        break;

        // GPFDAT
    case 0x54:
        s->GPFDAT = val;
        break;

        // GPFUDP
    case 0x58:
        s->GPFUDP = val;
        break;

        // GPGCON
    case 0x60:
        s->GPGCON = val;
        break;

        // GPGDAT
    case 0x64:
        s->GPGDAT = val;
        break;

        // GPGUDP
    case 0x68:
        s->GPGUDP = val;
        break;

        // GPHCON
    case 0x70:
        s->GPHCON = val;
        break;

        // GPHDAT
    case 0x74:
        s->GPHDAT = val;
        break;

        // GPHUDP
    case 0x78:
        s->GPHUDP = val;
        break;

        // GPJCON
    case 0xD0:
        s->GPJCON = val;
        break;

        // GPJDAT
    case 0xD4:
        s->GPJDAT = val;
        break;

        // GPJUDP
    case 0xD8:
        s->GPJUDP = val;
        break;

        // GPJSEL
    case 0xDC:
        s->GPJSEL = val;
        break;

        // GPKCON
    case 0xE0:
        s->GPKCON = val;
        break;

        // GPKDAT
    case 0xE4:
        s->GPKDAT = val;
        break;

        // GPKUDP
    case 0xE8:
        s->GPKUDP = val;
        break;

        // GPLCON
    case 0xF0:
        s->GPLCON = val;
        break;

        // GPLDAT
    case 0xF4:
        s->GPLDAT = val;
        break;

        // GPLUDP
    case 0xF8:
        s->GPLUDP = val;
        break;

        // GPLSEL
    case 0xFC:
        s->GPLSEL = val;
        break;

        // GPMCON
    case 0x100:
        s->GPMCON = val;
        break;

        // GPMUDP
    case 0x108:
        s->GPMUDP = val;
        break;

        // MISCCR
    case 0x80:
        s->MISCCR = val;
        break;

        // DCLKCON
    case 0x84:
        s->DCLKCON = val;
        break;

        // EXTINT0
    case 0x88:
        s->EXTINT0 = val;
        break;

        // EXTINT1
    case 0x8C:
        s->EXTINT1 = val;
        break;

        // EXTINT2
    case 0x90:
        s->EXTINT2 = val;
        break;

        // EINTFLT2
    case 0x9C:
        s->EINTFLT2 = val;
        break;

        // EINTFLT3
    case 0xA0:
        s->EINTFLT3 = val;
        break;

        // EINTMASK
    case 0xA4:
        s->EINTMASK = val;
        break;

        // EINTPEND
    case 0xA8:
        s->EINTPEND = val;
        break;

        // DSC0
    case 0xC0:
        s->DSC0 = val;
        break;

        // DSC1
    case 0xC4:
        s->DSC1 = val;
        break;

        // DSC2
    case 0xC8:
        s->DSC2 = val;
        break;

        // DSC3
    case 0x110:
        s->DSC3 = val;
        break;

        // PDDMCON
    case 0x114:
        s->PDDMCON = val;
        break;

        // PDSMCON
    case 0x118:
        s->PDSMCON = val;
        break;

    default:
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset %"HWADDR_PRIx"\n",
                      __func__, offset);
        break;
    }
};

static void s3c2416_gpio_keyboard_event(void *opaque, int keycode)
{
    int rel;
    s3c2416_gpio_state* s = (s3c2416_gpio_state*)opaque;

    rel = (keycode & 0x80) ? 1 : 0; /* key release from qemu */
    keycode &= ~(0x80); /* strip qemu key release bit */

    if (keycode  == 80)
    {
        if (rel)
        {
            s->GPDDAT &= ~(1 << 5);
            s->GPGDAT &= ~(1 << 4);
        }
        else
        {
            s->GPDDAT |= (1 << 5);
            s->GPGDAT |= (1 << 4);
        }
    }
    else if (keycode == 72)
    {
        if (rel)
        {
            s->GPDDAT &= ~(1 << 4);
            s->GPGDAT &= ~(1 << 5);
        }
        else
        {
            s->GPDDAT |= (1 << 4);
            s->GPGDAT |= (1 << 5);
        }
    }
    else if (keycode == 28)
    {
        if (rel)
        {
            s->GPDDAT &= ~(1 << 0);
            s->GPGDAT &= ~(1 << 7);
        }
        else
        {
            s->GPDDAT |= (1 << 0);
            s->GPGDAT |= (1 << 7);
        }
    }
};


static const MemoryRegionOps s3c2416_gpio_ops = {
    .read = s3c2416_gpio_read,
    .write = s3c2416_gpio_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};


static void s3c2416_gpio_init(Object *obj)
{
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);
    s3c2416_gpio_state *s = S3C2416_GPIO(obj);

    memory_region_init_io(&s->iomem, OBJECT(s), &s3c2416_gpio_ops, s, "s3c2416_gpio", 0x00001000);
    sysbus_init_mmio(sbd, &s->iomem);

    s->GPACON = 0xFFFFFF;
    s->GPADAT = 0x0;
    s->GPBCON = 0x0;
    s->GPBDAT = 0x0;
    s->GPBUDP = 0x155555;
    s->GPBSEL = 0x1;
    s->GPCCON = 0x0;
    s->GPCDAT = 0x0;
    s->GPCUDP = 0x55555555;
    s->GPDCON = 0x0;
    s->GPDDAT = 0x0;
    s->GPDUDP = 0x55555555;
    s->GPECON = 0x0;
    s->GPEDAT = 0x0;
    s->GPEUDP = 0x55555555;
    s->GPESEL = 0x0;
    s->GPFCON = 0x0;
    s->GPFDAT = 0x0;
    s->GPFUDP = 0x5555;
    s->GPGCON = 0x0;
    s->GPGDAT = 0x0;
    s->GPGUDP = 0x55555555;
    s->GPHCON = 0x0;
    s->GPHDAT = 0x0;
    s->GPHUDP = 0x15555555;
    s->GPJCON = 0x0;
    s->GPJDAT = 0x0;
    s->GPJUDP = 0x55555555;
    s->GPJSEL = 0x0;
    s->GPKCON = 0xAAAAAAAA;
    s->GPKDAT = 0x0;
    s->GPKUDP = 0x55555555;
    s->GPLCON = 0x0;
    s->GPLDAT = 0x0;
    s->GPLUDP = 0x15555555;
    s->GPLSEL = 0x0;
    s->GPMCON = 0xA;
    s->GPMDAT = 0x0;
    s->GPMUDP = 0x0;
    s->MISCCR = 0xD0010020;
    s->DCLKCON = 0x0;
    s->EXTINT0 = 0x0;
    s->EXTINT1 = 0x0;
    s->EXTINT2 = 0x0;
    s->EINTFLT2 = 0x0;
    s->EINTFLT3 = 0x0;
    s->EINTMASK = 0xFFFFF0;
    s->EINTPEND = 0x0;
    s->GSTATUS0 = 0x0;
    s->GSTATUS1 = 0x32440001;
    s->DSC0 = 0x2AAAAAAA;
    s->DSC1 = 0xAAAAAAA;
    s->DSC2 = 0xAAAAAAA;
    s->DSC3 = 0x2AA;
    s->PDDMCON = 0x411540;
    s->PDSMCON = 0x5451500;

    qemu_add_kbd_event_handler(s3c2416_gpio_keyboard_event, s);
};

static const TypeInfo s3c2416_gpio_type = {
    .name = TYPE_S3C2416_GPIO,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(s3c2416_gpio_state),
    .instance_init = s3c2416_gpio_init
};

static void s3c2416_gpio_register(void)
{
    type_register_static(&s3c2416_gpio_type);
}

type_init(s3c2416_gpio_register);
