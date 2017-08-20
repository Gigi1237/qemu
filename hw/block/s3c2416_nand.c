#include "qemu/osdep.h"
#include "qemu-common.h"
#include "hw/sysbus.h"
#include "hw/block/flash.h"
#include "qemu/log.h"
#include "hw/boards.h"


#define TYPE_PRIME_NAND "s3c2416-nand"
#define PRIME_NAND(obj) OBJECT_CHECK(s3c2416_nand_state, (obj), TYPE_PRIME_NAND)



#define NFCE0(s) (s->NFCONT & (1u << 1))
#define NFCONF_OFF 0x0
#define NFCONT_OFF 0x4
#define NFCMMD_OFF 0x8
#define NFADDR_OFF 0xC
#define NFDATA_OFF 0x10
#define NFSBLK_OFF 0x20
#define NFEBLK_OFF 0x24
#define NFSTAT_OFF 0x28
#define NFECCERR0_off 0x2C
#define NFECCERR1_off 0x30


typedef struct
{
    SysBusDevice parent_obj;

    MemoryRegion iomem;
    DeviceState* nand;

    uint32_t NFCONF;
    uint32_t NFCONT;
    uint32_t NFCMMD;
    uint32_t NFADDR;
    uint32_t NFDATA;
    uint32_t NFMECCD[3];
    uint32_t NFSBLK;
    uint32_t NFEBLK;
    uint32_t NFSTAT;

} s3c2416_nand_state;

static uint64_t s3c2416_nand_read(void *opaque,
    hwaddr offset,
    unsigned size)
{
    s3c2416_nand_state *s = (s3c2416_nand_state*)opaque;

    uint64_t ret;
    switch (offset) {
    case NFCMMD_OFF:
        return s->NFCMMD;
    case NFCONF_OFF:
        return s->NFCONF;
    case NFCONT_OFF:
        return s->NFCONT;
    case NFADDR_OFF:
        return s->NFADDR;
    case NFSBLK_OFF:
        return s->NFSBLK;
        break;
    case NFEBLK_OFF:
        return s->NFEBLK;
        break;
    case NFDATA_OFF:
        if (s->nand != NULL) {
            nand_setpins(s->nand, 0, 0, NFCE0(s), 1, 0);
            s->NFDATA = nand_getio(s->nand);
            return s->NFDATA;
        }
        else {
            s->NFDATA = 0;
            return 0;
        }
        // Untested may be compleltey wrong
    case NFSTAT_OFF:
        if (s->nand != NULL) {
            nand_getpins(s->nand, (int *)&ret);
            s->NFSTAT |= ret | (1u << 4) | (1u << 6);
            return s->NFSTAT;
        }
        else {
            s->NFSTAT = 0;
            return 0;
        }
    case NFECCERR0_off:
        if (((s->NFCONF >> 23) & 0x3) == 0x2) {
            return 1 << 30;
        }
        else
        {
            qemu_log_mask(LOG_UNIMP, "S3C2416_nand: Unimplemented ecc other than 4bit\n");
            return 0;
        }
    case NFECCERR1_off:
        if (((s->NFCONF >> 23) & 0x3) == 0x2) {
            return 0;
        }
        else
        {
            qemu_log_mask(LOG_UNIMP, "S3C2416_nand: Unimplemented ecc other than 4bit\n");
            return 0;
        }
    default:
        /* The rest read-back what was written to them */
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset %"HWADDR_PRIx"\n",
            __func__, offset);
        break;
    }

    return 0;
}

static void s3c2416_nand_write(void *opaque,
    hwaddr offset,
    uint64_t val,
    unsigned size)
{
    s3c2416_nand_state *s = (s3c2416_nand_state*)opaque;

    switch (offset) {
    case NFCONF_OFF:
        s->NFCONF = val;
        if (s->nand != NULL)
            nand_setpins(s->nand, 0, 0, NFCE0(s), 1, 0);
        break;

    case NFCONT_OFF:
        s->NFCONT = val;
        if (s->nand != NULL)
            nand_setpins(s->nand, 0, 0, NFCE0(s), 1, 0);
        break;

    case NFCMMD_OFF:
        s->NFCMMD = val;
        if (s->nand != NULL) {
            nand_setpins(s->nand, 1, 0, NFCE0(s), 1, 0);
            nand_setio(s->nand, val);
        }
        break;

    case NFADDR_OFF:
        s->NFADDR = val;
        if (s->nand != NULL) {
            nand_setpins(s->nand, 0, 1, NFCE0(s), 1, 0);
            nand_setio(s->nand, val);
        }
        break;

    case NFDATA_OFF:
        s->NFDATA = val;
        if (s->nand != NULL) {
            nand_setpins(s->nand, 0, 0, NFCE0(s), 1, 0);
            nand_setio(s->nand, val);
        }
        break;

    case NFSBLK_OFF:
        s->NFSBLK = val;
        break;

    case NFEBLK_OFF:
        s->NFEBLK = val;
        break;

    case NFSTAT_OFF:
        s->NFSTAT &= ~0x70;
        s->NFSTAT |= (val & 0x70);
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset %"HWADDR_PRIx"\n",
            __func__, offset);
        /* Do nothing because the other registers are read only */
        break;
    }

    //printf("S3C2416_nand write: 0x%llx, val: 0x%llx\n", 0x4e000000 + addr, val);

}

static const MemoryRegionOps s3c2416_nand_ops = {
    .read = s3c2416_nand_read,
    .write = s3c2416_nand_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};


static void s3c2416_nand_init(Object *obj)
{
    s3c2416_nand_state *s = PRIME_NAND(obj);
    SysBusDevice *dev = SYS_BUS_DEVICE(obj);
    DriveInfo *nand;

    /* FIXME use a qdev drive property instead of drive_get() */
    nand = drive_get(IF_MTD, 0, 0);
    s->nand = nand_init(nand ? blk_by_legacy_dinfo(nand) : NULL,
        0xAD/*0xEC*/, 0xDA);

    memory_region_init_io(&s->iomem, obj, &s3c2416_nand_ops, s, "prime", 0x40);
    sysbus_init_mmio(dev, &s->iomem);
}

static const TypeInfo s3c2416_nand_info = {
    .name = TYPE_PRIME_NAND,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(s3c2416_nand_state),
    .instance_init = s3c2416_nand_init
};

static void s3c2416_nand_register(void)
{
    type_register_static(&s3c2416_nand_info);
}
type_init(s3c2416_nand_register);
