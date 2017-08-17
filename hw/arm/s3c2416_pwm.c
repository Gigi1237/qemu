#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu-common.h"
#include "qemu/log.h"
#include "hw/sysbus.h"

#define TYPE_S3C2416_PWM "s3c2416-pwm"
#define S3C2416_PWM(obj) OBJECT_CHECK(s3c2416_pwm_state, (obj), TYPE_S3C2416_PWM)

typedef struct {
    SysBusDevice parent_obj;

    MemoryRegion iomem;
    
    uint32_t TCFG0;
    uint32_t TCFG1;
    uint32_t TCON;
    uint32_t TCNTB0;
    uint32_t TCMPB0;
    uint32_t TCNTO0;
    uint32_t TCNTB1;
    uint32_t TCMPB1;
    uint32_t TCNTO1;
    uint32_t TCNTB2;
    uint32_t TCMPB2;
    uint32_t TCNTO2;
    uint32_t TCNTB3;
    uint32_t TCMPB3;
    uint32_t TCNTO3;
    uint32_t TCNTB4;
    uint32_t TCNTO4;
} s3c2416_pwm_state;


static uint64_t s3c2416_pwm_read(void *opaque, hwaddr offset,
    unsigned size)
{
    s3c2416_pwm_state *s = (s3c2416_pwm_state*)opaque;
    uint32_t val;
    
    switch (offset)
    {
    // TCFG0
    case 0x0:
        val = s->TCFG0;
        break;

    // TCFG1
    case 0x4:
        val = s->TCFG1;
        break;

    // TCON
    case 0x8:
        val = s->TCON;
        break;

    // TCNTB0
    case 0xC:
        val = s->TCNTB0;
        break;

    // TCMPB0
    case 0x10:
        val = s->TCMPB0;
        break;

    // TCNTO0
    case 0x14:
        val = s->TCNTO0;
        break;

    // TCNTB1
    case 0x18:
        val = s->TCNTB1;
        break;

    // TCMPB1
    case 0x1C:
        val = s->TCMPB1;
        break;

    // TCNTO1
    case 0x20:
        val = s->TCNTO1;
        break;

    // TCNTB2
    case 0x24:
        val = s->TCNTB2;
        break;

    // TCMPB2
    case 0x28:
        val = s->TCMPB2;
        break;

    // TCNTO2
    case 0x2C:
        val = s->TCNTO2;
        break;

    // TCNTB3
    case 0x30:
        val = s->TCNTB3;
        break;

    // TCMPB3
    case 0x34:
        val = s->TCMPB3;
        break;

    // TCNTO3
    case 0x38:
        val = s->TCNTO3;
        break;

    // TCNTB4
    case 0x3C:
        val = s->TCNTB4;
        break;

    // TCNTO4
    case 0x40:
        val = s->TCNTO4;
        break;

        default:
        qemu_log_mask(LOG_GUEST_ERROR, "s3c2416_pwm: Attempted write to invalid register! off: 0x%llx\n", offset);
        break;
    }
    
    return val;
};

static void s3c2416_pwm_write(void *opaque, hwaddr offset,
    uint64_t val, unsigned size)
{
    s3c2416_pwm_state *s = (s3c2416_pwm_state*)opaque;
    
    switch (offset)
    {
    // TCFG0
    case 0x0:
        s->TCFG0 = val;
        break;

    // TCFG1
    case 0x4:
        s->TCFG1 = val;
        break;

    // TCON
    case 0x8:
        s->TCON = val;
        break;

    // TCNTB0
    case 0xC:
        s->TCNTB0 = val;
        break;

    // TCMPB0
    case 0x10:
        s->TCMPB0 = val;
        break;

    // TCNTB1
    case 0x18:
        s->TCNTB1 = val;
        break;

    // TCMPB1
    case 0x1C:
        s->TCMPB1 = val;
        break;

    // TCNTB2
    case 0x24:
        s->TCNTB2 = val;
        break;

    // TCMPB2
    case 0x28:
        s->TCMPB2 = val;
        break;

    // TCNTB3
    case 0x30:
        s->TCNTB3 = val;
        break;

    // TCMPB3
    case 0x34:
        s->TCMPB3 = val;
        break;

    // TCNTB4
    case 0x3C:
        s->TCNTB4 = val;
        break;

    default:
        qemu_log_mask(LOG_GUEST_ERROR, "s3c2416_pwm: Attempted write to invalid register! off: 0x%llx\n", offset);
        break;
    }
};
 

static const MemoryRegionOps s3c2416_pwm_ops = {
    .read = s3c2416_pwm_read,
    .write = s3c2416_pwm_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};


static void s3c2416_pwm_init(Object *obj)
{
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);
    s3c2416_pwm_state *s = S3C2416_PWM(obj);

    memory_region_init_io(&s->iomem, OBJECT(s), &s3c2416_pwm_ops, s, "s3c2416_pwm", 0x00001000);
    sysbus_init_mmio(sbd, &s->iomem);

    s->TCFG0 = 0x0;
    s->TCFG1 = 0x0;
    s->TCON = 0x0;
    s->TCNTB0 = 0x0;
    s->TCMPB0 = 0x0;
    s->TCNTO0 = 0x0;
    s->TCNTB1 = 0x0;
    s->TCMPB1 = 0x0;
    s->TCNTO1 = 0x0;
    s->TCNTB2 = 0x0;
    s->TCMPB2 = 0x0;
    s->TCNTO2 = 0x0;
    s->TCNTB3 = 0x0;
    s->TCMPB3 = 0x0;
    s->TCNTO3 = 0x0;
    s->TCNTB4 = 0x0;
    s->TCNTO4 = 0x0;
};

static const TypeInfo s3c2416_pwm_type = {
    .name = TYPE_S3C2416_PWM,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(s3c2416_pwm_state),
    .instance_init = s3c2416_pwm_init
};

static void s3c2416_pwm_register(void)
{
    type_register_static(&s3c2416_pwm_type);
}

type_init(s3c2416_pwm_register);
