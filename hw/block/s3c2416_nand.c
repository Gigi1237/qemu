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
#define S3C_NFCONT_MODE		(1 << 0)	/* NAND flash controller operating mode */
#define S3C_NFCONT_CE		(1 << 1)	/* NAND Flash Memory nFCE signal control 0: active) */
#define S3C_NFCONT_INITSECC	(1 << 4)	/* Initialize ECC decoder/encoder(Write-only) */
#define S3C_NFCONT_INITMECC	(1 << 5)	/* Initialize ECC decoder/encoder(Write-only) */
#define S3C_NFCONT_MECCL	(1 << 7)	/* Lock Main data area ECC generation */
#define S3C_NFCONT_SECCL	(1 << 6)	/* Lock spare area ECC generation */
#define S3C_NFCONT_SLOCK	(1 << 16)	/* Soft Lock configuration */
#define S3C_NFCONT_TLOCK	(1 << 17)	/* Lock-tight configuration */
#define S3C_NFCONT_LOCK (S3C_NFCONT_SLOCK | S3C_NFCONT_TLOCK)


#define NFCMMD_OFF    0x8
#define NFADDR_OFF    0xC
#define NFDATA_OFF    0x10
#define NFMECCD0_OFF  0x14
#define NFMECCD1_OFF  0x18
#define NFSECCD_OFF   0x1C
#define NFSBLK_OFF    0x20
#define NFEBLK_OFF    0x24
#define NFSTAT_OFF    0x28
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
    uint32_t NFSBLK;
    uint32_t NFEBLK;
    uint32_t NFSTAT;

    ECCState nfecc;
    ECCState nfsecc;

    uint32_t nfaddr_cur;
    uint32_t nfsblk;
    uint32_t nfeblk;

} s3c2416_nand_state;

uint8_t dbu[16];
uint8_t cmd;

