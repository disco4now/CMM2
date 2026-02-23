/**
  ******************************************************************************
  * @file    USB_Host/HID_Standalone/Src/keyboard.c
  * @author  MCD Application Team
  * @version V1.2.0
  * @date    29-December-2017
  * @brief   This file implements the HID keyboard functions
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V.
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------ */
#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"
/* Private typedef ----------------------------------------------------------- */
/* Private define ------------------------------------------------------------ */
#define KYBRD_FIRST_COLUMN               (uint16_t)7
#define KYBRD_LAST_COLUMN                (uint16_t)479
#define KYBRD_FIRST_LINE                 (uint8_t) 70
#define SMALL_FONT_COLUMN_WIDTH                    8
#define SMALL_FONT_LINE_WIDTH                      15
#define KYBRD_LAST_LINE                  (uint16_t)200
/* Private macro ------------------------------------------------------------- */
HID_DEMO_StateMachine hid_demo;
uint8_t prev_select = 0;
uint32_t sendlights=0;
uint8_t KeybrdCharYpos = 0;
uint16_t KeybrdCharXpos = 0;
uint16_t CurrentLastXpos[KYBRD_LAST_LINE] = { 0 };
extern volatile char ConsoleRxBuf[CONSOLE_RX_BUF_SIZE];
uint32_t repeattime;
extern volatile int ConsoleRxBufHead;
extern volatile int ConsoleRxBufTail;
extern volatile int keyboardseen;
extern char *KeyInterrupt;
extern volatile int Keycomplete;
extern int keyselect;
typedef enum
{
    USB_HID_KEYBOARD_KEYPAD_RESERVED_NO_EVENT_INDICATED                      = 0x00,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_ERROR_ROLL_OVER                         = 0x01,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_POST_FAIL                               = 0x02,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_ERROR_UNDEFINED                         = 0x03,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_A                                       = 0x04,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_B                                       = 0x05,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_C                                       = 0x06,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_D                                       = 0x07,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_E                                       = 0x08,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F                                       = 0x09,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_G                                       = 0x0A,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_H                                       = 0x0B,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_I                                       = 0x0C,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_J                                       = 0x0D,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_K                                       = 0x0E,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_L                                       = 0x0F,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_M                                       = 0x10,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_N                                       = 0x11,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_O                                       = 0x12,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_P                                       = 0x13,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Q                                       = 0x14,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_R                                       = 0x15,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_S                                       = 0x16,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_T                                       = 0x17,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_U                                       = 0x18,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_V                                       = 0x19,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_W                                       = 0x1A,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_X                                       = 0x1B,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Y                                       = 0x1C,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Z                                       = 0x1D,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_1_AND_EXCLAMATION_POINT                 = 0x1E,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_2_AND_AT                                = 0x1F,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_3_AND_HASH                              = 0x20,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_4_AND_DOLLAR                            = 0x21,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_5_AND_PERCENT                           = 0x22,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_6_AND_CARROT                            = 0x23,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_7_AND_AMPERSAND                         = 0x24,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_8_AND_ASTERISK                          = 0x25,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_9_AND_OPEN_PARENTHESIS                  = 0x26,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_0_AND_CLOSE_PARENTHESIS                 = 0x27,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RETURN_ENTER                            = 0x28,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_ESCAPE                                  = 0x29,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_DELETE                                  = 0x2A,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_TAB                                     = 0x2B,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SPACEBAR                                = 0x2C,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_MINUS_AND_UNDERSCORE                    = 0x2D,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_EQUAL_AND_PLUS                          = 0x2E,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_OPEN_BRACKET_AND_OPEN_CURLY_BRACE       = 0x2F,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CLOSE_BRACKET_AND_CLOSE_CURLY_BRACE     = 0x30,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_BACK_SLASH_AND_PIPE                     = 0x31,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_NON_US_HASH_AND_TILDE                   = 0x32,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SEMICOLON_AND_COLON                     = 0x33,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_APOSTROPHE_AND_QUOTE                    = 0x34,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_GRAVE_ACCENT_AND_TILDE                  = 0x35,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_COMMA_AND_LESS_THAN                     = 0x36,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PERIOD_AND_GREATER_THAN                 = 0x37,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_FORWARD_SLASH_AND_QUESTION_MARK         = 0x38,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CAPS_LOCK                               = 0x39,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F1                                      = 0x3A,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F2                                      = 0x3B,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F3                                      = 0x3C,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F4                                      = 0x3D,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F5                                      = 0x3E,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F6                                      = 0x3F,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F7                                      = 0x40,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F8                                      = 0x41,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F9                                      = 0x42,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F10                                     = 0x43,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F11                                     = 0x44,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F12                                     = 0x45,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PRINT_SCREEN                            = 0x46,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SCROLL_LOCK                             = 0x47,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PAUSE                                   = 0x48,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_INSERT                                  = 0x49,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_HOME                                    = 0x4A,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PAGE_UP                                 = 0x4B,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_DELETE_FORWARD                          = 0x4C,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_END                                     = 0x4D,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PAGE_DOWN                               = 0x4E,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RIGHT_ARROW                             = 0x4F,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LEFT_ARROW                              = 0x50,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_DOWN_ARROW                              = 0x51,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_UP_ARROW                                = 0x52,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_NUM_LOCK_AND_CLEAR                        = 0x53,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_BACK_SLASH                                = 0x54,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_ASTERISK                                  = 0x55,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_MINUS                                     = 0x56,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_PLUS                                      = 0x57,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_ENTER                                     = 0x58,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_1_AND_END                                 = 0x59,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_2_AND_DOWN_ARROW                          = 0x5A,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_3_AND_PAGE_DOWN                           = 0x5B,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_4_AND_LEFT_ARROW                          = 0x5C,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_5                                         = 0x5D,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_6_AND_RIGHT_ARROW                         = 0x5E,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_7_AND_HOME                                = 0x5F,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_8_AND_UP_ARROW                            = 0x60,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_9_AND_PAGE_UP                             = 0x61,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_0_AND_INSERT                              = 0x62,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_PERIOD_AND_DELETE                         = 0x63,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_NON_US_FORWARD_SLASH_AND_PIPE           = 0x64,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_APPLICATION                             = 0x65,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_POWER                                   = 0x66,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_EQUAL_SIZE                              = 0x67,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F13                                     = 0x68,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F14                                     = 0x69,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F15                                     = 0x6A,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F16                                     = 0x6B,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F17                                     = 0x6C,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F18                                     = 0x6D,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F19                                     = 0x6E,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F20                                     = 0x6F,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F21                                     = 0x70,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F22                                     = 0x71,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F23                                     = 0x72,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F24                                     = 0x73,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_EXECUTE                                 = 0x74,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_HELP                                    = 0x75,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_MENU                                    = 0x76,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SELECT                                  = 0x77,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_STOP                                    = 0x78,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_AGAIN                                   = 0x79,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_UNDO                                    = 0x7A,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CUT                                     = 0x7B,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_COPY                                    = 0x7C,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PASTE                                   = 0x7D,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_FIND                                    = 0x7E,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_MUTE                                    = 0x7F,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_VOLUME_UP                               = 0x80,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_VOLUME_DOWN                             = 0x81,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LOCKING_CAPS_LOCK                       = 0x82,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LOCKING_NUM_LOCK                        = 0x83,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LOCKING_SCROLL_LOCK                     = 0x84,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_COMMA                                     = 0x85,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_EQUAL_SIGN                                = 0x86,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_INTERNATIONAL1                          = 0x87,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_INTERNATIONAL2                          = 0x88,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_INTERNATIONAL3                          = 0x89,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_INTERNATIONAL4                          = 0x8A,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_INTERNATIONAL5                          = 0x8B,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_INTERNATIONAL6                          = 0x8C,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_INTERNATIONAL7                          = 0x8D,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_INTERNATIONAL8                          = 0x8E,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_INTERNATIONAL9                          = 0x8F,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LANG1                                   = 0x90,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LANG2                                   = 0x91,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LANG3                                   = 0x92,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LANG4                                   = 0x93,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LANG5                                   = 0x94,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LANG6                                   = 0x95,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LANG7                                   = 0x96,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LANG8                                   = 0x97,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LANG9                                   = 0x98,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_ALTERNATE_ERASE                         = 0x99,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SYS_REQ_ATTENTION                       = 0x9A,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CANCEL                                  = 0x9B,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CLEAR                                   = 0x9C,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PRIOR                                   = 0x9D,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RETURN                                  = 0x9E,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SEPARATOR                               = 0x9F,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_OUT                                     = 0xA0,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_OPER                                    = 0xA1,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CLEAR_AGAIN                             = 0xA2,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CR_SEL_PROPS                            = 0xA3,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_EX_SEL                                  = 0xA4,
    /* Reserved                                                                         = 0xA5-0xAF */
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_00                                        = 0xB0,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_000                                       = 0xB1,
    USB_HID_KEYBOARD_KEYPAD_THOUSANDS_SEPARATOR                              = 0xB2,
    USB_HID_KEYBOARD_KEYPAD_DECIMAL_SEPARATOR                                = 0xB3,
    USB_HID_KEYBOARD_KEYPAD_CURRENCY_UNIT                                    = 0xB4,
    USB_HID_KEYBOARD_KEYPAD_CURRENTY_SUB_UNIT                                = 0xB5,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_OPEN_PARENTHESIS                          = 0xB6,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_CLOSE_PARENTHESIS                         = 0xB7,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_OPEN_CURLY_BRACE                          = 0xB8,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_CLOSE_CURLY_BRACE                         = 0xB9,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_TAB                                       = 0xBA,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_BACKSPACE                                 = 0xBB,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_A                                         = 0xBC,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_B                                         = 0xBD,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_C                                         = 0xBE,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_D                                         = 0xBF,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_E                                         = 0xC0,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_F                                         = 0xC1,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_XOR                                       = 0xC2,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_CARROT                                    = 0xC3,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_PERCENT_SIGN                              = 0xC4,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_LESS_THAN                                 = 0xC5,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_GREATER_THAN                              = 0xC6,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_AMPERSAND                                 = 0xC7,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_DOUBLE_AMPERSAND                          = 0xC8,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_PIPE                                      = 0xC9,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_DOUBLE_PIPE                               = 0xCA,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_COLON                                     = 0xCB,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_HASH                                      = 0xCC,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_SPACE                                     = 0xCD,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_AT                                        = 0xCE,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_EXCLAMATION_POINT                         = 0xCF,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_MEMORY_STORE                              = 0xD0,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_MEMORY_RECALL                             = 0xD1,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_MEMORY_CLEAR                              = 0xD2,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_MEMORY_ADD                                = 0xD3,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_MEMORY_SUBTRACT                           = 0xD4,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_MEMORY_MULTIPLY                           = 0xD5,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_MEMORY_DIVIDE                             = 0xD6,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_PLUS_MINUS                                = 0xD7,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_CLEAR                                     = 0xD8,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_CLEAR_ENTRY                               = 0xD9,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_BINARY                                    = 0xDA,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_OCTAL                                     = 0xDB,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_DECIMAL                                   = 0xDC,
    USB_HID_KEYBOARD_KEYPAD_KEYPAD_HEXADECIMAL                               = 0xDD,
    /* Reserved                                                                         = 0xDE-0xDF */
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LEFT_CONTROL                            = 0xE0,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LEFT_SHIFT                              = 0xE1,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LEFT_ALT                                = 0xE2,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LEFT_GUI                                = 0xE3,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RIGHT_CONTROL                           = 0xE4,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RIGHT_SHIFT                             = 0xE5,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RIGHT_ALT                               = 0xE6,
    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RIGHT_GUI                               = 0xE7
    //0xE8-0xFFFF reserved
} USB_HID_KEYBOARD_KEYPAD;
//HID_KEYBD_Info_TypeDef     keybd_info;
//uint32_t                   keybd_report_data[2];
const int UKkeyValue[202] = {
	0,0,//    USB_HID_KEYBOARD_KEYPAD_RESERVED_NO_EVENT_INDICATED                      = 0x00,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_ERROR_ROLL_OVER                         = 0x01,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_POST_FAIL                               = 0x02,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_ERROR_UNDEFINED                         = 0x03,
	97,65,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_A                                       = 0x04,
	98,66,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_B                                       = 0x05,
	99,67,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_C                                       = 0x06,
	100,68,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_D                                       = 0x07,
	101,69,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_E                                       = 0x08,
	102,70,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F                                       = 0x09,
	103,71,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_G                                       = 0x0A,
	104,72,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_H                                       = 0x0B,
	105,73,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_I                                       = 0x0C,
	106,74,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_J                                       = 0x0D,
	107,75,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_K                                       = 0x0E,
	108,76,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_L                                       = 0x0F,
	109,77,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_M                                       = 0x10,
	110,78,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_N                                       = 0x11,
	111,79,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_O                                       = 0x12,
	112,80,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_P                                       = 0x13,
	113,81,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Q                                       = 0x14,
	114,82,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_R                                       = 0x15,
	115,83,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_S                                       = 0x16,
	116,84,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_T                                       = 0x17,
	117,85,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_U                                       = 0x18,
	118,86,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_V                                       = 0x19,
	119,87,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_W                                       = 0x1A,
	120,88,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_X                                       = 0x1B,
	121,89,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Y                                       = 0x1C,
	122,90,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Z                                       = 0x1D,
	49,33,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_1_AND_EXCLAMATION_POINT                 = 0x1E,
	50,34,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_2_AND_AT                                = 0x1F,  QUOTE UK!!!!
	51,35,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_3_AND_HASH                              = 0x20,
	52,36,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_4_AND_DOLLAR                            = 0x21,
	53,37,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_5_AND_PERCENT                           = 0x22,
	54,94,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_6_AND_CARROT                            = 0x23,
	55,38,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_7_AND_AMPERSAND                         = 0x24,
	56,42,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_8_AND_ASTERISK                          = 0x25,
	57,40,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_9_AND_OPEN_PARENTHESIS                  = 0x26,
	48,41,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_0_AND_CLOSE_PARENTHESIS                 = 0x27,
	10,10,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RETURN_ENTER                            = 0x28,
	27,27,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_ESCAPE                                  = 0x29,
	8,8,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_DELETE                                  = 0x2A,
	9,0x9f,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_TAB                                     = 0x2B,
	32,32,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SPACEBAR                                = 0x2C,
	45,95,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_MINUS_AND_UNDERSCORE                    = 0x2D,
	61,43,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_EQUAL_AND_PLUS                          = 0x2E,
	91,123,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_OPEN_BRACKET_AND_OPEN_CURLY_BRACE       = 0x2F,
	93,125,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CLOSE_BRACKET_AND_CLOSE_CURLY_BRACE     = 0x30,
	92,124,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_BACK_SLASH_AND_PIPE                     = 0x31,
	35,126,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_NON_US_HASH_AND_TILDE                   = 0x32,
	59,58,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SEMICOLON_AND_COLON                     = 0x33,
	39,64,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_APOSTROPHE_AND_QUOTE                    = 0x34, @ UK///
	96,126,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_GRAVE_ACCENT_AND_TILDE                  = 0x35,
	44,60,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_COMMA_AND_LESS_THAN                     = 0x36,
	46,62,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PERIOD_AND_GREATER_THAN                 = 0x37,
	47,63,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_FORWARD_SLASH_AND_QUESTION_MARK         = 0x38,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CAPS_LOCK                               = 0x39,
	0x91,0xB1,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F1                                = 0x3A,
	0x92,0xB2,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F2                                = 0x3B,
	0x93,0xB3,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F3                                = 0x3C,
	0x94,0xB4,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F4                                = 0x3D,
	0x95,0xB5,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F5                                = 0x3E,
	0x96,0xB6,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F6                                = 0x3F,
	0x97,0xB7,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F7                                = 0x40,
	0x98,0xB8,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F8                                = 0x41,
	0x99,0xB9,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F9                                = 0x42,
	0x9a,0xBa,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F10                               = 0x43,
	0x9b,0xBb,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F11                               = 0x44,
	0x9c,0xBc,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F12                               = 0x45,
	0x9d,0x9d,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PRINT_SCREEN                      = 0x46,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SCROLL_LOCK                             = 0x47,
	0x9e,0x9e,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PAUSE                                   = 0x48,
	0x84,0x84,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_INSERT                                  = 0x49,
	0x86,0x86,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_HOME                                    = 0x4A,
	0x88,0x88,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PAGE_UP                                 = 0x4B,
	0x7f,0xa0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_DELETE_FORWARD                          = 0x4C,
	0x87,0x87,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_END                                     = 0x4D,
	0x89,0x89,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PAGE_DOWN                               = 0x4E,
	0x83,0xA3,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RIGHT_ARROW                             = 0x4F,
	0x82,0x82,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LEFT_ARROW                              = 0x50,
	0x81,0xA1,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_DOWN_ARROW                              = 0x51,
	0x80,0x80,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_UP_ARROW                                = 0x52,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_NUM_LOCK_AND_CLEAR                        = 0x53,
	47,47,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_BACK_SLASH                                = 0x54,
	42,42,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_ASTERISK                                  = 0x55,
	45,45,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_MINUS                                     = 0x56,
	43,43,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_PLUS                                      = 0x57,
	10,10,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_ENTER                                     = 0x58,
	49,0x87,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_1_AND_END                                 = 0x59,
	50,0x81,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_2_AND_DOWN_ARROW                          = 0x5A,
	51,0x89,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_3_AND_PAGE_DOWN                           = 0x5B,
	52,0x82,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_4_AND_LEFT_ARROW                          = 0x5C,
	53,53,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_5                                         = 0x5D,
	54,0x83,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_6_AND_RIGHT_ARROW                         = 0x5E,
	55,0x86,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_7_AND_HOME                                = 0x5F,
	56,0x80,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_8_AND_UP_ARROW                            = 0x60,
	57,0x88,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_9_AND_PAGE_UP                             = 0x61,
	48,0x84,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_0_AND_INSERT                              = 0x62,
	46,0x7f,    //USB_HID_KEYBOARD_KEYPAD_KEYPAD_PERIOD_AND_DELETE                         = 0x63,
	92,124    //USB_HID_KEYBOARD_KEYPAD_KEYBOARD_NON_US_FORWARD_SLASH_AND_PIPE           = 0x64,
};

