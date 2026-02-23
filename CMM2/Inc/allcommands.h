#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
#define RADCONV   (MMFLOAT)57.2957795130823209L	  // Used when converting degrees -> radians and vice versa
#define PI_VALUE  (MMFLOAT)3.14159265358979323L
#define Rad(a)  (((MMFLOAT)a) / RADCONV)
typedef enum {
    MMHRES,
    MMVRES,
    MMVER,
    MMI2C,
	MMFONTHEIGHT,
	MMFONTWIDTH,
	MMHPOS,
	MMVPOS,
    MMERRNO,
    MMERRMSG,
	MMWATCHDOG,
	MMDEVICE,
	MMCMDLINE,
    MMWIDTH,
    MMHEIGHT,
	MMONEWIRE,
	MMFLAGS,
	MMESC,
	MMPOS,
    MMEND
} Operation;
extern const char* overlaid_functions[];extern volatile int vol_left, vol_right;
int getinttblpos(int tbl[],int pin);
//
void cmd_play(void);
void CloseAudio(int all);
void StopAudio(void);
void audioInterrupt(void);
void CheckAudio(void);
void PWMClose();
void SPIClose(void);
void SPI2Close(void);
//
void cmd_ds18b20(void);
void cmd_DAC(void);
void cmd_ADC(void);
void cmd_turtle(void);
void cmd_onewire(void);
void cmd_memory(void);
void cmd_pwm(void);
void cmd_open(void);
void cmd_close(void);
void cmd_spi(void);
void cmd_spi2(void);
void cmd_xmodem(void);
void cmd_sprite(void);
void cmd_clear(void);
void cmd_erase(void);
void cmd_continue(void);
void cmd_dim(void);
void cmd_do(void);
void cmd_else(void);
void cmd_end(void);
void cmd_endfun(void);
void cmd_endsub(void);
void cmd_error(void);
void cmd_exit(void);
void cmd_exitfor(void);
void cmd_for(void);
void cmd_subfun(void);
void cmd_gosub(void);
void cmd_goto(void);
void cmd_if(void);
void cmd_inc(void);
void cmd_input(void);
void cmd_let(void);
void cmd_lineinput(void);
void cmd_list(void);
void cmd_listfiles(void);
void cmd_loop(void);
void cmd_new(void);
void cmd_next(void);
void cmd_null(void);
void cmd_on(void);
void cmd_print(void);
void cmd_read(void);
void cmd_restore(void);
void cmd_return(void);
void cmd_run(void);
void cmd_trace(void);
void cmd_const(void);

#ifdef STRUCTENABLED
struct s_structdef; // Forward declaration
void cmd_type(void);
void cmd_endtype(void);
void cmd_struct(void);
void fun_struct(void);
const char *ParseStructMember( char *p, struct s_structdef *sd); // Returns NULL on success, error message on failure
int FindStructType(char *name);
int FindStructMember(int struct_idx, char *membername, int *member_type, int *member_offset, int *member_size, short *member_dims);
#endif