static uint64_t s3c2416_nand_read(void *opaque,
    hwaddr offset,
    unsigned size)
{
    s3c2416_nand_state *s = (s3c2416_nand_state*)opaque;
    int shr = 0;
    uint32_t ret = 0;

    //printf("s3c2416_nand read: %02llx\n", offset + 0x4e000000);

    if (!s->nand) {
        return 0;
    }

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
        if (s->NFCONT & S3C_NFCONT_MODE)
        {
            uint32_t value;
            value = nand_getio(s->nand);

            if (s->nfaddr_cur < 512) {
                if (!(s->NFCONT & S3C_NFCONT_MECCL)) {
                    value = ecc_digest(&s->nfecc, value & 0xFF);
                    //printf("ecc %02x -> %08x %08x cp %08x cnt %d\n", value, s->nfecc.lp[0], s->nfecc.lp[1], s->nfecc.cp, s->nfecc.count);
                }
            }
            else {
                if (!(s->NFCONT & S3C_NFCONT_SECCL)) {
                    value = ecc_digest(&s->nfsecc, value & 0xFF);
                }
            }
            if (s->nfaddr_cur < 16) dbu[s->nfaddr_cur] = value;
            s->nfaddr_cur++;
            return value;
        }
        else {
            return 0;
        }
        // Untested may be compleltey wrong
    case NFSTAT_OFF:
        nand_getpins(s->nand, (int *)&ret);
        s->NFSTAT |= ret | (1u << 4) | (1u << 6);
        return s->NFSTAT;
    case NFMECCD0_OFF + 3: shr += 8;
    case NFMECCD0_OFF + 2: shr += 8;
    case NFMECCD0_OFF + 1: shr += 8;
    case NFMECCD0_OFF: {
#define ECC(shr, b, shl)	((s->nfecc.lp[b] << (shl - shr)) & (1 << shl))
        uint32_t ecc = ~(
            ECC(0, 1, 0) | ECC(0, 0, 1) | ECC(1, 1, 2) | ECC(1, 0, 3) | ECC(2, 1, 4) | ECC(2, 0, 5) | ECC(3, 1, 6) | ECC(3, 0, 7) |
            ECC(4, 1, 8) | ECC(4, 0, 9) | ECC(5, 1, 10) | ECC(5, 0, 11) | ECC(6, 1, 12) | ECC(6, 0, 13) | ECC(7, 1, 14) | ECC(7, 0, 15) |
            ECC(8, 1, 16) | ECC(8, 0, 17) | ((s->nfecc.cp & 0x3f) << 18) |
            ECC(9, 1, 28) | ECC(9, 0, 29) | ECC(10, 1, 30) | ECC(10, 0, 31)
            );
        //printf("Read ECC %02llx = %08x >> %d [ 0: %08x 1: %08x cnt %d ]\n", offset, ecc, shr, s->nfecc.lp[0], s->nfecc.lp[1], s->nfecc.count);
        ecc = (ecc >> shr) & 0xFF;
        return ecc;
    }	break;
#undef ECC

    case NFMECCD1_OFF + 3:
    case NFMECCD1_OFF + 2:
    case NFMECCD1_OFF + 1:
    case NFMECCD1_OFF:
        /* the ecc digester is limited to 2, the s3c2440 can do 4... */
        //printf("%s: Bad register S3C_NFECCD1 NOT HANDLED\n", __FUNCTION__);
        break;

    case NFSECCD_OFF + 3: shr += 8;
    case NFSECCD_OFF + 2: shr += 8;
    case NFSECCD_OFF + 1: shr += 8;
    case NFSECCD_OFF:
#define ECC(shr, b, shl)	((s->nfsecc.lp[b] << (shl - shr)) & (1 << shl))
        return (~(
            ECC(0, 1, 0) | ECC(0, 0, 1) | ECC(1, 1, 2) | ECC(1, 0, 3) | ECC(2, 1, 4) | ECC(2, 0, 5) | ECC(3, 1, 6) | ECC(3, 0, 7) |
            ECC(4, 1, 8) | ECC(4, 0, 9) | ECC(5, 1, 10) | ECC(5, 0, 11) | ECC(6, 1, 12) | ECC(6, 0, 13) | ECC(7, 1, 14) | ECC(7, 0, 15) |
            ECC(8, 1, 16) | ECC(8, 0, 17) | ((s->nfecc.cp & 0x3f) << 18) |
            ECC(9, 1, 28) | ECC(9, 0, 29) | ECC(10, 1, 30) | ECC(10, 0, 31)
            ) >> shr) &
            0xff;
#undef ECC
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

    if (!s->nand)
        return;

    switch (offset) {
    case NFCONF_OFF:
        s->NFCONF = val & 0x380777F;
        break;

    case NFCONT_OFF:
        if (val & S3C_NFCONT_INITMECC)
            ecc_reset(&s->nfecc);
        if (val & S3C_NFCONT_INITSECC)
            ecc_reset(&s->nfsecc);

        s->NFCONT = (val & 0x71FF7) | (s->NFCONT & S3C_NFCONT_TLOCK);
        break;
    case NFCMMD_OFF:
        s->NFCMMD = val;
        if (s->nand != NULL) {
            if (s->NFCONT & S3C_NFCONT_MODE)
            {
                nand_setpins(s->nand, 1, 0, NFCE0(s), 1, 0);
                nand_setio(s->nand, s->NFCMMD);
                nand_setpins(s->nand, 0, 0, NFCE0(s), 1, 0);
            }
        }
        break;

    case NFADDR_OFF:
         s->NFADDR = val & 0xFF;
         s->nfaddr_cur = 0;
         if (s->NFCONT & S3C_NFCONT_MODE) {
             nand_setpins(s->nand, 0, 1, NFCE0(s), 1, 0);
             nand_setio(s->nand, val & 0xFF);
             nand_setpins(s->nand, 0, 0, NFCE0(s), 1, 0);
         }
        break;

    case NFDATA_OFF:
        if (s->NFCONT & S3C_NFCONT_MODE) {
            if (s->NFCONT & S3C_NFCONT_LOCK) {
                if (s->nfaddr_cur < (s->nfsblk << 6) ||
                    s->nfaddr_cur >(s->nfeblk << 6)) {
                    /* TODO: ADD IRQ */
                    break;
                }
            }
            if (s->nfaddr_cur < 512) {
                if (!(s->NFCONT & S3C_NFCONT_MECCL)) {
                    val = ecc_digest(&s->nfecc, val & 0xff);
                    //printf("ecc %02llx -> %08x %08x cp %08x cnt %d\n", val, s->nfecc.lp[0], s->nfecc.lp[1], s->nfecc.cp, s->nfecc.count);
                }
            }
            else {
                if (!(s->NFCONT & S3C_NFCONT_SECCL))
                    val = ecc_digest(&s->nfsecc, val & 0xff);
            }
            if (s->nfaddr_cur < 16) dbu[s->nfaddr_cur] = val;
            s->nfaddr_cur++;
            nand_setio(s->nand, val & 0xff);
        }

    case NFSBLK_OFF:
        s->NFSBLK = val & 0xFFFFFF;
        break;

    case NFEBLK_OFF:
        s->NFEBLK = val & 0xFFFFFF;
        break;

    case NFSTAT_OFF:
        s->NFSTAT = val;
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset %"HWADDR_PRIx"\n",
            __func__, offset);
        /* Do nothing because the other registers are read only */
        break;
    }

    //printf("S3C2416_nand write: 0x%llx, val: 0x%llx\n", 0x4e000000 + offset, val);

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
        0xEC, 0xDA);

    s->NFCONF = 0x1000 |
        (0 << 3) |		/* NCON0: not an advanced flash */
        (1 << 2) | 		/* GPG13: 0: 256 Word/page, 1:   512 Bytes/page */
        (1 << 1) |		/* GPG14: 0: 3 address cycle 1: 4 address cycle */
        (0 << 2) 		/* GPG15: 0: 8-bit bus 1: 16-bit bus */
        ;
    s->NFCONT = 0x0384;
    s->NFCMMD = 0x0;
    s->NFADDR = 0x0;
    s->NFSBLK = 0x0;
    s->NFEBLK = 0x0;
    s->NFSTAT = 0x3;

    ecc_reset(&s->nfecc);
    ecc_reset(&s->nfsecc);

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