const int USkeyValue[202] = {
	0,0,//    USB_HID_KEYBOARD_KEYPAD_RESERVED_NO_EVENT_INDICATED                      = 0x00,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_ERROR_ROLL_OVER                         = 0x01,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_POST_FAIL                               = 0x02,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_ERROR_UNDEFINED                         = 0x03,
	97,65,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_A                                       = 0x04,
	98,66,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_B                                       = 0x05,
	99,67,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_C                                       = 0x06,
	100,68,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_D                                       = 0x07,
	101,69,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_E                                       = 0x08,
	102,70,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F                                       = 0x09,
	103,71,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_G                                       = 0x0A,
	104,72,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_H                                       = 0x0B,
	105,73,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_I                                       = 0x0C,
	106,74,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_J                                       = 0x0D,
	107,75,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_K                                       = 0x0E,
	108,76,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_L                                       = 0x0F,
	109,77,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_M                                       = 0x10,
	110,78,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_N                                       = 0x11,
	111,79,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_O                                       = 0x12,
	112,80,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_P                                       = 0x13,
	113,81,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Q                                       = 0x14,
	114,82,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_R                                       = 0x15,
	115,83,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_S                                       = 0x16,
	116,84,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_T                                       = 0x17,
	117,85,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_U                                       = 0x18,
	118,86,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_V                                       = 0x19,
	119,87,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_W                                       = 0x1A,
	120,88,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_X                                       = 0x1B,
	121,89,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Y                                       = 0x1C,
	122,90,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Z                                       = 0x1D,
	49,33,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_1_AND_EXCLAMATION_POINT                 = 0x1E,
	50,64,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_2_AND_AT                                = 0x1F,  QUOTE UK!!!!
	51,35,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_3_AND_HASH                              = 0x20,
	52,36,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_4_AND_DOLLAR                            = 0x21,
	53,37,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_5_AND_PERCENT                           = 0x22,
	54,94,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_6_AND_CARROT                            = 0x23,
	55,38,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_7_AND_AMPERSAND                         = 0x24,
	56,42,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_8_AND_ASTERISK                          = 0x25,
	57,40,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_9_AND_OPEN_PARENTHESIS                  = 0x26,
	48,41,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_0_AND_CLOSE_PARENTHESIS                 = 0x27,
	10,10,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RETURN_ENTER                            = 0x28,
	27,27,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_ESCAPE                                  = 0x29,
	8,8,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_DELETE                                  = 0x2A,
	9,0x9f,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_TAB                                     = 0x2B,
	32,32,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SPACEBAR                                = 0x2C,
	45,95,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_MINUS_AND_UNDERSCORE                    = 0x2D,
	61,43,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_EQUAL_AND_PLUS                          = 0x2E,
	91,123,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_OPEN_BRACKET_AND_OPEN_CURLY_BRACE       = 0x2F,
	93,125,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CLOSE_BRACKET_AND_CLOSE_CURLY_BRACE     = 0x30,
	92,124,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_BACK_SLASH_AND_PIPE                     = 0x31,
	92,124,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_NON_US_HASH_AND_TILDE                   = 0x32,
	59,58,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SEMICOLON_AND_COLON                     = 0x33,
	39,34,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_APOSTROPHE_AND_QUOTE                    = 0x34, @ UK///
	96,126,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_GRAVE_ACCENT_AND_TILDE                  = 0x35,
	44,60,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_COMMA_AND_LESS_THAN                     = 0x36,
	46,62,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PERIOD_AND_GREATER_THAN                 = 0x37,
	47,63,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_FORWARD_SLASH_AND_QUESTION_MARK         = 0x38,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CAPS_LOCK                               = 0x39,
	0x91,0xB1,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F1                                = 0x3A,
	0x92,0xB2,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F2                                = 0x3B,
	0x93,0xB3,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F3                                = 0x3C,
	0x94,0xB4,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F4                                = 0x3D,
	0x95,0xB5,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F5                                = 0x3E,
	0x96,0xB6,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F6                                = 0x3F,
	0x97,0xB7,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F7                                = 0x40,
	0x98,0xB8,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F8                                = 0x41,
	0x99,0xB9,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F9                                = 0x42,
	0x9a,0xBa,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F10                               = 0x43,
	0x9b,0xBb,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F11                               = 0x44,
	0x9c,0xBc,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F12                               = 0x45,
	0x9d,0x9d,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PRINT_SCREEN                      = 0x46,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SCROLL_LOCK                             = 0x47,
	0x9e,0x9e,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PAUSE                                   = 0x48,
	0x84,0x84,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_INSERT                                  = 0x49,
	0x86,0x86,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_HOME                                    = 0x4A,
	0x88,0x88,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PAGE_UP                                 = 0x4B,
	0x7f,0xa0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_DELETE_FORWARD                          = 0x4C,
	0x87,0x87,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_END                                     = 0x4D,
	0x89,0x89,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PAGE_DOWN                               = 0x4E,
	0x83,0xA3,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RIGHT_ARROW                             = 0x4F,
	0x82,0x82,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LEFT_ARROW                              = 0x50,
	0x81,0xA1,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_DOWN_ARROW                              = 0x51,
	0x80,0x80,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_UP_ARROW                                = 0x52,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_NUM_LOCK_AND_CLEAR                        = 0x53,
	47,47,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_BACK_SLASH                                = 0x54,
	42,42,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_ASTERISK                                  = 0x55,
	45,45,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_MINUS                                     = 0x56,
	43,43,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_PLUS                                      = 0x57,
	10,10,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_ENTER                                     = 0x58,
	49,0x87,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_1_AND_END                                 = 0x59,
	50,0x81,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_2_AND_DOWN_ARROW                          = 0x5A,
	51,0x89,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_3_AND_PAGE_DOWN                           = 0x5B,
	52,0x82,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_4_AND_LEFT_ARROW                          = 0x5C,
	53,53,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_5                                         = 0x5D,
	54,0x83,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_6_AND_RIGHT_ARROW                         = 0x5E,
	55,0x86,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_7_AND_HOME                                = 0x5F,
	56,0x80,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_8_AND_UP_ARROW                            = 0x60,
	57,0x88,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_9_AND_PAGE_UP                             = 0x61,
	48,0x84,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_0_AND_INSERT                              = 0x62,
	46,0x7f,    //USB_HID_KEYBOARD_KEYPAD_KEYPAD_PERIOD_AND_DELETE                         = 0x63,
	92,124    //USB_HID_KEYBOARD_KEYPAD_KEYBOARD_NON_US_FORWARD_SLASH_AND_PIPE           = 0x64,
};
const int DEkeyValue[202] = {
	0,0,//    USB_HID_KEYBOARD_KEYPAD_RESERVED_NO_EVENT_INDICATED                      = 0x00,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_ERROR_ROLL_OVER                         = 0x01,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_POST_FAIL                               = 0x02,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_ERROR_UNDEFINED                         = 0x03,
	97,65,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_A                                       = 0x04,
	98,66,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_B                                       = 0x05,
	99,67,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_C                                       = 0x06,
	100,68,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_D                                       = 0x07,
	101,69,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_E                                       = 0x08,
	102,70,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F                                       = 0x09,
	103,71,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_G                                       = 0x0A,
	104,72,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_H                                       = 0x0B,
	105,73,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_I                                       = 0x0C,
	106,74,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_J                                       = 0x0D,
	107,75,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_K                                       = 0x0E,
	108,76,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_L                                       = 0x0F,
	109,77,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_M                                       = 0x10,
	110,78,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_N                                       = 0x11,
	111,79,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_O                                       = 0x12,
	112,80,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_P                                       = 0x13,
	113,81,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Q                                       = 0x14,
	114,82,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_R                                       = 0x15,
	115,83,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_S                                       = 0x16,
	116,84,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_T                                       = 0x17,
	117,85,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_U                                       = 0x18,
	118,86,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_V                                       = 0x19,
	119,87,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_W                                       = 0x1A,
	120,88,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_X                                       = 0x1B,
	122,90,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Y                                       = 0x1C,
	121,89,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Z                                       = 0x1D,
	49,33,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_1_AND_EXCLAMATION_POINT                 = 0x1E,
	50,34,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_2_AND_AT                                = 0x1F,
	51,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_3_AND_HASH                              = 0x20,      DE: §=245
	52,36,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_4_AND_DOLLAR                            = 0x21,
	53,37,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_5_AND_PERCENT                           = 0x22,
	54,38,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_6_AND_CARROT                            = 0x23,
	55,47,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_7_AND_AMPERSAND                         = 0x24,
	56,40,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_8_AND_ASTERISK                          = 0x25,
	57,41,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_9_AND_OPEN_PARENTHESIS                  = 0x26,
	48,61,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_0_AND_CLOSE_PARENTHESIS                 = 0x27,
	10,10,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RETURN_ENTER                            = 0x28,
	27,27,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_ESCAPE                                  = 0x29,
	8,8,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_DELETE                                  = 0x2A,
	9,0x9f,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_TAB                                     = 0x2B,
	32,32,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SPACEBAR                                = 0x2C,

	/* Added special keyboard characters where keyboard characters that don't have
	 * an ascii representation give high value codes 200 upwards*/
    201, 63, //    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_MINUS_AND_UNDERSCORE                 = 0x2D,   DE: ß=225
	202, 96, //    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_EQUAL_AND_PLUS                       = 0x2E,   DE: ´=239
	203, 204, //    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_OPEN_BRACKET_AND_OPEN_CURLY_BRACE   = 0x2F,   DE: ü=129,Ü=154
	43, 42, //    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CLOSE_BRACKET_AND_CLOSE_CURLY_BRACE   = 0x30,
	35, 39, //    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_BACK_SLASH_AND_PIPE                   = 0x31,
	35, 39, //    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_NON_US_HASH_AND_TILDE                 = 0x32,
	205, 206, //    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SEMICOLON_AND_COLON                 = 0x33,   DE: ö=228,Ö=229
	207, 208, //    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_APOSTROPHE_AND_QUOTE                = 0x34,   DE: ä=132,Ä=142
	94, 209, //    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_GRAVE_ACCENT_AND_TILDE               = 0x35,   DE: °=167

//	0,63,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_MINUS_AND_UNDERSCORE                    = 0x2D,   DE: ß=225
//	0,96,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_EQUAL_AND_PLUS                          = 0x2E,   DE: ´=239
//	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_OPEN_BRACKET_AND_OPEN_CURLY_BRACE       = 0x2F,    DE: ü=129,Ü=154
//	43,42,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CLOSE_BRACKET_AND_CLOSE_CURLY_BRACE     = 0x30,
//	35,39,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_BACK_SLASH_AND_PIPE                     = 0x31,
//	35,39,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_NON_US_HASH_AND_TILDE                   = 0x32,
//	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SEMICOLON_AND_COLON                     = 0x33,   DE: ö=228,Ö=229
//	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_APOSTROPHE_AND_QUOTE                    = 0x34,   DE: ä=132,Ä=142
//	94,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_GRAVE_ACCENT_AND_TILDE                  = 0x35,  DE: °=167


	44,59,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_COMMA_AND_LESS_THAN                     = 0x36,
	46,58,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PERIOD_AND_GREATER_THAN                 = 0x37,
	45,95,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_FORWARD_SLASH_AND_QUESTION_MARK         = 0x38,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CAPS_LOCK                               = 0x39,
	0x91,0xB1,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F1                                = 0x3A,
	0x92,0xB2,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F2                                = 0x3B,
	0x93,0xB3,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F3                                = 0x3C,
	0x94,0xB4,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F4                                = 0x3D,
	0x95,0xB5,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F5                                = 0x3E,
	0x96,0xB6,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F6                                = 0x3F,
	0x97,0xB7,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F7                                = 0x40,
	0x98,0xB8,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F8                                = 0x41,
	0x99,0xB9,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F9                                = 0x42,
	0x9a,0xBa,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F10                               = 0x43,
	0x9b,0xBb,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F11                               = 0x44,
	0x9c,0xBc,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F12                               = 0x45,
	0x9d,0x9d,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PRINT_SCREEN                      = 0x46,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SCROLL_LOCK                             = 0x47,
	0x9e,0x9e,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PAUSE                                   = 0x48,
	0x84,0x84,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_INSERT                                  = 0x49,
	0x86,0x86,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_HOME                                    = 0x4A,
	0x88,0x88,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PAGE_UP                                 = 0x4B,
	0x7f,0xa0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_DELETE_FORWARD                          = 0x4C,
	0x87,0x87,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_END                                     = 0x4D,
	0x89,0x89,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PAGE_DOWN                               = 0x4E,
	0x83,0xA3,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RIGHT_ARROW                             = 0x4F,
	0x82,0x82,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LEFT_ARROW                              = 0x50,
	0x81,0xA1,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_DOWN_ARROW                              = 0x51,
	0x80,0x80,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_UP_ARROW                                = 0x52,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_NUM_LOCK_AND_CLEAR                        = 0x53,
	47,47,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_BACK_SLASH                                = 0x54,
	42,42,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_ASTERISK                                  = 0x55,
	45,45,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_MINUS                                     = 0x56,
	43,43,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_PLUS                                      = 0x57,
	10,10,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_ENTER                                     = 0x58,
	49,0x87,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_1_AND_END                                 = 0x59,
	50,0x81,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_2_AND_DOWN_ARROW                          = 0x5A,
	51,0x89,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_3_AND_PAGE_DOWN                           = 0x5B,
	52,0x82,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_4_AND_LEFT_ARROW                          = 0x5C,
	53,53,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_5                                         = 0x5D,
	54,0x83,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_6_AND_RIGHT_ARROW                         = 0x5E,
	55,0x86,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_7_AND_HOME                                = 0x5F,
	56,0x80,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_8_AND_UP_ARROW                            = 0x60,
	57,0x88,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_9_AND_PAGE_UP                             = 0x61,
	48,0x84,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_0_AND_INSERT                              = 0x62,
	46,0x7f,    //USB_HID_KEYBOARD_KEYPAD_KEYPAD_PERIOD_AND_DELETE                         = 0x63,
	60,62    //USB_HID_KEYBOARD_KEYPAD_KEYBOARD_NON_US_FORWARD_SLASH_AND_PIPE           = 0x64,
};
const int FRkeyValue[202] = {
	0,0,//    USB_HID_KEYBOARD_KEYPAD_RESERVED_NO_EVENT_INDICATED                      = 0x00,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_ERROR_ROLL_OVER                         = 0x01,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_POST_FAIL                               = 0x02,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_ERROR_UNDEFINED                         = 0x03,
	113,81,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_A                                    = 0x04, FR Q
	98,66,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_B                                     = 0x05,
	99,67,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_C                                     = 0x06,
	100,68,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_D                                    = 0x07,
	101,69,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_E                                    = 0x08,
	102,70,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F                                    = 0x09,
	103,71,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_G                                    = 0x0A,
	104,72,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_H                                    = 0x0B,
	105,73,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_I                                    = 0x0C,
	106,74,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_J                                    = 0x0D,
	107,75,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_K                                    = 0x0E,
	108,76,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_L                                    = 0x0F,
	44,63,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_M                                     = 0x10, FR ,?
	110,78,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_N                                    = 0x11,
	111,79,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_O                                    = 0x12,
	112,80,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_P                                    = 0x13,
	97,65,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Q                                     = 0x14, FR A
	114,82,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_R                                    = 0x15,
	115,83,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_S                                    = 0x16,
	116,84,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_T                                    = 0x17,
	117,85,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_U                                    = 0x18,
	118,86,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_V                                    = 0x19,
	122,90,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_W                                    = 0x1A, FR Z
	120,88,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_X                                    = 0x1B,
	121,89,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Y                                    = 0x1C,
	119,87,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Z                                    = 0x1D, FR W
	38,49,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_1_AND_EXCLAMATION_POINT               = 0x1E, FR & 1
	233,50,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_2_AND_AT                             = 0x1F, FR é 2 ~
	34,51,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_3_AND_HASH                            = 0x20, FR " 3 #
	39,52,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_4_AND_DOLLAR                          = 0x21, FR ' 4 {
	40,53,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_5_AND_PERCENT                         = 0x22, FR ( 5 [
	45,54,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_6_AND_CARROT                          = 0x23, FR - 6
	232,55,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_7_AND_AMPERSAND                      = 0x24, FR è 7 `
	95,56,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_8_AND_ASTERISK                        = 0x25, FR _ 8 '\'
	135,57,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_9_AND_OPEN_PARENTHESIS               = 0x26, FR ç 9 ^
	00,48,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_0_AND_CLOSE_PARENTHESIS               = 0x27, FR à 0 @
	10,10,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RETURN_ENTER                          = 0x28,
	27,27,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_ESCAPE                                = 0x29,
	8,8,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_DELETE                                  = 0x2A,
	9,0x9f,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_TAB                                     = 0x2B,
	32,32,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SPACEBAR                              = 0x2C,
	41,00,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_MINUS_AND_UNDERSCORE                  = 0x2D, FR ) ]
	61,43,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_EQUAL_AND_PLUS                        = 0x2E,
	94,168,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_OPEN_BRACKET_AND_OPEN_CURLY_BRACE    = 0x2F, FR ^ ¨
	36,163,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CLOSE_BRACKET_AND_CLOSE_CURLY_BRACE  = 0x30, FR $ £
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_BACK_SLASH_AND_PIPE                     = 0x31, -- not present --
	42,181,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_NON_US_HASH_AND_TILDE                = 0x32, FR * µ
	109,77,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SEMICOLON_AND_COLON                  = 0x33, FR M
	249,37,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_APOSTROPHE_AND_QUOTE                 = 0x34, FR ù %
	178,00,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_GRAVE_ACCENT_AND_TILDE               = 0x35, FR ²
	59,46,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_COMMA_AND_LESS_THAN                   = 0x36, FR ; .
	58,47,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PERIOD_AND_GREATER_THAN               = 0x37, FR : /
	33,167,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_FORWARD_SLASH_AND_QUESTION_MARK      = 0x38, FR ! §
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CAPS_LOCK                               = 0x39,
	0x91,0xB1,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F1                                = 0x3A,
	0x92,0xB2,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F2                                = 0x3B,
	0x93,0xB3,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F3                                = 0x3C,
	0x94,0xB4,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F4                                = 0x3D,
	0x95,0xB5,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F5                                = 0x3E,
	0x96,0xB6,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F6                                = 0x3F,
	0x97,0xB7,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F7                                = 0x40,
	0x98,0xB8,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F8                                = 0x41,
	0x99,0xB9,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F9                                = 0x42,
	0x9a,0xBa,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F10                               = 0x43,
	0x9b,0xBb,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F11                               = 0x44,
	0x9c,0xBc,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F12                               = 0x45,
	0x9d,0x9d,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PRINT_SCREEN                      = 0x46,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SCROLL_LOCK                             = 0x47,
	0x9e,0x9e,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PAUSE                                   = 0x48,
	0x84,0x84,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_INSERT                                = 0x49,
	0x86,0x86,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_HOME                                  = 0x4A,
	0x88,0x88,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PAGE_UP                               = 0x4B,
	0x7f,0xa0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_DELETE_FORWARD                    = 0x4C,
	0x87,0x87,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_END                               = 0x4D,
	0x89,0x89,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PAGE_DOWN                         = 0x4E,
	0x83,0xA3,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RIGHT_ARROW                       = 0x4F,
	0x82,0x82,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LEFT_ARROW                        = 0x50,
	0x81,0xA1,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_DOWN_ARROW                        = 0x51,
	0x80,0x80,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_UP_ARROW                          = 0x52,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_NUM_LOCK_AND_CLEAR                        = 0x53,
	47,47,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_BACK_SLASH                              = 0x54,
	42,42,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_ASTERISK                                = 0x55,
	45,45,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_MINUS                                   = 0x56,
	43,43,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_PLUS                                    = 0x57,
	10,10,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_ENTER                                   = 0x58,
	49,0x87,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_1_AND_END                             = 0x59,
	50,0x81,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_2_AND_DOWN_ARROW                      = 0x5A,
	51,0x89,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_3_AND_PAGE_DOWN                       = 0x5B,
	52,0x82,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_4_AND_LEFT_ARROW                      = 0x5C,
	53,53,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_5                                       = 0x5D,
	54,0x83,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_6_AND_RIGHT_ARROW                     = 0x5E,
	55,0x86,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_7_AND_HOME                            = 0x5F,
	56,0x80,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_8_AND_UP_ARROW                        = 0x60,
	57,0x88,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_9_AND_PAGE_UP                         = 0x61,
	48,0x84,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_0_AND_INSERT                          = 0x62,
	46,0x7f,    //USB_HID_KEYBOARD_KEYPAD_KEYPAD_PERIOD_AND_DELETE                     = 0x63,
	60,62    //USB_HID_KEYBOARD_KEYPAD_KEYBOARD_NON_US_FORWARD_SLASH_AND_PIPE          = 0x64, FR < >
};
const int ESkeyValue[202] = {
	0,0,//          USB_HID_KEYBOARD_KEYPAD_RESERVED_NO_EVENT_INDICATED                    = 0x00,
	0,0,//          USB_HID_KEYBOARD_KEYPAD_KEYBOARD_ERROR_ROLL_OVER                       = 0x01,
	0,0,//          USB_HID_KEYBOARD_KEYPAD_KEYBOARD_POST_FAIL                             = 0x02,
	0,0,//          USB_HID_KEYBOARD_KEYPAD_KEYBOARD_ERROR_UNDEFINED                       = 0x03,
	97,65,//        USB_HID_KEYBOARD_KEYPAD_KEYBOARD_A                                     = 0x04,
	98,66,//        USB_HID_KEYBOARD_KEYPAD_KEYBOARD_B                                     = 0x05,
	99,67,//        USB_HID_KEYBOARD_KEYPAD_KEYBOARD_C                                     = 0x06,
	100,68,//       USB_HID_KEYBOARD_KEYPAD_KEYBOARD_D                                     = 0x07,
	101,69,//       USB_HID_KEYBOARD_KEYPAD_KEYBOARD_E                                     = 0x08, ALT GR -> €
	102,70,//       USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F                                     = 0x09,
	103,71,//       USB_HID_KEYBOARD_KEYPAD_KEYBOARD_G                                     = 0x0A,
	104,72,//       USB_HID_KEYBOARD_KEYPAD_KEYBOARD_H                                     = 0x0B,
	105,73,//       USB_HID_KEYBOARD_KEYPAD_KEYBOARD_I                                     = 0x0C,
	106,74,//       USB_HID_KEYBOARD_KEYPAD_KEYBOARD_J                                     = 0x0D,
	107,75,//       USB_HID_KEYBOARD_KEYPAD_KEYBOARD_K                                     = 0x0E,
	108,76,//       USB_HID_KEYBOARD_KEYPAD_KEYBOARD_L                                     = 0x0F,
	109,77,//       USB_HID_KEYBOARD_KEYPAD_KEYBOARD_M                                     = 0x10,
	110,78,//       USB_HID_KEYBOARD_KEYPAD_KEYBOARD_N                                     = 0x11,
	111,79,//       USB_HID_KEYBOARD_KEYPAD_KEYBOARD_O                                     = 0x12,
	112,80,//       USB_HID_KEYBOARD_KEYPAD_KEYBOARD_P                                     = 0x13,
	113,81,//       USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Q                                     = 0x14,
	114,82,//       USB_HID_KEYBOARD_KEYPAD_KEYBOARD_R                                     = 0x15,
	115,83,//       USB_HID_KEYBOARD_KEYPAD_KEYBOARD_S                                     = 0x16,
	116,84,//       USB_HID_KEYBOARD_KEYPAD_KEYBOARD_T                                     = 0x17,
	117,85,//       USB_HID_KEYBOARD_KEYPAD_KEYBOARD_U                                     = 0x18,
	118,86,//       USB_HID_KEYBOARD_KEYPAD_KEYBOARD_V                                     = 0x19,
	119,87,//       USB_HID_KEYBOARD_KEYPAD_KEYBOARD_W                                     = 0x1A,
	120,88,//       USB_HID_KEYBOARD_KEYPAD_KEYBOARD_X                                     = 0x1B,
	121,89,//       USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Y                                     = 0x1C,
	122,90,//       USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Z                                     = 0x1D,·
	49,33,//        USB_HID_KEYBOARD_KEYPAD_KEYBOARD_1_AND_EXCLAMATION_POINT               = 0x1E, ALT GR -> |
	50,34,//        USB_HID_KEYBOARD_KEYPAD_KEYBOARD_2_AND_AT                              = 0x1F, ALT GR -> @
	51,0,//         USB_HID_KEYBOARD_KEYPAD_KEYBOARD_3_AND_HASH                            = 0x20, ALT GR -> #
	52,36,//        USB_HID_KEYBOARD_KEYPAD_KEYBOARD_4_AND_DOLLAR                          = 0x21, ALT GR -> ~
	53,37,//        USB_HID_KEYBOARD_KEYPAD_KEYBOARD_5_AND_PERCENT                         = 0x22,
	54,38,//        USB_HID_KEYBOARD_KEYPAD_KEYBOARD_6_AND_CARROT                          = 0x23,
	55,47,//        USB_HID_KEYBOARD_KEYPAD_KEYBOARD_7_AND_AMPERSAND                       = 0x24,
	56,40,//        USB_HID_KEYBOARD_KEYPAD_KEYBOARD_8_AND_ASTERISK                        = 0x25,
	57,41,//        USB_HID_KEYBOARD_KEYPAD_KEYBOARD_9_AND_OPEN_PARENTHESIS                = 0x26,
	48,61,//        USB_HID_KEYBOARD_KEYPAD_KEYBOARD_0_AND_CLOSE_PARENTHESIS               = 0x27,
	10,10,//        USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RETURN_ENTER                          = 0x28,
	27,27,//        USB_HID_KEYBOARD_KEYPAD_KEYBOARD_ESCAPE                                = 0x29,
	8,8,//          USB_HID_KEYBOARD_KEYPAD_KEYBOARD_DELETE                                = 0x2A,
	9,0x9f,//          USB_HID_KEYBOARD_KEYPAD_KEYBOARD_TAB                                   = 0x2B,
	32,32,//        USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SPACEBAR                              = 0x2C,
	39,63,//        USB_HID_KEYBOARD_KEYPAD_KEYBOARD_MINUS_AND_UNDERSCORE                  = 0x2D,
	0,0,//          USB_HID_KEYBOARD_KEYPAD_KEYBOARD_EQUAL_AND_PLUS                        = 0x2E,
	0,94,//         USB_HID_KEYBOARD_KEYPAD_KEYBOARD_OPEN_BRACKET_AND_OPEN_CURLY_BRACE     = 0x2F, ALT GR -> [
	43,42,//        USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CLOSE_BRACKET_AND_CLOSE_CURLY_BRACE   = 0x30, ALT GR -> ]
	0,0,//          USB_HID_KEYBOARD_KEYPAD_KEYBOARD_BACK_SLASH_AND_PIPE                   = 0x31, ALT GR -> }
	92,0,//         USB_HID_KEYBOARD_KEYPAD_KEYBOARD_NON_US_HASH_AND_TILDE                 = 0x32,
	0,0,//          USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SEMICOLON_AND_COLON                   = 0x33,
	0,0,//          USB_HID_KEYBOARD_KEYPAD_KEYBOARD_APOSTROPHE_AND_QUOTE                  = 0x34, ALT GR -> {
	96,96,//        USB_HID_KEYBOARD_KEYPAD_KEYBOARD_GRAVE_ACCENT_AND_TILDE                = 0x35, ALT GR -> backslash  (Tecla º)
	44,59,//        USB_HID_KEYBOARD_KEYPAD_KEYBOARD_COMMA_AND_LESS_THAN                   = 0x36,
	46,58,//        USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PERIOD_AND_GREATER_THAN               = 0x37,
	45,95,//        USB_HID_KEYBOARD_KEYPAD_KEYBOARD_FORWARD_SLASH_AND_QUESTION_MARK       = 0x38,
	0,0,//          USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CAPS_LOCK                             = 0x39,
	0x91,0xB1,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F1                                = 0x3A,
	0x92,0xB2,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F2                                = 0x3B,
	0x93,0xB3,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F3                                = 0x3C,
	0x94,0xB4,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F4                                = 0x3D,
	0x95,0xB5,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F5                                = 0x3E,
	0x96,0xB6,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F6                                = 0x3F,
	0x97,0xB7,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F7                                = 0x40,
	0x98,0xB8,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F8                                = 0x41,
	0x99,0xB9,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F9                                = 0x42,
	0x9a,0xBa,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F10                               = 0x43,
	0x9b,0xBb,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F11                               = 0x44,
	0x9c,0xBc,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F12                               = 0x45,
	0x9d,0x9d,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PRINT_SCREEN                      = 0x46,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SCROLL_LOCK                             = 0x47,
	0x9e,0x9e,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PAUSE                                   = 0x48,
	0x84,0x84,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_INSERT                                = 0x49,
	0x86,0x86,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_HOME                                  = 0x4A,
	0x88,0x88,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PAGE_UP                               = 0x4B,
	0x7f,0xa0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_DELETE_FORWARD                        = 0x4C,
	0x87,0x87,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_END                                   = 0x4D,
	0x89,0x89,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PAGE_DOWN                             = 0x4E,
	0x83,0xA3,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RIGHT_ARROW                           = 0x4F,
	0x82,0x82,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LEFT_ARROW                            = 0x50,
	0x81,0xA1,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_DOWN_ARROW                            = 0x51,
	0x80,0x80,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_UP_ARROW                              = 0x52,
	0,0,//          USB_HID_KEYBOARD_KEYPAD_KEYPAD_NUM_LOCK_AND_CLEAR                      = 0x53,
	47,47,//        USB_HID_KEYBOARD_KEYPAD_KEYPAD_BACK_SLASH                              = 0x54,
	42,42,//        USB_HID_KEYBOARD_KEYPAD_KEYPAD_ASTERISK                                = 0x55,
	45,45,//        USB_HID_KEYBOARD_KEYPAD_KEYPAD_MINUS                                   = 0x56,
	43,43,//        USB_HID_KEYBOARD_KEYPAD_KEYPAD_PLUS                                    = 0x57,
	10,10,//        USB_HID_KEYBOARD_KEYPAD_KEYPAD_ENTER                                   = 0x58,
	49,0x87,//      USB_HID_KEYBOARD_KEYPAD_KEYPAD_1_AND_END                               = 0x59,
	50,0x81,//      USB_HID_KEYBOARD_KEYPAD_KEYPAD_2_AND_DOWN_ARROW                        = 0x5A,
	51,0x89,//      USB_HID_KEYBOARD_KEYPAD_KEYPAD_3_AND_PAGE_DOWN                         = 0x5B,
	52,0x82,//      USB_HID_KEYBOARD_KEYPAD_KEYPAD_4_AND_LEFT_ARROW                        = 0x5C,
	53,53,//        USB_HID_KEYBOARD_KEYPAD_KEYPAD_5                                       = 0x5D,
	54,0x83,//      USB_HID_KEYBOARD_KEYPAD_KEYPAD_6_AND_RIGHT_ARROW                       = 0x5E,
	55,0x86,//      USB_HID_KEYBOARD_KEYPAD_KEYPAD_7_AND_HOME                              = 0x5F,
	56,0x80,//      USB_HID_KEYBOARD_KEYPAD_KEYPAD_8_AND_UP_ARROW                          = 0x60,
	57,0x88,//      USB_HID_KEYBOARD_KEYPAD_KEYPAD_9_AND_PAGE_UP                           = 0x61,
	48,0x84,//      USB_HID_KEYBOARD_KEYPAD_KEYPAD_0_AND_INSERT                            = 0x62,
	46,0x7f,//      USB_HID_KEYBOARD_KEYPAD_KEYPAD_PERIOD_AND_DELETE                       = 0x63,
	60,62 //        USB_HID_KEYBOARD_KEYPAD_KEYBOARD_NON_US_FORWARD_SLASH_AND_PIPE         = 0x64,
};
const int BEkeyValue[202] = {
	0,0,//    USB_HID_KEYBOARD_KEYPAD_RESERVED_NO_EVENT_INDICATED                      = 0x00,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_ERROR_ROLL_OVER                         = 0x01,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_POST_FAIL                               = 0x02,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_ERROR_UNDEFINED                         = 0x03,
	113,81,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_A                                       = 0x04,
	98,66,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_B                                       = 0x05,
	99,67,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_C                                       = 0x06,
	100,68,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_D                                       = 0x07,
	101,69,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_E                                       = 0x08,
	102,70,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F                                       = 0x09,
	103,71,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_G                                       = 0x0A,
	104,72,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_H                                       = 0x0B,
	105,73,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_I                                       = 0x0C,
	106,74,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_J                                       = 0x0D,
	107,75,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_K                                       = 0x0E,
	108,76,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_L                                       = 0x0F,
	44,63,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_M                                       = 0x10,
	110,78,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_N                                       = 0x11,
	111,79,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_O                                       = 0x12,
	112,80,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_P                                       = 0x13,
	97,65,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Q                                       = 0x14,
	114,82,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_R                                       = 0x15,
	115,83,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_S                                       = 0x16,
	116,84,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_T                                       = 0x17,
	117,85,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_U                                       = 0x18,
	118,86,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_V                                       = 0x19,
	122,90,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_W                             = 0x1A,
	120,88,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_X                                       = 0x1B,
	121,89,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Y                             = 0x1C,
	119,87,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Z                                       = 0x1D,
	38,49,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_1_AND_EXCLAMATION_POINT                 = 0x1E,
	64,50,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_2_AND_AT                                = 0x1F,
	34,51,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_3_AND_HASH                              = 0x20,
	39,52,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_4_AND_DOLLAR                            = 0x21,
	40,53,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_5_AND_PERCENT                           = 0x22,
	36,54,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_6_AND_CARROT                            = 0x23,
	96,55,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_7_AND_AMPERSAND                         = 0x24,
	33,56,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_8_AND_ASTERISK                          = 0x25,
	123,57,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_9_AND_OPEN_PARENTHESIS                  = 0x26,
	125,48,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_0_AND_CLOSE_PARENTHESIS                 = 0x27,
	10,10,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RETURN_ENTER                            = 0x28,
	27,27,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_ESCAPE                                  = 0x29,
	8,8,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_DELETE                                  = 0x2A,
	9,9,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_TAB                                     = 0x2B,
	32,32,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SPACEBAR                                = 0x2C,
	41,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_MINUS_AND_UNDERSCORE                    = 0x2D,
	45,95,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_EQUAL_AND_PLUS                          = 0x2E,
	91,94,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_OPEN_BRACKET_AND_OPEN_CURLY_BRACE       = 0x2F,
	36,42,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CLOSE_BRACKET_AND_CLOSE_CURLY_BRACE     = 0x30,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_BACK_SLASH_AND_PIPE                     = 0x31,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_NON_US_HASH_AND_TILDE                   = 0x32,
	109,77,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SEMICOLON_AND_COLON                     = 0x33,
	0,37,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_APOSTROPHE_AND_QUOTE                    = 0x34,
	35,124,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_GRAVE_ACCENT_AND_TILDE                  = 0x35,
	59,46,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_COMMA_AND_LESS_THAN                     = 0x36,
	58,47,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PERIOD_AND_GREATER_THAN                 = 0x37,
	61,43,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_FORWARD_SLASH_AND_QUESTION_MARK         = 0x38,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CAPS_LOCK                               = 0x39,
	0x91,0xB1,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F1                                = 0x3A,
	0x92,0xB2,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F2                                = 0x3B,
	0x93,0xB3,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F3                                = 0x3C,
	0x94,0xB4,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F4                                = 0x3D,
	0x95,0xB5,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F5                                = 0x3E,
	0x96,0xB6,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F6                                = 0x3F,
	0x97,0xB7,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F7                                = 0x40,
	0x98,0xB8,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F8                                = 0x41,
	0x99,0xB9,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F9                                = 0x42,
	0x9a,0xBa,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F10                               = 0x43,
	0x9b,0xBb,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F11                               = 0x44,
	0x9c,0xBc,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F12                               = 0x45,
	0x9d,0x9d,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PRINT_SCREEN                      = 0x46,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SCROLL_LOCK                             = 0x47,
	0x9e,0x9e,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PAUSE                                   = 0x48,
	0x84,0x84,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_INSERT                                  = 0x49,
	0x86,0x86,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_HOME                                    = 0x4A,
	0x88,0x88,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PAGE_UP                                 = 0x4B,
	0x7f,0xa0,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_DELETE_FORWARD                          = 0x4C,
	0x87,0x87,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_END                                     = 0x4D,
	0x89,0x89,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PAGE_DOWN                               = 0x4E,
	0x83,0xA3,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RIGHT_ARROW                             = 0x4F,
	0x82,0x82,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LEFT_ARROW                              = 0x50,
	0x81,0xA1,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_DOWN_ARROW                              = 0x51,
	0x80,0x80,//    USB_HID_KEYBOARD_KEYPAD_KEYBOARD_UP_ARROW                                = 0x52,
	0,0,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_NUM_LOCK_AND_CLEAR                        = 0x53,
	47,47,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_BACK_SLASH                                = 0x54,
	42,42,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_ASTERISK                                  = 0x55,
	45,45,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_MINUS                                     = 0x56,
	43,43,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_PLUS                                      = 0x57,
	10,10,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_ENTER                                     = 0x58,
	49,0x87,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_1_AND_END                                 = 0x59,
	50,0x81,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_2_AND_DOWN_ARROW                          = 0x5A,
	51,0x89,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_3_AND_PAGE_DOWN                           = 0x5B,
	52,0x82,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_4_AND_LEFT_ARROW                          = 0x5C,
	53,53,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_5                                         = 0x5D,
	54,0x83,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_6_AND_RIGHT_ARROW                         = 0x5E,
	55,0x86,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_7_AND_HOME                                = 0x5F,
	56,0x80,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_8_AND_UP_ARROW                            = 0x60,
	57,0x88,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_9_AND_PAGE_UP                             = 0x61,
	48,0x84,//    USB_HID_KEYBOARD_KEYPAD_KEYPAD_0_AND_INSERT                              = 0x62,
	46,0x7f,    //USB_HID_KEYBOARD_KEYPAD_KEYPAD_PERIOD_AND_DELETE                         = 0x63,
	60,62    //USB_HID_KEYBOARD_KEYPAD_KEYBOARD_NON_US_FORWARD_SLASH_AND_PIPE           = 0x64,
};
/* Private function prototypes ----------------------------------------------- */
static void USBH_KeybdDemo(USBH_HandleTypeDef * phost);void HID_MenuProcess(void);
const int *keylayout;
int caps_lock=0;
int num_lock=0;
int scroll_lock=0;
HID_KEYBD_Info_TypeDef last_k_info;
HID_KEYBD_Info_TypeDef *k_pinfo;
int KeyDown[7];
uint8_t APP_MapKeyToUsage(HID_KEYBD_Info_TypeDef *info, int keyno);
/* Private functions --------------------------------------------------------- */

/**
  * @brief  Demo state machine.
  * @param  None
  * @retval None
  */

void CheckKeyboard(void){
	if(Option.USBKeyboard != NO_KEYBOARD){
		HID_MenuProcess();
		MX_USB_HOST_Process();
	}
}
void HID_MenuInit(void)
{
  /* Start HID Interface */
  if(Option.USBKeyboard==CONFIG_UK)keylayout=UKkeyValue;
  else if(Option.USBKeyboard==CONFIG_US)keylayout=USkeyValue;
  else if(Option.USBKeyboard==CONFIG_DE)keylayout=DEkeyValue;
  else if(Option.USBKeyboard==CONFIG_FR)keylayout=FRkeyValue;
  else if(Option.USBKeyboard==CONFIG_ES)keylayout=ESkeyValue;
  else if(Option.USBKeyboard==CONFIG_BE)keylayout=BEkeyValue;
  hid_demo.state = HID_DEMO_IDLE;
  HID_MenuProcess();
}

/* Private functions --------------------------------------------------------- */
/**
  * @brief  Manages HID Menu Process.
  * @param  None
  * @retval None
  */
void HID_MenuProcess(void)
{
  static int count=32;
  switch (hid_demo.state)
  {
  case HID_DEMO_IDLE:
    hid_demo.state = HID_DEMO_WAIT;
    hid_demo.select = 0;
    break;

  case HID_DEMO_WAIT:
          hid_demo.state = HID_DEMO_START;
    break;

  case HID_DEMO_START:
    if (Appli_state == APPLICATION_READY)
    {
      if (USBH_HID_GetDeviceType(&hUsbHostFS) == HID_KEYBOARD)
      {
        if(!CurrentLinePtr){
        	if((CurrentY+(3*gui_font_height))>=PageTable[WritePage].ymax) cmd_cls();
        	MMPrintString("USB Keyboard Connected OK \r\n> ");
        }
        hid_demo.keyboard_state = HID_KEYBOARD_IDLE;
        hid_demo.state = HID_DEMO_KEYBOARD;
        SD_LED_GPIO_Port->BSRR = SD_LED_Pin;
		count=32;
		scroll_lock=0;
		num_lock=0;
		caps_lock=0;
		sendlights=0;
      }
    }
    else
    {
      hid_demo.state = HID_DEMO_WAIT;
    }
    break;

  case HID_DEMO_REENUMERATE:
    /* Force HID Device to re-enumerate */
    USBH_ReEnumerate(&hUsbHostFS);
    hid_demo.state = HID_DEMO_WAIT;
    break;


  case HID_DEMO_KEYBOARD:
    if (Appli_state == APPLICATION_READY)
    {
      keyboardseen=1;
      USBH_KeybdDemo(&hUsbHostFS);
      if(!Option.noLED  && (sendlights & 0x8000)){
    	  USBH_HID_SetReport(&hUsbHostFS,HID_SEND_DATA,0,(uint8_t *)&sendlights,1);
          uSec(10); //Delay added as fix for CapsLock,NumLock,ScrollLock not being set G.A. 22/06/2024
                    // https://www.thebackshed.com/forum/ViewTopic.php?FID=16&TID=17037
   	      count--;
    	  if(!count){
    		  sendlights &=0xFF;
    		  count=32;
    	  }
      }
    }
    break;

  default:
    break;
  }

  if (Appli_state == APPLICATION_DISCONNECT)
  {
    if(!CurrentLinePtr){
    	if((CurrentY+(3*gui_font_height))>=PageTable[WritePage].ymax) cmd_cls();
       	MMPrintString("USB Keyboard DISConnected \r\n> ");  //USBFIX
    }  //USBFIX
    Appli_state = APPLICATION_IDLE;
    hid_demo.state = HID_DEMO_IDLE;
    hid_demo.select = 0;
    SD_LED_GPIO_Port->BSRR = SD_LED_Pin<<16;
  	keyboardseen=0;
	scroll_lock=0;
	num_lock=0;
	caps_lock=0;
	sendlights=0;
	count=32;
  }
}

/**
  * @brief  Main routine for Keyboard application
  * @param  phost: Host handle
  * @retval None
  */
void clearrepeat(void){
	keytimer=0;
	repeattime=Option.RepeatStart;
	mymemset(&last_k_info,0,sizeof(HID_KEYBD_Info_TypeDef));
	mymemset(k_pinfo,0,sizeof(HID_KEYBD_Info_TypeDef));
}

static void USBH_KeybdDemo(USBH_HandleTypeDef * phost)
{
	k_pinfo = USBH_HID_GetKeybdInfo(phost);
	uint8_t c;
	int i;
	if (k_pinfo != NULL){

		if((keytimer>Option.RepeatStart || k_pinfo->keys[0]!=last_k_info.keys[0])){
			keytimer=0;
			if(k_pinfo->keys[0]==0x39){
				if(caps_lock){
					sendlights&=~(uint8_t)2;
					sendlights|=0x8000;
					caps_lock=0;
				} else {
					sendlights|=0x02;
					sendlights|=0x8000;
					caps_lock=1;
				}
			} else if(k_pinfo->keys[0]==0x53){
				if(num_lock){
					sendlights&=~(uint8_t)1;
					sendlights|=0x8000;
					num_lock=0;
				} else {
					sendlights|=0x01;
					sendlights|=0x8000;
					num_lock=1;
				}
			} else if(k_pinfo->keys[0]==0x47){
				if(scroll_lock){
					sendlights&=~(uint8_t)4;
					sendlights|=0x8000;
					scroll_lock=0;
				} else {
					sendlights|=0x04;
					sendlights|=0x8000;
					scroll_lock=1;
				}
			} else {
				c = APP_MapKeyToUsage(k_pinfo,0);
				if (c != 0)
				{
					USR_KEYBRD_ProcessData(c);
				}
			}
		}
		for(i=0;i<6;i++){
				c = APP_MapKeyToUsage(k_pinfo,i);
				if (c != 0)	KeyDown[i]=c;
				else KeyDown[i]=0;
		}
		KeyDown[6]=(k_pinfo->lalt ? 1: 0) |
				(k_pinfo->lctrl ? 2: 0) |
				(k_pinfo->lgui ? 4: 0) |
				(k_pinfo->lshift ? 8: 0) |
				(k_pinfo->ralt ? 16: 0) |
				(k_pinfo->rctrl ? 32: 0) |
				(k_pinfo->rgui ? 64: 0) |
				(k_pinfo->rshift ? 128: 0);

		mycpy(&last_k_info,k_pinfo,sizeof(HID_KEYBD_Info_TypeDef));
		repeattime=Option.RepeatStart;
	} else {
		if(keytimer>repeattime){
			c = APP_MapKeyToUsage(&last_k_info,0);
			if (!(c == 0 || c==PDOWN || c==PUP || c==CTRLKEY('P') || c==CTRLKEY('L') || c==25 || c==F5 || c==F4 || c==CTRLKEY('T')/* || (markmode && (c==DEL || c==CTRLKEY(']')))*/))
			{
				USR_KEYBRD_ProcessData(c);
			}
			if(c==PDOWN || c==PUP || c==CTRLKEY('P') || c==CTRLKEY('L') || c==25 || c==F5  || c==F4 || c==CTRLKEY('T')/* || (markmode && (c==DEL || c==CTRLKEY(']')))*/)mymemset(&last_k_info,0,sizeof(HID_KEYBD_Info_TypeDef));
			keytimer=0;
			repeattime=Option.RepeatRate;
		}
	}
}


/**
  * @brief  Processes Keyboard data.
  * @param  data: Keyboard data to be displayed
  * @retval None
  */
void USR_KEYBRD_ProcessData(uint8_t data)
{
	if(data==0)return;
	if(data=='\n'){
		if(sendCRLF==3)USR_KEYBRD_ProcessData('\r');
		if(sendCRLF==2)data='\r';
	}
	if(data==keyselect && KeyInterrupt!=NULL){
		Keycomplete=1;
		return;
	}
	ConsoleRxBuf[ConsoleRxBufHead]  = data;   // store the byte in the ring buffer
	if(BreakKey && ConsoleRxBuf[ConsoleRxBufHead] == BreakKey) {// if the user wants to stop the progran
		MMAbort = true;                                         // set the flag for the interpreter to see
		ConsoleRxBufHead = ConsoleRxBufTail;                    // empty the buffer
		return;
	}
	ConsoleRxBufHead = (ConsoleRxBufHead + 1) % CONSOLE_RX_BUF_SIZE;     // advance the head of the queue
	if(ConsoleRxBufHead == ConsoleRxBufTail) {                           // if the buffer has overflowed
		ConsoleRxBufTail = (ConsoleRxBufTail + 1) % CONSOLE_RX_BUF_SIZE; // throw away the oldest char
	}
}
uint8_t APP_MapKeyToUsage(HID_KEYBD_Info_TypeDef *info, int keyno)
{
    uint8_t keyCode=info->keys[keyno];
    if(keyCode == USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CAPS_LOCK
                    || keyCode == USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SCROLL_LOCK
                    || keyCode == USB_HID_KEYBOARD_KEYPAD_KEYPAD_NUM_LOCK_AND_CLEAR) return 0;

    if(keyCode >= USB_HID_KEYBOARD_KEYPAD_KEYBOARD_A && keyCode <= USB_HID_KEYBOARD_KEYPAD_KEYBOARD_NON_US_FORWARD_SLASH_AND_PIPE)
    {
        if(info->ralt && Option.USBKeyboard==CONFIG_DE){
        	if(keyCode==0x24)return 0x7B;
        	else if(keyCode==0x25)return 0x5B;
        	else if(keyCode==0x26)return 0x5D;
        	else if(keyCode==0x27)return 0x7D;
        	else if(keyCode==0x2D)return 0x5C;
        	else if(keyCode==0x14)return 0x40;
        	else if(keyCode==0x64)return 124;
        }
        if(info->ralt && Option.USBKeyboard==CONFIG_FR){
			 if(keyCode==0x1F)     return 126;// ~
			 else if(keyCode==0x20)return 35; // #
			 else if(keyCode==0x21)return 123;// {
			 else if(keyCode==0x22)return 91; // [
			 else if(keyCode==0x23)return 124;// |
			 else if(keyCode==0x24)return 96 ;// `
			 else if(keyCode==0x25)return 92; // '\'
			 else if(keyCode==0x26)return 94; // ^
			 else if(keyCode==0x27)return 64; // @
			 else if(keyCode==0x2D)return 93; // ]
			 else if(keyCode==0x2E)return 125;// }
        }
        if(info->ralt && Option.USBKeyboard == CONFIG_ES){
			if(keyCode==0x35)     return 92;   // backslash
			else if(keyCode==0x1E)return 124;  // |
			else if(keyCode==0x08)return 0;    // €
			else if(keyCode==0x1F)return 64;   // @
			else if(keyCode==0x20)return 35;   // #
			else if(keyCode==0x21)return 0;    // ~
			else if(keyCode==0x2F)return 91;   // [
			else if(keyCode==0x30)return 93;   // ]
			else if(keyCode==0x31)return 125;  // }
			else if(keyCode==0x34)return 123;  // {
			}
        if(info->ralt && Option.USBKeyboard == CONFIG_BE){
			if(keyCode==0x64)     return 92;   // backslash
			else if(keyCode==0x20)return 35;  // |
			else if(keyCode==0x2F)return 91;    // €
			else if(keyCode==0x30)return 93;   // @
			else if(keyCode==0x31)return 96;   // #
			else if(keyCode==0x34)return 39;    // ~
			else if(keyCode==0x38)return 126;   // [
        }
        if(keyCode >= USB_HID_KEYBOARD_KEYPAD_KEYBOARD_A && keyCode <= USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Z && (info->lctrl || info->rctrl)) //Ctrl Key pressed for normal alpha key
        {
            return (keylayout[keyCode<<1]-96);
        }
        if(keyCode >=USB_HID_KEYBOARD_KEYPAD_KEYPAD_BACK_SLASH && keyCode<=USB_HID_KEYBOARD_KEYPAD_KEYPAD_PERIOD_AND_DELETE) //Key pressed on the numeric keypad
        {
        	if(num_lock)return (keylayout[keyCode<<1]);
        	else return (keylayout[(keyCode<<1)+1]);
        }
        if(keyCode >=USB_HID_KEYBOARD_KEYPAD_KEYBOARD_A && keyCode<=USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Z) //Alpha key pressed
        {
        	int toggle=1;
        	if(caps_lock)toggle=!toggle;
        	if(info->lshift || info->rshift)toggle=!toggle;
        	if(toggle)return (keylayout[keyCode<<1]);
            else return (keylayout[(keyCode<<1)+1]);
        } else { //remaining keys
        	if(!(info->lshift || info->rshift))return (keylayout[keyCode<<1]);
            else return (keylayout[(keyCode<<1)+1]);
        }

    }
    return 0;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