void cmd_select(void);
void cmd_case(void);
void cmd_mid(void);
void cmd_lmid(void);
void cmd_execute(void);
void cmd_call(void);
void cmd_debug(void);
void cmd_help(void);
void cmd_text(void);
void cmd_pixel(void);
void cmd_circle(void);
void cmd_line(void);
void cmd_box(void);
void cmd_rbox(void);
void cmd_polygon(void);
void cmd_triangle(void);
void cmd_blit(void);
void fun_pixel(void);
void cmd_page(void);
void cmd_clut(void);
void cmd_cls(void);
void cmd_font(void);
void cmd_colour(void);
void cmd_bezier(void);
void cmd_arc(void);
void cmd_image(void);
void cmd_framebuffer(void);
void cmd_3D(void);
void cmd_setpin(void);
void cmd_pulse(void);
void cmd_backlight(void);
void cmd_ctrlval(void);
void cmd_GUIpage(char *p);
void cmd_i2c(void);
void cmd_i2c2(void);
void cmd_i2c3(void);
void cmd_math(void);
void cmd_autosave(void);
//void cmd_autosave2(void);
void cmd_option(void);
void cmd_pause(void);
void cmd_timer(void);
void cmd_date(void);
void cmd_time(void);
void cmd_ireturn(void);
void cmd_poke(void);
void cmd_settick(void);
void cmd_var(void);
void cmd_watchdog(void);
void cmd_cpu(void);
void cmd_mode(void);
void cmd_cfunction(void);
void cmd_longString(void);
void cmd_Controller(void);
void cmd_sort(void);
void cmd_uSec(void);
void cmd_newedit(void);
void cmd_filemanager(void);
void cmd_guiMX170(void);
void cmd_gui(void);
void cmd_JumpToBootloader(void);
void cmd_pin(void);
void cmd_port(void);
void cmd_ir(void);
void cmd_csubinterrupt(void);
void cmd_WS2812(void);
void cmd_dht22(void);
void cmd_bitbang(void);
void cmd_load(void);
void cmd_mkdir(void);
void cmd_rmdir(void);
void cmd_chdir(void);
void cmd_kill(void);
void cmd_seek(void);
void cmd_copy(void);
void cmd_save(void);
void cmd_files(void);
void cmd_name(void);
void cmd_can(void);
#ifdef CMD16BIT
void cmd_redim(void);
void cmd_slice(void);
void cmd_insert(void);
void cmd_add(void);
void cmd_arrayset(void);
void cmd_fill(void);
void cmd_bezier(void);
void cmd_sync(void);
//void cmd_byte(void);
//void cmd_bit(void);
void cmd_bitbyteflag(void);
//void cmd_flag(void);
void cmd_flags(void);
#endif
//
void fun_port(void);
void fun_pin(void);
void fun_3D(void);
void fun_distance(void);
void fun_pulsin(void);
void fun_cwd(void);
void fun_dir(void);
void fun_rgb(void);
void fun_mmhres(void);
void fun_mmvres(void);
void fun_mmcharwidth(void);
void fun_mmcharheight(void);
void fun_mmhpos(void);
void fun_mmvpos(void);
void fun_clut(void);
void fun_getscanline(void);
void fun_at(void);
void fun_abs(void);
void fun_asc(void);
void fun_atn(void);
void fun_atan2(void);
void fun_base(void);
void fun_chr(void);
void fun_cint(void);
void fun_cos(void);
void fun_deg(void);
void fun_errno(void);
void fun_errmsg(void);
void fun_exp(void);
void fun_fix(void);
void fun_hex(void);
void fun_inkey(void);
void fun_instr(void);
void fun_int(void);
void fun_lcase(void);
void fun_left(void);
void fun_len(void);
void fun_log(void);
void fun_mid(void);
void fun_oct(void);
void fun_pi(void);
//void fun_pos(void);
void fun_rad(void);
void fun_right(void);
void fun_rnd(void);
void fun_sgn(void);
void fun_sin(void);
void fun_space(void);
void fun_sqr(void);
void fun_str(void);
void fun_string(void);
void fun_tab(void);
void fun_tan(void);
void fun_ucase(void);
void fun_val(void);
void fun_eval(void);
void fun_version(void);
void fun_asin(void);
void fun_acos(void);
void fun_field(void);
void fun_max(void);
void fun_min(void);
void fun_bin2str(void);
void fun_str2bin(void);
void fun_dummy(void);
void fun_test(void);
void fun_cmdline(void);
void fun_bound(void);
void fun_ternary(void);
void fun_call(void);
void fun_port(void);
void fun_pin(void);
void fun_3D(void);
void fun_distance(void);
void fun_pulsin(void);
void fun_msgbox(void);
void fun_ctrlval(void);
void fun_mmhpos(void);
void fun_mmvpos(void);
void fun_touch(void);
void fun_math(void);
void fun_timer(void);
void fun_date(void);
void fun_time(void);
void fun_device(void);
void fun_keydown(void);
void fun_peek(void);
void fun_restart(void);
void fun_day(void);
void fun_info(void);
void fun_json(void);
void fun_LLen(void);
void fun_LGetByte(void);
void fun_LGetStr(void);
void fun_LCompare(void);
void fun_LInstr(void);
void fun_epoch(void);
void fun_datetime(void);
void fun_nunchuck(void);
void fun_classic(void);
void fun_uSec(void);
void fun_format(void);
void fun_mouse(void);
void fun_tilde(void);
void fun_baudrate(void);
void fun_mmOW(void);
void fun_ds18b20(void);
void fun_eof(void);
void fun_loc(void);
void fun_lof(void);
void fun_inputstr(void);
void fun_spi(void);
void fun_spi2(void);
void fun_sprite(void);
void fun_trim(void);
void fun_byte(void);
//void fun_bit(void);
void fun_bitbyteflag(void);
void fun_flag(void);
void fun_linputstr(void);

