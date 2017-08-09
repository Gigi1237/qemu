#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu-common.h"
#include "hw/hw.h"
#include "hw/boards.h"
#include "hw/arm/arm.h"
#include "hw/sysbus.h"
#include "exec/address-spaces.h"
#include "exec/memory.h"
#include "hw/block/flash.h"

#include <stdio.h>
#include <stdlib.h>


#define TYPE_PRIME_NAND "prime-nand"
#define PRIME_NAND(obj) OBJECT_CHECK(PrimeNANDState, (obj), TYPE_PRIME_NAND)

typedef struct
{
    SysBusDevice parent_obj;

    MemoryRegion iomem;
    DeviceState* nand;
} PrimeNANDState;

static uint64_t prime_read(void *opaque,
    hwaddr addr,
    unsigned size)
{
    return 0xFF;
}
static void prime_write(void *opaque,
    hwaddr addr,
    uint64_t data,
    unsigned size)
{
    
}

//static char b_to_steppingstone[] =
//{
//    0x01,
//    0xF1,
//    0xA0,
//    0xE3,
//};

static const MemoryRegionOps prime_nand_ops = {
    .read = prime_read,
    .write = prime_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void prime_init(MachineState *machine)
{
    ObjectClass *cpu_oc;
    Object *cpuobj;
    //ARMCPU *cpu;

    //Init CPU
    cpu_oc = cpu_class_by_name(TYPE_ARM_CPU, "arm926");
    if (!cpu_oc) {
        fprintf(stderr, "Unable to find CPU definition\n");
        exit(1);
    }

    cpuobj = object_new(object_class_get_name(cpu_oc));
    object_property_set_bool(cpuobj, true, "realized", &error_fatal);

   // cpu = ARM_CPU(cpuobj);

    // Init memory before nand
    MemoryRegion *address_space_mem = get_system_memory();
    MemoryRegion *ram = g_new(MemoryRegion, 1);
    //MemoryRegion *boot = g_new(MemoryRegion, 1);
    MemoryRegion *steppingStone = g_new(MemoryRegion, 1);

    FILE *f = fopen("D:/BXCBOOT0.BIN", "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  //same as rewind(f);

    char *mem = malloc(fsize);
    fread(mem, fsize, 1, f);

    printf("Read file:\n    + size: %ld\n", fsize);

    memory_region_init_ram(ram, NULL, "prime.ram", 0x00002000, &error_fatal);
    memory_region_add_subregion(address_space_mem, 0x30000000, ram);

    memory_region_init_ram_ptr(steppingStone, NULL, "prime.steppingstone", fsize, mem);
    memory_region_add_subregion(address_space_mem, 0x00000000, steppingStone);
    //memory_region_init_ram_ptr(boot, NULL, "prime.boot", 4, &b_to_steppingstone);
    //memory_region_add_subregion(address_space_mem, 0x0, boot);



    DeviceState *nand;
    nand = qdev_create(NULL, TYPE_PRIME_NAND);

    qdev_init_nofail(nand);
    sysbus_mmio_map(SYS_BUS_DEVICE(nand), 0, 0x4E000000);
}


static void prime_nand_init(Object *obj)
{
    PrimeNANDState *s = PRIME_NAND(obj);
    SysBusDevice *dev = SYS_BUS_DEVICE(obj);
    DriveInfo *nand;

    /* FIXME use a qdev drive property instead of drive_get() */
    nand = drive_get(IF_MTD, 0, 0);
    if (!nand) {
        fprintf(stderr, "A flash image must be given with the "
            "'blockmtd' parameter\n");
        exit(1);
    }
    s->nand = nand_init(nand ? blk_by_legacy_dinfo(nand) : NULL,
       0xEC, 0xDA);


    memory_region_init_io(&s->iomem, obj, &prime_nand_ops, s, "prime", 0x40);
    sysbus_init_mmio(dev, &s->iomem);
}

static void prime_class_init(ObjectClass *oc, void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);

    mc->desc = "HP Prime (S3C2416)";
    mc->init = prime_init;
}

static const TypeInfo prime_type = {
    .name = MACHINE_TYPE_NAME("hp-prime"),
    .parent = TYPE_MACHINE,
    .class_init = prime_class_init,
};

static const TypeInfo prime_nand_info = {
    .name = TYPE_PRIME_NAND,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(PrimeNANDState),
    .instance_init = prime_nand_init
};

static void prime_machine_init(void)
{
    type_register_static(&prime_type);
    type_register_static(&prime_nand_info);
}

type_init(prime_machine_init);