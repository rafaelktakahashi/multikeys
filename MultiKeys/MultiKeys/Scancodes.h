#pragma once


// This header defines macros for common scancodes that always correspond to a certain function,
// such as the shift and control keys. Most physical keys change their function depending on
// keyboard layout.

// Importantly, these definitions do not take into account the 0xe0 prefix that many keys use.


#define SCANCODE_ERROR_0				0x00		// Represents no scancode
#define SCANCODE_ERROR					0xff
#define SCANCODE_INTERNAL_FAILURE		0xfd
#define SCANCODE_ESCAPE					0x01
#define SCANCODE_TAB					0x0f
#define SCANCODE_ENTER					0x0c
#define SCANCODE_LEFT_SHIFT				0x2a
#define SCANCODE_RIGHT_SHIFT			0x36
#define SCANCODE_CONTROL				0x1d		// Right variant has prefix 0xe0
#define SCANCODE_ALT					0x38		// Right variant has prefix 0xe0
#define SCANCODE_SPACE					0x39
#define SCANCODE_CAPS_LOCK				0x3a
#define SCANCODE_NUM_LOCK				0x45
#define SCANCODE_SCROLL_LOCK			0x46
#define SCANCODE_PRINT_SCREEN			0x37		// With prefix 0xe0
#define SCANCODE_SYS_REQ				0x54		// Usually input by pressing Alt+PrintScreen
#define SCANCODE_LEFT_WINDOWS			0x5b		// With prefix 0xe0
#define SCANCODE_RIGHT_WINDOWS			0x5c		// With prefix 0xe0
#define SCANCODE_APPS					0x5d		// With prefix 0xe0

#define SCANCODE_HOME					0x47
#define SCANCODE_ARROW_UP				0x48
#define SCANCODE_PAGE_UP				0x49
#define SCANCODE_ARROW_LEFT				0x4b
#define SCANCODE_KEYPAD_CENTER			0x4c		// Normally corresponds to no action
#define SCANCODE_ARROW_RIGHT			0x4d
#define SCANCODE_END					0x4f
#define SCANCODE_ARROW_DOWN				0x50
#define SCANCODE_PAGE_DOWN				0x51
#define	SCANCODE_INSERT					0x52
#define SCANCODE_DELETE					0x53

#define SCANCODE_NUMPAD_1				0x4f
#define SCANCODE_NUMPAD_2				0x50
#define SCANCODE_NUMPAD_3				0x51
#define SCANCODE_NUMPAD_4				0x4b
#define SCANCODE_NUMPAD_5				0x4c
#define SCANCODE_NUMPAD_6				0x4d
#define SCANCODE_NUMPAD_7				0x47
#define SCANCODE_NUMPAD_8				0x48
#define SCANCODE_NUMPAD_9				0x49
#define	SCANCODE_NUMPAD_0				0x52
#define SCANCODE_NUMPAD_SEPARATOR		0x53

#define SCANCODE_F1						0x3b
#define SCANCODE_F2						0x3c
#define SCANCODE_F3						0x3d
#define SCANCODE_F4						0x3e
#define SCANCODE_F5						0x3f
#define SCANCODE_F6						0x40
#define SCANCODE_F7						0x41
#define SCANCODE_F8						0x42
#define SCANCODE_F9						0x43
#define SCANCODE_F10					0x44
#define SCANCODE_F11					0x57
#define SCANCODE_F12					0x58

/*-- The following are present in almost no keyboard: --*/


#define SCANCODE_F13					0x5b
#define SCANCODE_F14					0x5c
#define SCANCODE_F15					0x5d
#define SCANCODE_F16					0x63
#define SCANCODE_F17					0x64
#define SCANCODE_F18					0x65
#define SCANCODE_F19					0x66
#define SCANCODE_F20					0x67
#define SCANCODE_F21					0x68
#define SCANCODE_F22					0x69
#define SCANCODE_F23					0x6a
#define SCANCODE_F24					0x6b


/*-- Keys present in the Japanese layout: --*/


#define SCANCODE_HANKAKU				0x29		// Hankaku / Zenkaku is a toggle
#define SCANCODE_EISUU					0x3a		// Eisuu occupies the same space as CapsLock
#define SCANCODE_HENKAN					0x79		// Henkan / Zenkouho
#define SCANCODE_MUHENKAN				0x7b
#define SCANCODE_KATAKANA_HIRAGANA		0x70


/*-- Keys present in the Korean layout --*/


#define SCANCODE_HANJA					0xf1
#define SCANCODE_HAN_YEONG				0xf2		// Hangeul / Yeongja