//
void op_invalid(void);
void op_exp(void);
void op_mul(void);
void op_div(void);
void op_divint(void);
void op_add(void);
void op_subtract(void);
void op_mod(void);
void op_ne(void);
void op_gte(void);
void op_lte(void);
void op_lt(void);
void op_gt(void);
void op_equal(void);
void op_and(void);
void op_or(void);
void op_xor(void);
void op_not(void);
void op_shiftleft(void);
void op_shiftright(void);
void op_shiftright_unsigned(void);
void op_inv(void);

#endif




/**********************************************************************************
 All command tokens tokens (eg, PRINT, FOR, etc) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_COMMAND_TABLE

	{ "Play",        	T_CMD,				0, cmd_play	    },
	{ "Call",		T_CMD,				0, cmd_call	},
	{ "Clear",		T_CMD,				0, cmd_clear	},
	{ "Continue",   T_CMD,              0, cmd_continue	},
	{ "Data",		T_CMD,				0, cmd_null		},
	{ "MMDebug",	T_CMD,				0, cmd_debug	},
	{ "Dim",		T_CMD,				0, cmd_dim		},
	{ "Do",			T_CMD,				0, cmd_do		},

	{ "ElseIf",		T_CMD,				0, cmd_else		},
	{ "Case Else",	T_CMD,				0, cmd_case		},
	{ "Else",		T_CMD,				0, cmd_else		},

	{ "Select Case",T_CMD,				0, cmd_select	},
	{ "End Select",	T_CMD,				0, cmd_null		},
	{ "Case",		T_CMD,				0, cmd_case		},

	{ "End Function", T_CMD,            0, cmd_endfun	},
	{ "End Sub",    T_CMD,              0, cmd_return	},
	{ "EndIf",		T_CMD,				0, cmd_null		},
	{ "End",		T_CMD,				0, cmd_end		},

	{ "Exit For",   T_CMD,				0, cmd_exitfor	},
	{ "Exit Sub",   T_CMD,				0, cmd_return	},
	{ "Exit Function",  T_CMD,          0, cmd_endfun	},
	{ "Exit",		T_CMD,				0, cmd_exit		},

	{ "Erase",		T_CMD,				0, cmd_erase	},
	{ "Error",		T_CMD,				0, cmd_error	},
	{ "For",		T_CMD,				0, cmd_for		},
	{ "Function",   T_CMD,				0, cmd_subfun	},
	{ "GoSub",		T_CMD,				0, cmd_gosub	},
	{ "GoTo",		T_CMD,				0, cmd_goto		},
	{ "Help",		T_CMD,				0, cmd_help		},
	{ "If",			T_CMD,				0, cmd_if		},
	{ "Inc",		T_CMD,				0, cmd_inc		},
	{ "Line Input", T_CMD,				0, cmd_lineinput},
	{ "Input",		T_CMD,				0, cmd_input	},
	{ "Let",		T_CMD,				0, cmd_let		},
	{ "List",		T_CMD,				0, cmd_list		},
	{ "Local",		T_CMD,				0, cmd_dim		},
	{ "Loop",		T_CMD,				0, cmd_loop		},
	{ "New",		T_CMD,				0, cmd_new		},
	{ "Next",		T_CMD,				0, cmd_next		},
	{ "On",			T_CMD,				0, cmd_on		},
	{ "Print",		T_CMD,				0, cmd_print	},
	{ "Read",		T_CMD,				0, cmd_read		},
	{ "Rem",		T_CMD,				0, cmd_null,	},
	{ "Restore",    T_CMD,				0, cmd_restore	},
	{ "Return",		T_CMD,				0, cmd_return,	},
	{ "RUN",		T_CMD,				0, cmd_run		},
   	{ "Static",		T_CMD,				0, cmd_dim		},
	{ "Sub",		T_CMD,				0, cmd_subfun   },
	{ "Trace",		T_CMD,				0, cmd_trace	},
	{ "Var",	   	T_CMD,				0, cmd_var	    },
	{ "While",		T_CMD,				0, cmd_do		},
	{ "Const",		T_CMD,				0, cmd_const	},
	{ "MID$(",		T_CMD | T_FUN,		0, cmd_mid      },
	{ "LMid(",		T_CMD | T_FUN,      0, cmd_lmid     },
	{ "Execute",	T_CMD,				0, cmd_execute	},
	{ "Triangle",       T_CMD,                      0, cmd_triangle	},
	{ "Text",           T_CMD,                      0, cmd_text	},
	{ "Pixel",          T_CMD,                      0, cmd_pixel	},
	{ "Circle",         T_CMD,                      0, cmd_circle	},
	{ "Line",           T_CMD,                      0, cmd_line	},
	{ "Box",            T_CMD,                      0, cmd_box	},
	{ "RBox",           T_CMD,                      0, cmd_rbox	},
	{ "CLS",            T_CMD,                      0, cmd_cls	},
	{ "Font",           T_CMD,                      0, cmd_font	},
	{ "Colour",         T_CMD,                      0, cmd_colour	},
	{ "Page",    		T_CMD,                      0, cmd_page	},
	{ "Arc",            T_CMD,                      0, cmd_arc	},
	{ "Polygon",        T_CMD,                  	0, cmd_polygon	},
	{ "Map(",	 		T_CMD | T_FUN,				0, cmd_clut          },
	{ "Map",	 		T_CMD,						0, cmd_clut          },
	{ "Image",	 		T_CMD,						0, cmd_image          },
	{ "Framebuffer",	T_CMD,						0, cmd_framebuffer          },
	{ "Draw3D",         T_CMD,                      0, cmd_3D	},
	{ "Pin(",				T_CMD | T_FUN,		0, cmd_pin      },
	{ "SetPin",				T_CMD,			0, cmd_setpin       },
	{ "Pulse",				T_CMD,			0, cmd_pulse        },
	{ "Port(",				T_CMD | T_FUN,		0, cmd_port	    },
	{ "IR",                 T_CMD,			0, cmd_ir           },
	{ "Interrupt", 	    T_CMD,             	0, cmd_csubinterrupt},
	{ "Bitbang",            T_CMD,			0, cmd_bitbang      },
  	{ "CtrlVal(",       T_CMD | T_FUN,              0, cmd_ctrlval    },
	{ "I2C",	T_CMD,		0, cmd_i2c              },
	{ "I2C2",	T_CMD,		0, cmd_i2c2              },
	{ "I2C3",	T_CMD,		0, cmd_i2c3              },
	{ "Math",		T_CMD,				0, cmd_math		},
	{ "Save",		T_CMD,				0, cmd_save	    },
	{ "Load",		T_CMD,				0, cmd_load	    },
	{ "Mkdir",		T_CMD,				0, cmd_mkdir	},
	{ "Rmdir",		T_CMD,				0, cmd_rmdir	},
	{ "Chdir",		T_CMD,				0, cmd_chdir	},
	{ "Kill",		T_CMD,				0, cmd_kill	    },
	{ "Seek",		T_CMD,				0, cmd_seek     },
	{ "Files",		T_CMD,				0, cmd_files    },
	{ "Rename",		T_CMD,				0, cmd_name     },
	{ "Copy",		T_CMD,				0, cmd_copy     },
	{ "DAC",		T_CMD,				0, cmd_DAC		},
	{ "ADC",		T_CMD,				0, cmd_ADC		},
	{ "Turtle",		T_CMD,				0, cmd_turtle		},
	{ "Memory",		T_CMD,				0, cmd_memory	},
	{ "AutoSave",		T_CMD,				0, cmd_autosave	},
	{ "Option",			T_CMD,				0, cmd_option	},
	{ "Pause",			T_CMD,				0, cmd_pause	},
	{ "Date$",			T_CMD | T_FUN,      0, cmd_date		},
	{ "Time$",			T_CMD | T_FUN,      0, cmd_time		},
	{ "IReturn",		T_CMD,				0, cmd_ireturn 	},
	{ "Poke",			T_CMD,				0, cmd_poke		},
	{ "SetTick",		T_CMD,				0, cmd_settick 	},
	{ "WatchDog",		T_CMD,				0, cmd_watchdog	},
	{ "CPU",			T_CMD,				0, cmd_cpu 	},
	{ "Sort",			T_CMD,				0, cmd_sort 	},
   	{ "DefineFont",     T_CMD,				0, cmd_cfunction},
   	{ "End DefineFont", T_CMD,				0, cmd_null 	},
	{ "LongString",		T_CMD,				0, cmd_longString	},
	{ "Mode",			T_CMD,				0, cmd_mode		},
	{ "Controller",		T_CMD,				0, cmd_Controller },
	{ "Timer",			T_CMD | T_FUN,		0, cmd_uSec		},
	{ "Edit",			T_CMD,				0, cmd_newedit },
	{ "Update Firmware",T_CMD,				0, cmd_JumpToBootloader },
	{ "CSub",           T_CMD,              0, cmd_cfunction},
	{ "End CSub",       T_CMD,              0, cmd_null     },
#ifdef CMD16BIT
	{ "CFunction",           T_CMD,              0, cmd_cfunction},
	{ "End CFunction",       T_CMD,              0, cmd_null     },
#endif
	{ "GUI",            T_CMD,              0, cmd_gui        },
	{ "OneWire",	    T_CMD,		0, cmd_onewire      },
	{ "TEMPR START",    T_CMD,	0, cmd_ds18b20      },
	{ "PWM",	    	T_CMD,				0, cmd_pwm	},
	{ "Servo",	    	T_CMD,				0, cmd_pwm	},
	{ "Open",		    T_CMD,				0, cmd_open		},
	{ "Close",		    T_CMD,				0, cmd_close	},
	{ "SPI",	        T_CMD,				0, cmd_spi	},
	{ "SPI2",	        T_CMD,				0, cmd_spi2	},
   	{ "Blit",           T_CMD,              0, cmd_blit     },
	{ "XModem",		    T_CMD,				0, cmd_xmodem	},
	{ "CAN",	        T_CMD,				0, cmd_can	  },
#ifdef CMD16BIT
	{ "ReDim",          T_CMD,              0, cmd_redim},
	{"Array Slice",     T_CMD,              0, cmd_slice},
	{"Array Insert",    T_CMD,              0, cmd_insert},
	{"Array Add",       T_CMD,              0, cmd_add},
	{"Array Set",       T_CMD,              0, cmd_arrayset},
	{ "Fill",           T_CMD,              0, cmd_fill    },
	{ "Bezier",         T_CMD,              0, cmd_bezier    },
	{ "SYNC",           T_CMD,			    0, cmd_sync    },
	{ "WS2812",         T_CMD,              0, cmd_WS2812},
//	{ "Byte(",          T_CMD | T_FUN, 0, cmd_byte},
//	{ "Flag(",          T_CMD | T_FUN, 0, cmd_flag},
//	{ "Bit(",           T_CMD | T_FUN, 0, cmd_bit},
	{ "~BBF(",           T_CMD | T_FUN, 0, cmd_bitbyteflag},
	{ "Flags",          T_CMD | T_FUN, 0, cmd_flags},
#endif
#ifdef STRUCTENABLED
	{"Type",            T_CMD,         0, cmd_type},
	{"End Type",        T_CMD,         0, cmd_endtype},
	{"Struct",          T_CMD,         0, cmd_struct},
#endif
	{ "",   0,                  0, cmd_null,    }                   // this dummy entry is always at the end
#endif


/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE
	// These 4 operators mustn't be moved
	{ "Not",		T_OPER | T_NBR | T_INT,			3, op_not		},
	{ "INV",		T_OPER | T_NBR | T_INT,			3, op_inv		},
	{ "+",			T_OPER | T_NBR | T_INT | T_STR, 2, op_add		},
	{ "-",			T_OPER | T_NBR | T_INT,		2, op_subtract          },
	//
	{ "^",			T_OPER | T_NBR | T_INT,		0, op_exp		},
	{ "*",			T_OPER | T_NBR | T_INT,		1, op_mul		},
	{ "/",			T_OPER | T_NBR,                 1, op_div		},
	{ "\\",			T_OPER | T_INT,			1, op_divint            },
	{ "MOD",		T_OPER | T_INT,			1, op_mod		},
	{ "<<",			T_OPER | T_INT,                 4, op_shiftleft		},      // this must come before less than (<)
	{ ">>>",		T_OPER | T_INT,                 4, op_shiftright	},      // this must come before greater than (>) and shift right
	{ ">>",			T_OPER | T_INT,                 4, op_shiftright_unsigned	},      // this must come before greater than (>)
	{ "<>",			T_OPER | T_NBR | T_INT | T_STR, 5, op_ne		},      // this must come before less than (<)
	{ ">=",			T_OPER | T_NBR | T_INT | T_STR, 5, op_gte		},      // this must come before greater than (>)
	{ "<=",			T_OPER | T_NBR | T_INT | T_STR, 5, op_lte		},      // this must come before less than (<)
	{ "<",			T_OPER | T_NBR | T_INT | T_STR, 5, op_lt		},
	{ ">",			T_OPER | T_NBR | T_INT | T_STR, 5, op_gt		},
	{ "=",			T_OPER | T_NBR | T_INT | T_STR, 6, op_equal		},
	{ "AND",		T_OPER | T_INT,			7, op_and		},
	{ "OR",			T_OPER | T_INT,			7, op_or		},
	{ "XOR",		T_OPER | T_INT,			7, op_xor		},
	{ "For",		T_NA,				0, op_invalid	},
	{ "Else",		T_NA,				0, op_invalid	},
	{ "GoSub",		T_NA,				0, op_invalid	},
	{ "GoTo",		T_NA,				0, op_invalid	},
	{ "Step",		T_NA,				0, op_invalid	},
	{ "Then",		T_NA,				0, op_invalid	},
	{ "To",			T_NA,				0, op_invalid	},
	{ "Until",		T_NA,				0, op_invalid	},
	{ "While",		T_NA,				0, op_invalid	},
	{ "RGB(",           T_FUN | T_INT,		0, fun_rgb	        },
//	{ "MM.HRes",	    T_FNA | T_INT,		0, fun_mmhres 	    },
//	{ "MM.VRes",	    T_FNA | T_INT,		0, fun_mmvres 	    },
 	{ "Pixel(",	        T_FUN | T_INT,		0, fun_pixel,	    },
// 	{ "DRAW3D(",	    T_FUN | T_INT,		0, fun_3D,	    },
	{ "DRAW3D(",        T_FUN | T_NBR,      0, fun_3D,      },
	{ "Map(",	        T_FUN | T_INT,		0, fun_clut,	    },
	{ "Getscanline",	T_FNA | T_INT,		0, fun_getscanline 	    },
	{ "@(",				T_FUN | T_STR,		0, fun_at		},
	{ "Pin(",		T_FUN | T_NBR | T_INT,	0, fun_pin		},
	{ "Port(",		T_FUN | T_INT,		0, fun_port		},
	{ "Distance(",		T_FUN | T_NBR,		0, fun_distance		},
	{ "Pulsin(",		T_FUN | T_INT,		0, fun_pulsin		},
	{ "Cwd$",		T_FNA | T_STR,		0, fun_cwd		},
	{ "Dir$(",		T_FUN | T_STR,		0, fun_dir		},
	{ "ACos(",		T_FUN  | T_NBR, 	    0, fun_acos		},
	{ "Abs(",		T_FUN  | T_NBR | T_INT, 0, fun_abs		},
	{ "Asc(",		T_FUN  | T_INT,			0, fun_asc		},
	{ "ASin(",		T_FUN  | T_NBR,			0, fun_asin		},
	{ "Atn(",		T_FUN  | T_NBR,			0, fun_atn		},
	{ "Atan2(",		T_FUN  | T_NBR,			0, fun_atan2	},
	{ "Base$(",		T_FUN  | T_STR,			0, fun_base		},
	{ "Bound(",		T_FUN  | T_INT,			0, fun_bound	},
	{ "Chr$(",		T_FUN  | T_STR,			0, fun_chr,		},
	{ "Choice(",	T_FUN | T_STR | T_INT | T_NBR,		0, fun_ternary	},
	{ "Cint(",		T_FUN  | T_INT,			0, fun_cint		},
	{ "Cos(",		T_FUN  | T_NBR,			0, fun_cos		},
	{ "Deg(",		T_FUN  | T_NBR,			0, fun_deg		},
	{ "Exp(",		T_FUN  | T_NBR,			0, fun_exp		},
	{ "Fix(",		T_FUN  | T_INT,			0, fun_fix		},
	{ "Field$(",    T_FUN  | T_STR,			0, fun_field    },
	{ "Inkey$",		T_FNA  | T_STR,         0, fun_inkey    },
	{ "Instr(",		T_FUN  | T_INT,			0, fun_instr    },
	{ "Int(",		T_FUN  | T_INT,			0, fun_int		},
	{ "LCase$(",    T_FUN  | T_STR,			0, fun_lcase    },
	{ "Left$(",		T_FUN  | T_STR,			0, fun_left		},
	{ "Len(",		T_FUN  | T_INT,			0, fun_len		},
	{ "Log(",		T_FUN  | T_NBR,			0, fun_log		},
	{ "Mid$(",		T_FUN  | T_STR,			0, fun_mid		},
	{ "Pi",			T_FNA  | T_NBR,			0, fun_pi		},
	//{ "Pos",		T_FNA  | T_INT,         0, fun_pos		},
	{ "Rad(",		T_FUN  | T_NBR,			0, fun_rad		},
	{ "Right$(",    T_FUN  | T_STR,			0, fun_right    },
	{ "Rnd(",		T_FUN  | T_NBR,			0, fun_rnd		},      // this must come before Rnd - without bracket
	{ "Rnd",		T_FNA  | T_NBR,			0, fun_rnd		},      // this must come after Rnd(
	{ "Sgn(",		T_FUN  | T_INT,			0, fun_sgn		},
	{ "Sin(",		T_FUN  | T_NBR,			0, fun_sin		},
	{ "Space$(",    T_FUN  | T_STR,			0, fun_space    },
	{ "Sqr(",		T_FUN  | T_NBR,			0, fun_sqr		},
	{ "Str$(",		T_FUN  | T_STR,			0, fun_str		},
	{ "String$(",   T_FUN  | T_STR,			0, fun_string   },
	{ "Tab(",		T_FUN  | T_STR,         0, fun_tab,		},
	{ "Tan(",		T_FUN  | T_NBR,			0, fun_tan		},
	{ "UCase$(",    T_FUN  | T_STR,			0, fun_ucase    },
	{ "Val(",		T_FUN  | T_NBR | T_INT,	0, fun_val		},
	{ "Eval(",		T_FUN  | T_NBR | T_INT | T_STR,	0, fun_eval		},
	{ "Max(",		T_FUN  | T_NBR,			0, fun_max		},
	{ "Min(",		T_FUN  | T_NBR,			0, fun_min		},
	{ "Bin2str$(",  T_FUN  | T_STR,			0, fun_bin2str  },
	{ "Str2bin(",	T_FUN  | T_NBR | T_INT,	0, fun_str2bin	},
//	{ "MM.CmdLine$",T_FNA  | T_STR,			0, fun_cmdline	},
	{ "Call(",		T_FUN | T_STR | T_INT | T_NBR,		0, fun_call	},
  	{ "MsgBox(",        T_FUN | T_INT,              0, fun_msgbox     },
  	{ "CtrlVal(",       T_FUN | T_NBR | T_STR,      0, fun_ctrlval    },
  	{ "Click(",       T_FUN | T_INT,        0, fun_touch  },
	{ "Math(",	    T_FUN | T_NBR | T_INT,		0, fun_math	},
	{ "JSON$(",		T_FUN | T_STR,          0, fun_json		},
	{ "LInStr(",		T_FUN | T_INT,		0, fun_LInstr		},
	{ "LCompare(",		T_FUN | T_INT,		0, fun_LCompare		},
	{ "LLen(",		T_FUN | T_INT,		0, fun_LLen		},
	{ "Nunchuk(",		T_FUN | T_INT,		0, fun_nunchuck		},
	{ "Classic(",		T_FUN | T_INT,		0, fun_classic		},
	{ "LGetByte(",		T_FUN | T_INT,		0, fun_LGetByte		},
	{ "LGetStr$(",		T_FUN | T_STR,		0, fun_LGetStr		},
	{ "As",		T_NA,			0, op_invalid	},
	{ "Date$",	T_FNA | T_STR,		0, fun_date	},
	{ "Day$(",	T_FUN | T_STR,		0, fun_day	},
	{ "Peek(",		T_FUN  | T_INT | T_STR | T_NBR,			0, fun_peek		},
	{ "Time$",	T_FNA | T_STR,		0, fun_time	},
//	{ "MM.Watchdog",T_FNA | T_INT,		0, fun_restart	},
	{ "Epoch(",		T_FUN  | T_INT,			0, fun_epoch		},
	{ "DateTime$(",		T_FUN | T_STR,		0, fun_datetime		},
	{ "MM.Info(",		T_FUN | T_STR | T_INT | T_NBR,		0, fun_info		},
	{ "KeyDown(",    T_FUN | T_INT,		0, fun_keydown	},
	{ "Timer",	T_FNA | T_NBR,		0, fun_uSec	},
	{ "Mouse(",    T_FUN | T_INT,		0, fun_mouse	},
	{ "Format$(",	T_FUN  | T_STR,			0, fun_format	},
	{ "TEMPR(",	T_FUN | T_NBR,	0, fun_ds18b20      },
	{ "Baudrate(",	    T_FUN | T_INT,		0, fun_baudrate 	},
	{ "Eof(",		T_FUN | T_INT,		0, fun_eof		},
	{ "Loc(",		T_FUN | T_INT,		0, fun_loc		},
	{ "Lof(",		T_FUN | T_INT,		0, fun_lof		},
	{ "Input$(",            T_FUN | T_STR,		0, fun_inputstr         },
	{ "SPI(",	T_FUN | T_INT,		0, fun_spi,	},
	{ "SPI2(",	T_FUN | T_INT,		0, fun_spi2,	},
	{ "sprite(",	    T_FUN | T_INT | T_NBR,		0, fun_sprite 	},
	{ "~(",	    T_FUN | T_INT | T_NBR | T_STR ,		0, fun_tilde },
	{ "Trim$(",            T_FUN | T_STR,		0, fun_trim        },
	//{ "Flag(",             T_FUN | T_INT, 0,   fun_flag},
	//{ "Bit(",              T_FUN | T_INT, 0,   fun_bit},
	{ "~BBF(",              T_FUN | T_INT, 0,   fun_bitbyteflag},
	//{ "Byte(",             T_FUN | T_INT, 0,   fun_byte},
	{ "LInput(",           T_FUN | T_INT, 0,   fun_linputstr},
#ifdef STRUCTENABLED
	{ "Struct(",           T_FUN | T_INT, 0,   fun_struct},
#endif

	{ "",   0,                  0, cmd_null,    }                   // this dummy entry is always at the end
#endif


