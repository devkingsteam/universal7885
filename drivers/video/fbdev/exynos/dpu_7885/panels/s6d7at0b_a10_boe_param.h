#ifndef __S6D7AT0B_PARAM_H__
#define __S6D7AT0B_PARAM_H__
#include <linux/types.h>
#include <linux/kernel.h>

#define EXTEND_BRIGHTNESS	306
#define UI_MAX_BRIGHTNESS	255
#define UI_DEFAULT_BRIGHTNESS	128

struct lcd_seq_info {
	unsigned char	*cmd;
	unsigned int	len;
	unsigned int	sleep;
};

struct i2c_rom_data {
	u8 addr;
	u8 val;
};

static u8 LM3632_INIT[] = {
	0x09, 0x41,
	0x02, 0x50,
	0x03, 0x8D,
	0x04, 0x07,
	0x05, 0xFF,
	0x0A, 0x19,
	0x0D, 0x26,
	0x0E, 0x26,
	0x0F, 0x26,
	0x0C, 0x1F,
	0x11, 0x0D,
};

/*
 * The applied command [0x9F] level key
 * 01h, 10h, 12h, 13h, 18h, 19h, 20h, 21h, 22h, 23h, 28h,
 * 29h, 30h, 34h, 35h, 36h, 38h, 39h, 3Ah, 44h, 4Fh, 51h.
 */

static const unsigned char SEQ_TEST_KEY_ON_9F[] = {
	0x9F,
	0x5A, 0x5A,
};

static const unsigned char SEQ_TEST_KEY_ON_F0[] = {
	0xF0,
	0x5A, 0x5A,
};

static const unsigned char SEQ_TEST_KEY_ON_FC[] = {
	0xFC,
	0x5A, 0x5A,
};

static const unsigned char SEQ_S6D7AT0B_53[] = {
	0x53,
	0x2C,
};

static const unsigned char SEQ_S6D7AT0B_55[] = {
	0x55,
	0x00,
};

static const unsigned char SEQ_S6D7AT0B_CABC_ON[] = {
	0x55,
	0x02,
};

/* BOE panel 116A01 */
static const unsigned char SEQ_S6D7AT0B_BOE_51[] = {
	0x51,
	0x3E, 0x09	/* 1000(DEC) | BIT(0) */
};

static const unsigned char SEQ_S6D7AT0B_BOE_71[] = {
	0x71,
	0x01,
};

static const unsigned char SEQ_S6D7AT0B_BOE_73[] = {
	0x73,
	0xB0, 0x9D, 0x10, 0x01, 0x60, 0x00,
};

static const unsigned char SEQ_S6D7AT0B_BOE_B1[] = {
	0xB1,
	0x1D, 0x22, 0x11, 0x00, 0xFF,
};

static const unsigned char SEQ_S6D7AT0B_BOE_B3[] = {
	0xB3,
	0x01, 0xBE, 0x07, 0x30, 0x18, 0x0A, 0x08, 0x08, 0x07, 0x6F,
	0x00, 0xE6, 0x00, 0xE6, 0x00, 0xE6,
};

