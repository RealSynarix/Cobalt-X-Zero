#include "vdrive.h"
#include <stm32g4xx.h>
#include <string.h>

typedef struct {
    int8_t (*Init)(uint8_t);
    int8_t (*GetCapacity)(uint8_t, uint32_t *, uint16_t *);
    int8_t (*IsReady)(uint8_t);
    int8_t (*IsWriteProtected)(uint8_t);
    int8_t (*Read)(uint8_t, uint8_t *, uint32_t, uint16_t);
    int8_t (*Write)(uint8_t, uint8_t *, uint32_t, uint16_t);
    int8_t (*GetMaxLun)(void);
    uint8_t *pInquiry;
} USBD_StorageTypeDef;

static volatile uint8_t g_enabled = 0;

enum : uint32_t {
    SECTOR_SIZE = 512u,
    TOTAL_SECTORS = 141312u,
    FAT_SECTORS = 69u,
    ROOT_ENTRIES = 128u,
    ROOT_SECTORS = ((ROOT_ENTRIES * 32u + (SECTOR_SIZE - 1u)) / SECTOR_SIZE),
    ROOT_START = 1u + (FAT_SECTORS * 2u),
    DATA_START = ROOT_START + ROOT_SECTORS
};

static const uint8_t kInquiryData[] = {
    0x00, 0x80, 0x02, 0x02, 0x1F, 0x00, 0x00, 0x00,
    'S','y','n','a','r','i','x',' ',
    'C','o','b','a','l','t','-','X',
    ' ','C','o','n','f','i','g',' ',
    '1','.','0','0'
};

static inline void w16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
}

static inline void w32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
    p[2] = (uint8_t)((v >> 16) & 0xFFu);
    p[3] = (uint8_t)((v >> 24) & 0xFFu);
}

static inline void fill11(uint8_t *dst, const char *src) {
    for (uint8_t i = 0; i < 11u; ++i) {
        dst[i] = (uint8_t)src[i];
    }
}

static void boot(uint8_t *b) {
    memset(b, 0, SECTOR_SIZE);
    b[0] = 0xEB;
    b[1] = 0x3C;
    b[2] = 0x90;
    fill11(b + 3, "MSDOS5.0");
    w16(b + 11, (uint16_t)SECTOR_SIZE);
    b[13] = 8u;
    w16(b + 14, 1u);
    b[16] = 2u;
    w16(b + 17, (uint16_t)ROOT_ENTRIES);
    w16(b + 19, 0u);
    b[21] = 0xF8u;
    w16(b + 22, (uint16_t)FAT_SECTORS);
    w16(b + 24, 63u);
    w16(b + 26, 255u);
    w32(b + 28, 0u);
    w32(b + 32, TOTAL_SECTORS);
    b[36] = 0x80u;
    b[37] = 0x00u;
    b[38] = 0x29u;
    w32(b + 39, 0x20260414u);
    fill11(b + 43, "COBALTXCFG ");
    fill11(b + 54, "FAT16   ");
    b[510] = 0x55u;
    b[511] = 0xAAu;
}

static void fat(uint8_t *b, uint32_t i) {
    memset(b, 0, SECTOR_SIZE);
    if (i == 0u) {
        b[0] = 0xF8u;
        b[1] = 0xFFu;
        b[2] = 0xFFu;
        b[3] = 0xFFu;
        b[4] = 0xFFu;
        b[5] = 0xFFu;
    }
}

static void root(uint8_t *b, uint32_t i) {
    memset(b, 0, SECTOR_SIZE);
    if (i != 0u) {
        return;
    }

    fill11(b + 0, "COBALTXCFG ");
    b[11] = 0x08u;

    uint8_t *f = b + 32;
    fill11(f + 0, "CONFIG  INI");
    f[11] = 0x20u;
    w16(f + 26, 0u);
    w32(f + 28, 0u);
}

static void data(uint8_t *b, uint32_t) {
    memset(b, 0, SECTOR_SIZE);
}

