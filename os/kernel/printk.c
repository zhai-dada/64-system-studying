#include "printk.h"
#include "lib.h"
#include "memory.h"
#include "UEFI.h"
char buf[4096] = {0};
unsigned char font_ascii[256][16]=
{
	/*	0000	*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},

	/*	0010	*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},

	/*	0020	*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},

	/*	0030	*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x00,0x10,0x10,0x00,0x00},	//33	'!'
	{0x28,0x28,0x28,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},	//	'"'
	{0x00,0x44,0x44,0x44,0xfe,0x44,0x44,0x44,0x44,0x44,0xfe,0x44,0x44,0x44,0x00,0x00},	//	'#'
	{0x10,0x3a,0x56,0x92,0x92,0x90,0x50,0x38,0x14,0x12,0x92,0x92,0xd4,0xb8,0x10,0x10},	//	'$'
	{0x62,0x92,0x94,0x94,0x68,0x08,0x10,0x10,0x20,0x2c,0x52,0x52,0x92,0x8c,0x00,0x00},	//	'%'
	{0x00,0x70,0x88,0x88,0x88,0x90,0x60,0x47,0xa2,0x92,0x8a,0x84,0x46,0x39,0x00,0x00},	//	'&'
	{0x04,0x08,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},	//	'''

	/*	0040	*/
	{0x02,0x04,0x08,0x08,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x08,0x08,0x04,0x02,0x00},	//	'('
	{0x80,0x40,0x20,0x20,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x20,0x20,0x40,0x80,0x00},	//	')'
	{0x00,0x00,0x00,0x00,0x00,0x10,0x92,0x54,0x38,0x54,0x92,0x10,0x00,0x00,0x00,0x00},	//	'*'
	{0x00,0x00,0x00,0x00,0x00,0x10,0x10,0x10,0xfe,0x10,0x10,0x10,0x00,0x00,0x00,0x00},	//	'+'
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x08,0x08,0x10},	//	','
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0x00,0x00,0x00,0x00,0x00,0x00,0x00},	//	'-'
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00},	//	'.'
	{0x02,0x02,0x04,0x04,0x08,0x08,0x08,0x10,0x10,0x20,0x20,0x40,0x40,0x40,0x80,0x80},	//	'/'
	{0x00,0x18,0x24,0x24,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x24,0x24,0x18,0x00,0x00},	//48	'0'
	{0x00,0x08,0x18,0x28,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x3e,0x00,0x00},	//	'1'

	/*	0050	*/
	{0x00,0x18,0x24,0x42,0x42,0x02,0x04,0x08,0x10,0x20,0x20,0x40,0x40,0x7e,0x00,0x00},	//	'2'
	{0x00,0x18,0x24,0x42,0x02,0x02,0x04,0x18,0x04,0x02,0x02,0x42,0x24,0x18,0x00,0x00},	//	'3'
	{0x00,0x0c,0x0c,0x0c,0x14,0x14,0x14,0x24,0x24,0x44,0x7e,0x04,0x04,0x1e,0x00,0x00},	//	'4'
	{0x00,0x7c,0x40,0x40,0x40,0x58,0x64,0x02,0x02,0x02,0x02,0x42,0x24,0x18,0x00,0x00},	//	'5'
	{0x00,0x18,0x24,0x42,0x40,0x58,0x64,0x42,0x42,0x42,0x42,0x42,0x24,0x18,0x00,0x00},	//	'6'
	{0x00,0x7e,0x42,0x42,0x04,0x04,0x08,0x08,0x08,0x10,0x10,0x10,0x10,0x38,0x00,0x00},	//	'7'
	{0x00,0x18,0x24,0x42,0x42,0x42,0x24,0x18,0x24,0x42,0x42,0x42,0x24,0x18,0x00,0x00},	//	'8'
	{0x00,0x18,0x24,0x42,0x42,0x42,0x42,0x42,0x26,0x1a,0x02,0x42,0x24,0x18,0x00,0x00},	//	'9'
	{0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00},	//58	':'
	{0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x18,0x18,0x08,0x08,0x10},	//	';'

	/*	0060	*/
	{0x00,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x00},	//	'<'
	{0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0x00,0x00,0xfe,0x00,0x00,0x00,0x00,0x00,0x00},	//	'='
	{0x00,0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x00},	//	'>'
	{0x00,0x38,0x44,0x82,0x82,0x82,0x04,0x08,0x10,0x10,0x00,0x00,0x18,0x18,0x00,0x00},	//	'?'
	{0x00,0x38,0x44,0x82,0x9a,0xaa,0xaa,0xaa,0xaa,0xaa,0x9c,0x80,0x46,0x38,0x00,0x00},	//	'@'
	{0x00,0x18,0x18,0x18,0x18,0x24,0x24,0x24,0x24,0x7e,0x42,0x42,0x42,0xe7,0x00,0x00},	//65	'A'
	{0x00,0xf0,0x48,0x44,0x44,0x44,0x48,0x78,0x44,0x42,0x42,0x42,0x44,0xf8,0x00,0x00},	//	'B'
	{0x00,0x3a,0x46,0x42,0x82,0x80,0x80,0x80,0x80,0x80,0x82,0x42,0x44,0x38,0x00,0x00},	//	'C'
	{0x00,0xf8,0x44,0x44,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x44,0x44,0xf8,0x00,0x00},	//	'D'
	{0x00,0xfe,0x42,0x42,0x40,0x40,0x44,0x7c,0x44,0x40,0x40,0x42,0x42,0xfe,0x00,0x00},	//	'E'

	/*	0070	*/
	{0x00,0xfe,0x42,0x42,0x40,0x40,0x44,0x7c,0x44,0x44,0x40,0x40,0x40,0xf0,0x00,0x00},	//	'F'
	{0x00,0x3a,0x46,0x42,0x82,0x80,0x80,0x9e,0x82,0x82,0x82,0x42,0x46,0x38,0x00,0x00},	//	'G'
	{0x00,0xe7,0x42,0x42,0x42,0x42,0x42,0x7e,0x42,0x42,0x42,0x42,0x42,0xe7,0x00,0x00},	//	'H'
	{0x00,0x7c,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x7c,0x00,0x00},	//	'I'
	{0x00,0x1f,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x84,0x48,0x30,0x00},	//	'J'
	{0x00,0xe7,0x42,0x44,0x48,0x50,0x50,0x60,0x50,0x50,0x48,0x44,0x42,0xe7,0x00,0x00},	//	'K'
	{0x00,0xf0,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x42,0x42,0xfe,0x00,0x00},	//	'L'
	{0x00,0xc3,0x42,0x66,0x66,0x66,0x5a,0x5a,0x5a,0x42,0x42,0x42,0x42,0xe7,0x00,0x00},	//	'M'
	{0x00,0xc7,0x42,0x62,0x62,0x52,0x52,0x52,0x4a,0x4a,0x4a,0x46,0x46,0xe2,0x00,0x00},	//	'N'
	{0x00,0x38,0x44,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x44,0x38,0x00,0x00},	//	'O'

	/*	0080	*/
	{0x00,0xf8,0x44,0x42,0x42,0x42,0x44,0x78,0x40,0x40,0x40,0x40,0x40,0xf0,0x00,0x00},	//	'P'
	{0x00,0x38,0x44,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x92,0x8a,0x44,0x3a,0x00,0x00},	//	'Q'
	{0x00,0xfc,0x42,0x42,0x42,0x42,0x7c,0x44,0x42,0x42,0x42,0x42,0x42,0xe7,0x00,0x00},	//	'R'
	{0x00,0x3a,0x46,0x82,0x82,0x80,0x40,0x38,0x04,0x02,0x82,0x82,0xc4,0xb8,0x00,0x00},	//	'S'
	{0x00,0xfe,0x92,0x92,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x7c,0x00,0x00},	//	'T'
	{0x00,0xe7,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x24,0x3c,0x00,0x00},	//	'U'
	{0x00,0xe7,0x42,0x42,0x42,0x42,0x24,0x24,0x24,0x24,0x18,0x18,0x18,0x18,0x00,0x00},	//	'V'
	{0x00,0xe7,0x42,0x42,0x42,0x5a,0x5a,0x5a,0x5a,0x24,0x24,0x24,0x24,0x24,0x00,0x00},	//	'W'
	{0x00,0xe7,0x42,0x42,0x24,0x24,0x24,0x18,0x24,0x24,0x24,0x42,0x42,0xe7,0x00,0x00},	//	'X'
	{0x00,0xee,0x44,0x44,0x44,0x28,0x28,0x28,0x10,0x10,0x10,0x10,0x10,0x7c,0x00,0x00},	//	'Y'

	/*	0090	*/
	{0x00,0xfe,0x84,0x84,0x08,0x08,0x10,0x10,0x20,0x20,0x40,0x42,0x82,0xfe,0x00,0x00},	//	'Z'
	{0x00,0x3e,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x3e,0x00},	//91	'['
	{0x80,0x80,0x40,0x40,0x20,0x20,0x20,0x10,0x10,0x08,0x08,0x04,0x04,0x04,0x02,0x02},	//	'\'
	{0x00,0x7c,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x7c,0x00},	//	']'
	{0x00,0x10,0x28,0x44,0x82,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},	//	'^'
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0x00},	//	'_'
	{0x10,0x08,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},	//	'`'
	{0x00,0x00,0x00,0x00,0x00,0x70,0x08,0x04,0x3c,0x44,0x84,0x84,0x8c,0x76,0x00,0x00},	//97	'a'
	{0xc0,0x40,0x40,0x40,0x40,0x58,0x64,0x42,0x42,0x42,0x42,0x42,0x64,0x58,0x00,0x00},	//	'b'
	{0x00,0x00,0x00,0x00,0x00,0x30,0x4c,0x84,0x84,0x80,0x80,0x82,0x44,0x38,0x00,0x00},	//	'c'

	/*	0100	*/
	{0x0c,0x04,0x04,0x04,0x04,0x34,0x4c,0x84,0x84,0x84,0x84,0x84,0x4c,0x36,0x00,0x00},	//	'd'
	{0x00,0x00,0x00,0x00,0x00,0x38,0x44,0x82,0x82,0xfc,0x80,0x82,0x42,0x3c,0x00,0x00},	//	'e'
	{0x0e,0x10,0x10,0x10,0x10,0x7c,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x7c,0x00,0x00},	//	'f'
	{0x00,0x00,0x00,0x00,0x00,0x36,0x4c,0x84,0x84,0x84,0x84,0x4c,0x34,0x04,0x04,0x38},	//	'g'
	{0xc0,0x40,0x40,0x40,0x40,0x58,0x64,0x42,0x42,0x42,0x42,0x42,0x42,0xe3,0x00,0x00},	//	'h'
	{0x00,0x10,0x10,0x00,0x00,0x30,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x38,0x00,0x00},	//	'i'
	{0x00,0x04,0x04,0x00,0x00,0x0c,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x08,0x08,0x30},	//	'j'
	{0xc0,0x40,0x40,0x40,0x40,0x4e,0x44,0x48,0x50,0x60,0x50,0x48,0x44,0xe6,0x00,0x00},	//	'k'
	{0x30,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x38,0x00,0x00},	//	'l'
	{0x00,0x00,0x00,0x00,0x00,0xf6,0x49,0x49,0x49,0x49,0x49,0x49,0x49,0xdb,0x00,0x00},	//	'm'

	/*	0110	*/
	{0x00,0x00,0x00,0x00,0x00,0xd8,0x64,0x42,0x42,0x42,0x42,0x42,0x42,0xe3,0x00,0x00},	//	'n'
	{0x00,0x00,0x00,0x00,0x00,0x38,0x44,0x82,0x82,0x82,0x82,0x82,0x44,0x38,0x00,0x00},	//	'o'
	{0x00,0x00,0x00,0x00,0xd8,0x64,0x42,0x42,0x42,0x42,0x42,0x64,0x58,0x40,0x40,0xe0},	//	'p'
	{0x00,0x00,0x00,0x00,0x34,0x4c,0x84,0x84,0x84,0x84,0x84,0x4c,0x34,0x04,0x04,0x0e},	//	'q'
	{0x00,0x00,0x00,0x00,0x00,0xdc,0x62,0x42,0x40,0x40,0x40,0x40,0x40,0xe0,0x00,0x00},	//	'r'
	{0x00,0x00,0x00,0x00,0x00,0x7a,0x86,0x82,0xc0,0x38,0x06,0x82,0xc2,0xbc,0x00,0x00},	//	's'
	{0x00,0x00,0x10,0x10,0x10,0x7c,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x0e,0x00,0x00},	//	't'
	{0x00,0x00,0x00,0x00,0x00,0xc6,0x42,0x42,0x42,0x42,0x42,0x42,0x46,0x3b,0x00,0x00},	//	'u'
	{0x00,0x00,0x00,0x00,0x00,0xe7,0x42,0x42,0x42,0x24,0x24,0x24,0x18,0x18,0x00,0x00},	//	'v'
	{0x00,0x00,0x00,0x00,0x00,0xe7,0x42,0x42,0x5a,0x5a,0x5a,0x24,0x24,0x24,0x00,0x00},	//	'w'

	/*	0120	*/
	{0x00,0x00,0x00,0x00,0x00,0xc6,0x44,0x28,0x28,0x10,0x28,0x28,0x44,0xc6,0x00,0x00},	//	'x'
	{0x00,0x00,0x00,0x00,0x00,0xe7,0x42,0x42,0x24,0x24,0x24,0x18,0x18,0x10,0x10,0x60},	//	'y'
	{0x00,0x00,0x00,0x00,0x00,0xfe,0x82,0x84,0x08,0x10,0x20,0x42,0x82,0xfe,0x00,0x00},	//	'z'
	{0x00,0x06,0x08,0x10,0x10,0x10,0x10,0x60,0x10,0x10,0x10,0x10,0x08,0x06,0x00,0x00},	//	'{'
	{0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10},	//	'|'
	{0x00,0x60,0x10,0x08,0x08,0x08,0x08,0x06,0x08,0x08,0x08,0x08,0x10,0x60,0x00,0x00},	//	'}'
	{0x00,0x72,0x8c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},	//	'~'
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},

	/*	0130	*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},


	/*	0140	*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},

	/*	0150	*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},

	/*	0160	*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},

	/*	0170	*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},

	/*	0180	*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},

	/*	0190	*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},

	/*	0200	*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},

	/*	0210	*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},

	/*	0220	*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},

	/*	0230	*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},

	/*	0240	*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},

	/*	0250~0255	*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
};

struct position Pos;
extern struct KERNEL_BOOT_PARAMETER_INFORMATION *boot_para_info;
void init_printk(void)
{
    int *addr = (int *)0xffff800003000000;
	#ifdef BOCHS
    Pos.XResolution = 1440;
    Pos.YResolution = 900;
	#else
	Pos.XResolution = boot_para_info->Graphics_Info.HorizontalResolution;
	Pos.YResolution = boot_para_info->Graphics_Info.VerticalResolution;
	#endif
    Pos.XPosition = 0;
    Pos.YPosition = 0;

    Pos.XCharSize = 8;
    Pos.YCharSize = 16;

    Pos.FB_addr = addr;
    Pos.FB_length = boot_para_info->Graphics_Info.FrameBufferSize;
}
void VBE_buffer_init()
{
	unsigned long i = 0;
	unsigned long* tmp = NULL;
	unsigned long* tmp1 = NULL;
	unsigned long* virtual = NULL;
	unsigned long phy = 0;
	#ifdef BOCHS
	unsigned int* FB_addr = (unsigned int*)P_TO_V(0xe0000000);
	#else
	unsigned int* FB_addr = (unsigned int*)P_TO_V(boot_para_info->Graphics_Info.FrameBufferBase);
	#endif
	GLOBAL_CR3 = get_gdt();
	tmp = (unsigned long*)(((unsigned long)P_TO_V((unsigned long)GLOBAL_CR3 & (~0xfffUL))) + (((unsigned long)FB_addr >> PAGE_GDT_SHIFT) & 0x1ff) * 8);
	if(*tmp == 0)
	{
		virtual = (unsigned long*)kmalloc(PAGE_4K_SIZE, 0);
		memset(virtual, 0, PAGE_4K_SIZE);
		set_pml4t(tmp, make_pml4t(V_TO_P(virtual), PAGE_KERNEL_GDT | PAGE_USER_GDT));
	}
	tmp = (unsigned long*)(((unsigned long)P_TO_V((unsigned long)(*tmp & (~0xfffUL)) & (~0xfffUL))) + (((unsigned long)FB_addr >> PAGE_1G_SHIFT) & 0x1ff) * 8);
	if(*tmp == 0)
	{
		virtual = (unsigned long*)kmalloc(PAGE_4K_SIZE, 0);
		memset(virtual, 0, PAGE_4K_SIZE);
		set_pdpt(tmp, make_pdpt(V_TO_P(virtual), PAGE_KERNEL_DIR | PAGE_USER_DIR));
	}
	for(i = 0; i < Pos.FB_length; i = i + PAGE_2M_SIZE)
	{
		tmp1 = (unsigned long*)(((unsigned long)P_TO_V((unsigned long)(*tmp & (~0xfffUL)) & (~0xfffUL))) + ((((unsigned long)FB_addr + i) >> PAGE_2M_SHIFT) & 0x1ff) * 8);
		#ifdef BOCHS
		phy = 0xe0000000 + i;
		#else
		phy = boot_para_info->Graphics_Info.FrameBufferBase + i;
		#endif
		set_pdt(tmp1, make_pdt(phy, PAGE_KERNEL_PAGE | PAGE_PWT | PAGE_PCD | PAGE_USER_PAGE));
	}
	#ifdef BOCHS
	Pos.FB_addr = (unsigned int*)P_TO_V(0xe0000000);
	#else
	Pos.FB_addr = (unsigned int*)P_TO_V(boot_para_info->Graphics_Info.FrameBufferBase);
	#endif
	flush_tlb();
	return;
}

int skip_atoi(const char **s)
{
    int i = 0;

    while (is_digit(**s))
    {
        i = i * 10 + *((*s)++) - '0';
    }
    return i;
}

void putchar(unsigned int *fb, int Xsize, int x, int y, unsigned int FRcolor, unsigned int BKcolor, unsigned char font)
{
    int i = 0, j = 0;
    unsigned int *addr = NULL;
    unsigned char *fontp = NULL;
    int testval = 0;
    fontp = font_ascii[font];

    for (i = 0; i < 16; i++)
    {
        addr = fb + Xsize * (y + i) + x;
        testval = 0x100;
        for (j = 0; j < 8; j++)
        {
            testval = testval >> 1;
            if (*fontp & testval)
            {
                *addr = FRcolor;
            }
            else
            {
                *addr = BKcolor;
            }
            addr++;
        }
        fontp++;
    }
	return;
}

static char * number(char * str, long num, int base, int size, int precision, int type)
{
	char c, sign, tmp[50];
	const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i;

	if (type & SMALL) digits = "0123456789abcdefghijklmnopqrstuvwxyz";
	if (type & LEFT) type &= ~ZEROPAD;
	if (base < 2 || base > 36)
		return 0;
	c = (type & ZEROPAD) ? '0' : ' ' ;
	sign = 0;
	if (type & SIGN && num < 0) {
		sign = '-';
		num = -num;
	} else
		sign = (type & PLUS) ? '+' : ((type & SPACE) ? ' ' : 0);
	if (sign) size--;
	if (type & SPECIAL)
		if (base == 16) size -= 2;
		else if (base == 8) size--;
	i = 0;
	if (num == 0)
		tmp[i++]='0';
	else while (num!=0)
		tmp[i++]=digits[do_div(num,base)];
	if (i > precision) precision=i;
	size -= precision;
	if (!(type & (ZEROPAD + LEFT)))
		while(size-- > 0)
			*str++ = ' ';
	if (sign)
		*str++ = sign;
	if (type & SPECIAL)
		if (base == 8)
			*str++ = '0';
		else if (base==16) 
		{
			*str++ = '0';
			*str++ = digits[33];
		}
	if (!(type & LEFT))
		while(size-- > 0)
			*str++ = c;

	while(i < precision--)
		*str++ = '0';
	while(i-- > 0)
		*str++ = tmp[i];
	while(size-- > 0)
		*str++ = ' ';
	return str;
}

int vsprintf(char * buf,const char *fmt, va_list args)
{
	char * str,*s;
	int flags;
	int field_width;
	int precision;
	int len,i;

	int qualifier;		/* 'h', 'l', 'L' or 'Z' for integer fields */

	for(str = buf; *fmt; fmt++)
	{

		if(*fmt != '%')
		{
			*str++ = *fmt;
			continue;
		}
		flags = 0;
		repeat:
			fmt++;
			switch(*fmt)
			{
				case '-':flags |= LEFT;	
				goto repeat;
				case '+':flags |= PLUS;	
				goto repeat;
				case ' ':flags |= SPACE;	
				goto repeat;
				case '#':flags |= SPECIAL;	
				goto repeat;
				case '0':flags |= ZEROPAD;	
				goto repeat;
			}

			field_width = -1;
			if(is_digit(*fmt))
				field_width = skip_atoi(&fmt);
			else if(*fmt == '*')
			{
				fmt++;
				field_width = va_arg(args, int);
				if(field_width < 0)
				{
					field_width = -field_width;
					flags |= LEFT;
				}
			}

			precision = -1;
			if(*fmt == '.')
			{
				fmt++;
				if(is_digit(*fmt))
					precision = skip_atoi(&fmt);
				else if(*fmt == '*')
				{	
					fmt++;
					precision = va_arg(args, int);
				}
				if(precision < 0)
					precision = 0;
			}
			
			qualifier = -1;
			if(*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'Z')
			{	
				qualifier = *fmt;
				fmt++;
			}
							
			switch(*fmt)
			{
				case 'c':

					if(!(flags & LEFT))
						while(--field_width > 0)
							*str++ = ' ';
					*str++ = (unsigned char)va_arg(args, int);
					while(--field_width > 0)
						*str++ = ' ';
					break;

				case 's':
				
					s = va_arg(args, char *);
					if(!s)
						s = '\0';
					len = strlen(s);
					if(precision < 0)
						precision = len;
					else if(len > precision)
						len = precision;
					
					if(!(flags & LEFT))
						while(len < field_width--)
							*str++ = ' ';
					for(i = 0; i < len; i++)
						*str++ = *s++;
					while(len < field_width--)
						*str++ = ' ';
					break;

				case 'o':
					if(qualifier == 'l')
						str = number(str, va_arg(args,unsigned long), 8, field_width, precision, flags);
					else
						str = number(str, va_arg(args,unsigned int), 8, field_width, precision, flags);
					break;

				case 'p':

					if(field_width == -1)
					{
						field_width = 2 * sizeof(void *);
						flags |= ZEROPAD;
					}

					str = number(str, (unsigned long)va_arg(args,void *), 16, field_width, precision, flags);
					break;

				case 'x':

					flags |= SMALL;

				case 'X':

					if(qualifier == 'l')
						str = number(str, va_arg(args,unsigned long), 16, field_width, precision, flags);
					else
						str = number(str, va_arg(args,unsigned int), 16, field_width, precision, flags);
					break;

				case 'd':
				case 'i':
					flags |= SIGN;
				case 'u':

					if(qualifier == 'l')
						str = number(str, va_arg(args,unsigned long), 10, field_width, precision, flags);
					else
						str = number(str, va_arg(args,unsigned int), 10, field_width, precision, flags);
					break;

				case 'n':
					
					if(qualifier == 'l')
					{
						long *ip = va_arg(args, long *);
						*ip = (str - buf);
					}
					else
					{
						int *ip = va_arg(args, int *);
						*ip = (str - buf);
					}
					break;

				case '%':
					
					*str++ = '%';
					break;

				default:

					*str++ = '%';	
					if(*fmt)
						*str++ = *fmt;
					else
						fmt--;
					break;
			}

	}
	*str = '\0';
	return (str - buf);
}