static const unsigned char SEQ_S6D7AT0B_BOE_BA[] = {
	0xBA,
	0x01, 0x00, 0x13, 0x00, 0x13, 0x00, 0x13, 0x90, 0x02, 0x58,
	0x00, 0x84, 0x08, 0x43, 0x01, 0x00, 0xC9, 0x00, 0xF0, 0x00,
	0x14, 0x00, 0x00, 0xED, 0x01, 0x10, 0xC2, 0x89, 0x50, 0x2C,
	0x99, 0x46, 0x46, 0x00, 0x01, 0xF1, 0x01, 0x01, 0x28, 0x32,
	0x38, 0x3F, 0x48, 0x50, 0x54, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0x2F, 0x48, 0x64, 0x98, 0xBF, 0xDF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
	0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const unsigned char SEQ_S6D7AT0B_BOE_BB[] = {
	0xBB,
	0x21, 0x68, 0x00, 0x00,
};

static const unsigned char SEQ_S6D7AT0B_BOE_EC[] = {
	0xEC,
	0x00, 0x00, 0x15, 0x00, 0x04, 0x06, 0x14, 0x03, 0x0C, 0x0E,
	0x10, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x15, 0x00, 0x05, 0x07, 0x14, 0x03,
	0x0D, 0x0F, 0x11, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
};

static const unsigned char SEQ_S6D7AT0B_BOE_ED[] = {
	0xED,
	0x30, 0x40, 0x12, 0x55, 0x00, 0x60, 0x80, 0x31, 0x40, 0x12,
	0x55, 0x00, 0x40, 0x80, 0x32, 0x40, 0x12, 0x55, 0x00, 0x20,
	0x80, 0x33, 0x40, 0x12, 0x55, 0x00, 0x00, 0x80, 0x10, 0x69,
	0x00, 0x00, 0x00, 0x80, 0x90, 0x10, 0x49, 0x00, 0x00, 0x00,
	0xA0, 0xA0, 0x40, 0x41, 0x93, 0x7D, 0x00, 0xE0, 0x20, 0x41,
	0x41, 0x93, 0x7D, 0x00, 0xC0, 0x20, 0x40, 0x48, 0x92, 0x08,
	0x40, 0x40, 0x80, 0x8C, 0x3F, 0x30, 0x00, 0x00, 0x00, 0x00,
};

static const unsigned char SEQ_S6D7AT0B_BOE_EE[] = {
	0xEE,
	0x33, 0x08, 0x12, 0x55, 0x00, 0x04, 0x00, 0x00, 0xE5, 0x33,
	0x09, 0x12, 0x55, 0x00, 0x04, 0x00, 0x00, 0xC6, 0x33, 0x0A,
	0x12, 0x55, 0x00, 0x04, 0x00, 0x00, 0xA7, 0x33, 0x0B, 0x12,
	0x55, 0x00, 0x04, 0x00, 0x00, 0x88, 0x33, 0x04, 0x12, 0x55,
	0x00, 0x04, 0x00, 0x00, 0x61, 0x33, 0x05, 0x12, 0x55, 0x00,
	0x04, 0x00, 0x00, 0x42, 0x33, 0x06, 0x12, 0x55, 0x00, 0x04,
	0x00, 0x00, 0x23, 0x33, 0x07, 0x12, 0x55, 0x00, 0x04, 0x00,
	0x00, 0x04, 0xFF,
};

static const unsigned char SEQ_S6D7AT0B_BOE_EF[] = {
	0xEF,
	0x00, 0xFF, 0x00, 0x00, 0x00, 0x0C, 0x40, 0x76, 0x76, 0x10,
	0x18, 0x38, 0x80, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x48,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const unsigned char SEQ_S6D7AT0B_BOE_F2[] = {
	0xF2,
	0x48, 0x22, 0x33, 0x33, 0xE3, 0x4C, 0x43, 0x00, 0x01, 0x00,
	0x00, 0x0D, 0x84, 0x00, 0x80, 0x00, 0xA6, 0x1E, 0x08, 0x03,
	0x34, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const unsigned char SEQ_S6D7AT0B_BOE_B0[] = {
	0xB0,
	0x02,
};

static const unsigned char SEQ_S6D7AT0B_BOE_F4[] = {
	0xF4,
	0x30, 0x0A, 0xB9, 0xE8, 0xE8, 0x14, 0x14, 0x09, 0xB5, 0x55,
	0x58, 0x3B, 0x10, 0x55, 0x15, 0x26, 0xFA, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
};

static const unsigned char SEQ_S6D7AT0B_BOE_F9[] = {
	0xF9,
	0x88, 0x88, 0x80, 0xC0, 0xC0, 0x04, 0x7A, 0x00, 0x0D, 0xC0,
	0x94, 0x00, 0x3C, 0xA0, 0x3C, 0xA0, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x03, 0x11, 0x00, 0x00, 0x00, 0x77, 0x00, 0x00, 0x00,
	0x00,
};

static const unsigned char SEQ_S6D7AT0B_BOE_FE[] = {
	0xFE,
	0x02, 0x00, 0x01, 0x4C, 0x00, 0x00, 0x00, 0x60, 0x00, 0x03,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const unsigned char SEQ_S6D7AT0B_BOE_C0[] = {
	0xC0,
	0x03, 0x08, 0x29, 0x01, 0x00, 0x80, 0x01, 0x24, 0x0F, 0xFF,
	0x30, 0x40, 0x86, 0xDA, 0x8E,
};

static const unsigned char SEQ_S6D7AT0B_BOE_C1[] = {
	0xC1,
	0xE5, 0xA6, 0x80, 0x25, 0x66, 0xA7, 0xA7, 0xAC, 0xB3, 0xB3,
	0xB3, 0xB3, 0xBA, 0xBD, 0xBF, 0xBF, 0xC4, 0xD6, 0xE8, 0xFF,
	0x88, 0x96, 0xA5, 0xB4, 0xBE, 0xC8, 0xCD, 0xD0, 0xD3, 0xD7,
	0xDF, 0xE6, 0xEA, 0xF0, 0xF5, 0xFF,
};

static const unsigned char SEQ_S6D7AT0B_BOE_C8[] = {
	0xC8,
	0x00, 0x24, 0x00, 0x41, 0x3A, 0x00, 0x00, 0x00, 0xB0, 0x02,
	0x00, 0x00, 0x00, 0x85, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x40, 0x4B,
};

static const unsigned char SEQ_S6D7AT0B_BOE_E8[] = {
	0xE8,
	0x01, 0x08, 0x0F, 0x07, 0x0D, 0x13, 0x13, 0x14, 0x20, 0x20,
	0x22, 0x23, 0x26, 0x25, 0x23, 0x2D, 0x32, 0x01, 0x08, 0x0F,
	0x07, 0x0D, 0x13, 0x13, 0x18, 0x20, 0x2A, 0x2A, 0x2B, 0x26,
	0x25, 0x23, 0x2D, 0x32,
};

static const unsigned char SEQ_TEST_KEY_OFF_9F[] = {
	0x9F,
	0xA5, 0xA5,
};

static const unsigned char SEQ_TEST_KEY_OFF_F0[] = {
	0xF0,
	0xA5, 0xA5,
};

static const unsigned char SEQ_TEST_KEY_OFF_FC[] = {
	0xFC,
	0xA5, 0xA5,
};

static const unsigned char SEQ_SLEEP_OUT[] = {
	0x11,
	0x00, 0x00
};

static const unsigned char SEQ_SLEEP_IN[] = {
	0x10,
	0x00, 0x00
};

static const unsigned char SEQ_DISPLAY_OFF[] = {
	0x28,
	0x00, 0x00
};

static const unsigned char SEQ_DISPLAY_ON[] = {
	0x29,
	0x00, 0x00
};


/* platform brightness <-> bl reg */
static unsigned int brightness_table[EXTEND_BRIGHTNESS + 1] = {
	0,
	13, 24, 35, 46, 57, 68, 79, 91, 102, 113, /* 1: 13 */
	124, 135, 146, 158, 169, 180, 191, 202, 213, 224,
	236, 247, 258, 269, 280, 291, 303, 314, 325, 336,
	347, 358, 370, 381, 392, 403, 414, 425, 436, 448,
	459, 470, 481, 492, 503, 515, 526, 537, 548, 559,
	570, 582, 593, 604, 615, 626, 637, 648, 660, 671,
	682, 693, 704, 715, 727, 738, 749, 760, 771, 782,
	794, 805, 816, 827, 838, 849, 860, 872, 883, 894,
	905, 916, 927, 939, 950, 961, 972, 983, 994, 1006,
	1017, 1028, 1039, 1050, 1061, 1072, 1084, 1095, 1106, 1117,
	1128, 1139, 1151, 1162, 1173, 1184, 1195, 1206, 1218, 1229,
	1240, 1251, 1262, 1273, 1284, 1296, 1307, 1318, 1329, 1340,
	1351, 1363, 1374, 1385, 1396, 1407, 1418, 1430, 1447, 1465, /* 128: 1430 */
	1483, 1501, 1519, 1537, 1555, 1572, 1590, 1608, 1626, 1644,
	1662, 1680, 1698, 1715, 1733, 1751, 1769, 1787, 1805, 1823,
	1841, 1858, 1876, 1894, 1912, 1930, 1948, 1966, 1984, 2001,
	2019, 2037, 2055, 2073, 2091, 2109, 2127, 2144, 2162, 2180,
	2198, 2216, 2234, 2252, 2270, 2287, 2305, 2323, 2341, 2359,
	2377, 2395, 2413, 2430, 2448, 2466, 2484, 2502, 2520, 2538,
	2556, 2573, 2591, 2609, 2627, 2645, 2663, 2681, 2699, 2716,
	2734, 2752, 2770, 2788, 2806, 2824, 2842, 2859, 2877, 2895,
	2913, 2931, 2949, 2967, 2985, 3002, 3020, 3038, 3056, 3074,
	3092, 3110, 3128, 3145, 3163, 3181, 3199, 3217, 3235, 3253,
	3271, 3288, 3306, 3324, 3342, 3360, 3378, 3396, 3414, 3431,
	3449, 3467, 3485, 3503, 3521, 3539, 3557, 3574, 3592, 3610,
	3628, 3646, 3664, 3682, 3700, 3700, 3700, 3700, 3700, 3700, /* 255: 3700 */
	3700, 3700, 3700, 3700, 3700, 3700, 3700, 3700, 3700, 3700,
	3700, 3700, 3700, 3700, 3700, 3700, 3700, 3700, 3700, 3700,
	3700, 3700, 3700, 3700, 3700, 3700, 3700, 3700, 3700, 3700,
	3700, 3700, 3700, 3700, 3700, 3700, 3700, 3700, 3700, 3700,
	3700, 3700, 3700, 3700, 3700, 4400,
};

#endif /* __S6D7AT0B_PARAM_H__ */