static void sector(uint8_t *b, uint32_t lba) {
    if (lba == 0u) {
        boot(b);
        return;
    }

    if (lba < (1u + FAT_SECTORS)) {
        fat(b, lba - 1u);
        return;
    }

    if (lba < (1u + (FAT_SECTORS * 2u))) {
        fat(b, lba - (1u + FAT_SECTORS));
        return;
    }

    if (lba < DATA_START) {
        root(b, lba - ROOT_START);
        return;
    }

    if (lba < TOTAL_SECTORS) {
        data(b, lba - DATA_START);
        return;
    }

    memset(b, 0, SECTOR_SIZE);
}

extern "C" void vdrive_init(void) {
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    (void)RCC->AHB2ENR;

    GPIOB->MODER &= ~(3u << (6u * 2u));
    GPIOB->PUPDR = (GPIOB->PUPDR & ~(3u << (6u * 2u))) | (1u << (6u * 2u));

    g_enabled = ((GPIOB->IDR & (1u << 6u)) == 0u) ? 1u : 0u;
}

extern "C" uint8_t vdrive_enabled(void) {
    return g_enabled;
}

extern "C" int8_t STORAGE_Init_FS(uint8_t lun) {
    (void)lun;
    return g_enabled ? 0 : -1;
}

extern "C" int8_t STORAGE_GetCapacity_FS(uint8_t lun, uint32_t *block_num, uint16_t *block_size) {
    (void)lun;
    if (!g_enabled) {
        return -1;
    }
    *block_num = TOTAL_SECTORS;
    *block_size = SECTOR_SIZE;
    return 0;
}

extern "C" int8_t STORAGE_IsReady_FS(uint8_t lun) {
    (void)lun;
    return g_enabled ? 0 : -1;
}

extern "C" int8_t STORAGE_IsWriteProtected_FS(uint8_t lun) {
    (void)lun;
    return 1;
}

extern "C" int8_t STORAGE_Read_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len) {
    (void)lun;
    if (!g_enabled) {
        return -1;
    }

    if (blk_len == 0u) {
        return 0;
    }

    if ((blk_addr >= TOTAL_SECTORS) || ((blk_addr + (uint32_t)blk_len) > TOTAL_SECTORS)) {
        return -1;
    }

    for (uint16_t i = 0; i < blk_len; ++i) {
        sector(buf + ((uint32_t)i * SECTOR_SIZE), blk_addr + (uint32_t)i);
    }

    return 0;
}

extern "C" int8_t STORAGE_Write_FS(uint8_t, uint8_t *, uint32_t, uint16_t) {
    return -1;
}

extern "C" int8_t STORAGE_GetMaxLun_FS(void) {
    return 0;
}

extern "C" int8_t MSC_Init(uint8_t lun) {
    return STORAGE_Init_FS(lun);
}

extern "C" int8_t MSC_GetCapacity(uint8_t lun, uint32_t *block_num, uint16_t *block_size) {
    return STORAGE_GetCapacity_FS(lun, block_num, block_size);
}

extern "C" int8_t MSC_IsReady(uint8_t lun) {
    return STORAGE_IsReady_FS(lun);
}

extern "C" int8_t MSC_IsWriteProtected(uint8_t lun) {
    return STORAGE_IsWriteProtected_FS(lun);
}

extern "C" int8_t MSC_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len) {
    return STORAGE_Read_FS(lun, buf, blk_addr, blk_len);
}

extern "C" int8_t MSC_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len) {
    return STORAGE_Write_FS(lun, buf, blk_addr, blk_len);
}

extern "C" int8_t MSC_GetMaxLun(void) {
    return STORAGE_GetMaxLun_FS();
}

USBD_StorageTypeDef USBD_Storage_Interface_fops_FS = {
    STORAGE_Init_FS,
    STORAGE_GetCapacity_FS,
    STORAGE_IsReady_FS,
    STORAGE_IsWriteProtected_FS,
    STORAGE_Read_FS,
    STORAGE_Write_FS,
    STORAGE_GetMaxLun_FS,
    (uint8_t *)kInquiryData
};