int color_printk(unsigned int FRcolor, unsigned int BKcolor, const char * fmt,...)
{
	int i = 0;
	int count = 0;
	int line = 0;
	unsigned long flags = 0;
	local_irq_save(flags);
	spinlock_lock(&Pos.printk_lock);
	va_list args;
	va_start(args, fmt);

	i = vsprintf(buf, fmt, args);

	va_end(args);
	for(count = 0;count < i || line;count++)
	{
		////	add \n \b \t
		if(line > 0)
		{
			count--;
			goto tab;
		}
		if((unsigned char)*(buf + count) == '\n')
		{
			Pos.YPosition++;
			Pos.XPosition = 0;
		}
		else if((unsigned char)*(buf + count) == '\b')
		{
			Pos.XPosition--;
			if(Pos.XPosition < 0)
			{
				Pos.XPosition = (Pos.XResolution / Pos.XCharSize - 1) * Pos.XCharSize;
				Pos.YPosition--;
				if(Pos.YPosition < 0)
					Pos.YPosition = (Pos.YResolution / Pos.YCharSize - 1) * Pos.YCharSize;
			}	
			putchar(Pos.FB_addr, Pos.XResolution, Pos.XPosition * Pos.XCharSize, Pos.YPosition * Pos.YCharSize, FRcolor, BKcolor, ' ');	
		}
		else if((unsigned char)*(buf + count) == '\t')
		{
			line = ((Pos.XPosition + 8) & ~(8 - 1)) - Pos.XPosition;

tab:
			line--;
			putchar(Pos.FB_addr, Pos.XResolution, Pos.XPosition * Pos.XCharSize, Pos.YPosition * Pos.YCharSize, FRcolor, BKcolor, ' ');	
			Pos.XPosition++;
		}
		else
		{
			putchar(Pos.FB_addr, Pos.XResolution, Pos.XPosition * Pos.XCharSize, Pos.YPosition * Pos.YCharSize, FRcolor, BKcolor, (unsigned char)*(buf + count));
			Pos.XPosition++;
		}


		if(Pos.XPosition >= (Pos.XResolution / Pos.XCharSize))
		{
			Pos.YPosition++;
			Pos.XPosition = 0;
		}
		if(Pos.YPosition >= (Pos.YResolution / Pos.YCharSize))
		{
            // roll_screen();
			// Pos.YPosition--;
			Pos.YPosition = 0;
			memset(Pos.FB_addr, 0, Pos.FB_length);
		}
	}
	spinlock_unlock(&Pos.printk_lock);
	local_irq_restore(flags);
	return i;
}
void roll_screen(void)
{
    char* i = (char*)Pos.FB_addr;
    char* j = (char*)i + Pos.XResolution * Pos.YCharSize * 4;
    char* res = (char*)Pos.FB_addr + Pos.XResolution * (Pos.YResolution / Pos.YCharSize - 1) * Pos.YCharSize * 4;
    int count = (int)(res - i);
    char* all = (char*)Pos.FB_addr + Pos.FB_length;
    char black = 0x00;
    asm volatile
    (
        "cld 		\n"
        "rep movsb 	\n"
        :
        :"D"(i), "S"(j), "c"(count)
        :"memory"
    );
	io_mfence();
    for(j = res; j < all; j++)
    {
        *j = 0;
    }
	io_mfence();
	return;
}
