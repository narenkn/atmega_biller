   1               		.file	"flash.cc"
   2               	__SREG__ = 0x3f
   3               	__SP_H__ = 0x3e
   4               	__SP_L__ = 0x3d
   5               	__CCP__  = 0x34
   6               	__tmp_reg__ = 0
   7               	__zero_reg__ = 1
  15               	.Ltext0:
  16               	.global	_ZN8SPIFlash6selectEv
  18               	_ZN8SPIFlash6selectEv:
  19               	.LFB10:
  20               	.LM1:
  21               	.LVL0:
  22               	/* prologue: function */
  23               	/* frame size = 0 */
  24 0000 FC01      		movw r30,r24
  25               	.LM2:
  26 0002 A2E6      		ldi r26,lo8(98)
  27 0004 B0E0      		ldi r27,hi8(98)
  28 0006 2C91      		ld r18,X
  29 0008 81E0      		ldi r24,lo8(1)
  30 000a 90E0      		ldi r25,hi8(1)
  31               	.LVL1:
  32 000c 0080      		ld r0,Z
  33 000e 00C0      		rjmp 2f
  34 0010 880F      	1:	lsl r24
  35 0012 991F      		rol r25
  36 0014 0A94      	2:	dec r0
  37 0016 02F4      		brpl 1b
  38 0018 8F70      		andi r24,lo8(15)
  39 001a 8095      		com r24
  40 001c 8223      		and r24,r18
  41 001e 8C93      		st X,r24
  42               	/* epilogue start */
  43               	.LM3:
  44 0020 0895      		ret
  45               	.LFE10:
  47               	.global	_ZN8SPIFlash8unselectEv
  49               	_ZN8SPIFlash8unselectEv:
  50               	.LFB11:
  51               	.LM4:
  52               	.LVL2:
  53               	/* prologue: function */
  54               	/* frame size = 0 */
  55               	.LM5:
  56 0022 E2E6      		ldi r30,lo8(98)
  57 0024 F0E0      		ldi r31,hi8(98)
  58 0026 8081      		ld r24,Z
  59               	.LVL3:
  60 0028 8F60      		ori r24,lo8(15)
  61 002a 8083      		st Z,r24
  62               	/* epilogue start */
  63               	.LM6:
  64 002c 0895      		ret
  65               	.LFE11:
  67               	.global	_ZN8SPIFlash10readStatusEv
  69               	_ZN8SPIFlash10readStatusEv:
  70               	.LFB17:
  71               	.LM7:
  72               	.LVL4:
  73 002e 0F93      		push r16
  74 0030 1F93      		push r17
  75               	/* prologue: function */
  76               	/* frame size = 0 */
  77 0032 FC01      		movw r30,r24
  78               	.LBB39:
  79               	.LBB40:
  80               	.LBB41:
  81               	.LM8:
  82 0034 02E6      		ldi r16,lo8(98)
  83 0036 10E0      		ldi r17,hi8(98)
  84 0038 D801      		movw r26,r16
  85 003a 2C91      		ld r18,X
  86 003c 81E0      		ldi r24,lo8(1)
  87 003e 90E0      		ldi r25,hi8(1)
  88               	.LVL5:
  89 0040 0080      		ld r0,Z
  90 0042 00C0      		rjmp 2f
  91 0044 880F      	1:	lsl r24
  92 0046 991F      		rol r25
  93 0048 0A94      	2:	dec r0
  94 004a 02F4      		brpl 1b
  95 004c 8F70      		andi r24,lo8(15)
  96 004e 8095      		com r24
  97 0050 8223      		and r24,r18
  98 0052 8C93      		st X,r24
  99               	.LBE41:
 100               	.LBE40:
 101               	.LM9:
 102 0054 85E0      		ldi r24,lo8(5)
 103 0056 0E94 0000 		call _Z11spiTransmith
 104               	.LVL6:
 105               	.LM10:
 106 005a 80E0      		ldi r24,lo8(0)
 107 005c 0E94 0000 		call _Z11spiTransmith
 108               	.LBB42:
 109               	.LBB43:
 110               	.LM11:
 111 0060 F801      		movw r30,r16
 112 0062 9081      		ld r25,Z
 113 0064 9F60      		ori r25,lo8(15)
 114 0066 9083      		st Z,r25
 115               	/* epilogue start */
 116               	.LBE43:
 117               	.LBE42:
 118               	.LBE39:
 119               	.LM12:
 120 0068 1F91      		pop r17
 121 006a 0F91      		pop r16
 122 006c 0895      		ret
 123               	.LFE17:
 125               	.global	_ZN8SPIFlash4busyEv
 127               	_ZN8SPIFlash4busyEv:
 128               	.LFB16:
 129               	.LM13:
 130               	.LVL7:
 131               	/* prologue: function */
 132               	/* frame size = 0 */
 133               	.LM14:
 134 006e 0E94 0000 		call _ZN8SPIFlash10readStatusEv
 135               	.LVL8:
 136               	.LM15:
 137 0072 8170      		andi r24,lo8(1)
 138               	/* epilogue start */
 139 0074 0895      		ret
 140               	.LFE16:
 142               	.global	_ZN8SPIFlash7commandEhb
 144               	_ZN8SPIFlash7commandEhb:
 145               	.LFB15:
 146               	.LM16:
 147               	.LVL9:
 148 0076 1F93      		push r17
 149 0078 CF93      		push r28
 150 007a DF93      		push r29
 151               	/* prologue: function */
 152               	/* frame size = 0 */
 153 007c EC01      		movw r28,r24
 154 007e 162F      		mov r17,r22
 155               	.LM17:
 156 0080 4423      		tst r20
 157 0082 01F0      		breq .L12
 158               	.LVL10:
 159               	.LM18:
 160 0084 66E0      		ldi r22,lo8(6)
 161 0086 40E0      		ldi r20,lo8(0)
 162               	.LVL11:
 163 0088 0E94 0000 		call _ZN8SPIFlash7commandEhb
 164               	.LBB44:
 165               	.LBB45:
 166               	.LM19:
 167 008c 8091 6200 		lds r24,98
 168 0090 8F60      		ori r24,lo8(15)
 169 0092 8093 6200 		sts 98,r24
 170               	.LVL12:
 171               	.L12:
 172               	.LBE45:
 173               	.LBE44:
 174               	.LM20:
 175 0096 CE01      		movw r24,r28
 176 0098 0E94 0000 		call _ZN8SPIFlash4busyEv
 177               	.LVL13:
 178 009c 8823      		tst r24
 179 009e 01F4      		brne .L12
 180               	.LBB46:
 181               	.LBB47:
 182               	.LM21:
 183 00a0 2091 6200 		lds r18,98
 184 00a4 81E0      		ldi r24,lo8(1)
 185 00a6 90E0      		ldi r25,hi8(1)
 186 00a8 0880      		ld r0,Y
 187 00aa 00C0      		rjmp 2f
 188 00ac 880F      	1:	lsl r24
 189 00ae 991F      		rol r25
 190 00b0 0A94      	2:	dec r0
 191 00b2 02F4      		brpl 1b
 192 00b4 8F70      		andi r24,lo8(15)
 193 00b6 8095      		com r24
 194 00b8 8223      		and r24,r18
 195 00ba 8093 6200 		sts 98,r24
 196               	.LBE47:
 197               	.LBE46:
 198               	.LM22:
 199 00be 812F      		mov r24,r17
 200 00c0 0E94 0000 		call _Z11spiTransmith
 201               	/* epilogue start */
 202               	.LM23:
 203 00c4 DF91      		pop r29
 204 00c6 CF91      		pop r28
 205               	.LVL14:
 206 00c8 1F91      		pop r17
 207               	.LVL15:
 208 00ca 0895      		ret
 209               	.LFE15:
 211               	.global	_ZN8SPIFlash6wakeupEv
 213               	_ZN8SPIFlash6wakeupEv:
 214               	.LFB22:
 215               	.LM24:
 216               	.LVL16:
 217               	/* prologue: function */
 218               	/* frame size = 0 */
 219               	.LM25:
 220 00cc 6BEA      		ldi r22,lo8(-85)
 221 00ce 40E0      		ldi r20,lo8(0)
 222 00d0 0E94 0000 		call _ZN8SPIFlash7commandEhb
 223               	.LVL17:
 224               	.LBB48:
 225               	.LBB49:
 226               	.LM26:
 227 00d4 E2E6      		ldi r30,lo8(98)
 228 00d6 F0E0      		ldi r31,hi8(98)
 229 00d8 8081      		ld r24,Z
 230 00da 8F60      		ori r24,lo8(15)
 231 00dc 8083      		st Z,r24
 232               	/* epilogue start */
 233               	.LBE49:
 234               	.LBE48:
 235               	.LM27:
 236 00de 0895      		ret
 237               	.LFE22:
 239               	.global	_ZN8SPIFlash5sleepEv
 241               	_ZN8SPIFlash5sleepEv:
 242               	.LFB21:
 243               	.LM28:
 244               	.LVL18:
 245               	/* prologue: function */
 246               	/* frame size = 0 */
 247               	.LM29:
 248 00e0 69EB      		ldi r22,lo8(-71)
 249 00e2 40E0      		ldi r20,lo8(0)
 250 00e4 0E94 0000 		call _ZN8SPIFlash7commandEhb
 251               	.LVL19:
 252               	.LBB50:
 253               	.LBB51:
 254               	.LM30:
 255 00e8 E2E6      		ldi r30,lo8(98)
 256 00ea F0E0      		ldi r31,hi8(98)
 257 00ec 8081      		ld r24,Z
 258 00ee 8F60      		ori r24,lo8(15)
 259 00f0 8083      		st Z,r24
 260               	/* epilogue start */
 261               	.LBE51:
 262               	.LBE50:
 263               	.LM31:
 264 00f2 0895      		ret
 265               	.LFE21:
 267               	.global	_ZN8SPIFlash9chipEraseEh
 269               	_ZN8SPIFlash9chipEraseEh:
 270               	.LFB19:
 271               	.LM32:
 272               	.LVL20:
 273               	/* prologue: function */
 274               	/* frame size = 0 */
 275 00f4 FC01      		movw r30,r24
 276               	.LM33:
 277 00f6 6083      		st Z,r22
 278               	.LM34:
 279 00f8 60E6      		ldi r22,lo8(96)
 280               	.LVL21:
 281 00fa 41E0      		ldi r20,lo8(1)
 282 00fc 0E94 0000 		call _ZN8SPIFlash7commandEhb
 283               	.LVL22:
 284               	.LBB52:
 285               	.LBB53:
 286               	.LM35:
 287 0100 E2E6      		ldi r30,lo8(98)
 288 0102 F0E0      		ldi r31,hi8(98)
 289 0104 8081      		ld r24,Z
 290 0106 8F60      		ori r24,lo8(15)
 291 0108 8083      		st Z,r24
 292               	/* epilogue start */
 293               	.LBE53:
 294               	.LBE52:
 295               	.LM36:
 296 010a 0895      		ret
 297               	.LFE19:
 299               	.global	_ZN8SPIFlash12blockErase4KEj
 301               	_ZN8SPIFlash12blockErase4KEj:
 302               	.LFB20:
 303               	.LM37:
 304               	.LVL23:
 305 010c 0F93      		push r16
 306 010e 1F93      		push r17
 307               	/* prologue: function */
 308               	/* frame size = 0 */
 309 0110 FC01      		movw r30,r24
 310 0112 8B01      		movw r16,r22
 311               	.LM38:
 312 0114 812F      		mov r24,r17
 313               	.LVL24:
 314 0116 8295      		swap r24
 315 0118 8695      		lsr r24
 316 011a 8695      		lsr r24
 317 011c 8370      		andi r24,lo8(3)
 318 011e 8083      		st Z,r24
 319               	.LM39:
 320 0120 000F      		lsl r16
 321 0122 111F      		rol r17
 322 0124 000F      		lsl r16
 323 0126 111F      		rol r17
 324               	.LVL25:
 325               	.LM40:
 326 0128 CF01      		movw r24,r30
 327 012a 60E2      		ldi r22,lo8(32)
 328 012c 41E0      		ldi r20,lo8(1)
 329 012e 0E94 0000 		call _ZN8SPIFlash7commandEhb
 330               	.LVL26:
 331               	.LM41:
 332 0132 812F      		mov r24,r17
 333               	.LVL27:
 334 0134 0E94 0000 		call _Z11spiTransmith
 335               	.LM42:
 336 0138 802F      		mov r24,r16
 337 013a 0E94 0000 		call _Z11spiTransmith
 338               	.LM43:
 339 013e 80E0      		ldi r24,lo8(0)
 340 0140 0E94 0000 		call _Z11spiTransmith
 341               	.LBB54:
 342               	.LBB55:
 343               	.LM44:
 344 0144 E2E6      		ldi r30,lo8(98)
 345 0146 F0E0      		ldi r31,hi8(98)
 346 0148 8081      		ld r24,Z
 347 014a 8F60      		ori r24,lo8(15)
 348 014c 8083      		st Z,r24
 349               	/* epilogue start */
 350               	.LBE55:
 351               	.LBE54:
 352               	.LM45:
 353 014e 1F91      		pop r17
 354               	.LVL28:
 355 0150 0F91      		pop r16
 356               	.LVL29:
 357 0152 0895      		ret
 358               	.LFE20:
 360               	.global	_ZN8SPIFlash10writeBytesEjPhj
 362               	_ZN8SPIFlash10writeBytesEjPhj:
 363               	.LFB18:
 364               	.LM46:
 365               	.LVL30:
 366 0154 2F92      		push r2
 367 0156 3F92      		push r3
 368 0158 4F92      		push r4
 369 015a 5F92      		push r5
 370 015c 6F92      		push r6
 371 015e 7F92      		push r7
 372 0160 8F92      		push r8
 373 0162 9F92      		push r9
 374 0164 AF92      		push r10
 375 0166 BF92      		push r11
 376 0168 CF92      		push r12
 377 016a DF92      		push r13
 378 016c EF92      		push r14
 379 016e FF92      		push r15
 380 0170 0F93      		push r16
 381 0172 1F93      		push r17
 382 0174 CF93      		push r28
 383 0176 DF93      		push r29
 384               	/* prologue: function */
 385               	/* frame size = 0 */
 386 0178 3C01      		movw r6,r24
 387 017a 2A01      		movw r4,r20
 388 017c 322E      		mov r3,r18
 389 017e 232E      		mov r2,r19
 390               	.LVL31:
 391               	.LBB56:
 392               	.LM47:
 393 0180 872F      		mov r24,r23
 394               	.LVL32:
 395 0182 8295      		swap r24
 396 0184 8695      		lsr r24
 397 0186 8695      		lsr r24
 398 0188 8370      		andi r24,lo8(3)
 399 018a F301      		movw r30,r6
 400 018c 8083      		st Z,r24
 401               	.LM48:
 402 018e 4B01      		movw r8,r22
 403 0190 880C      		lsl r8
 404 0192 991C      		rol r9
 405 0194 880C      		lsl r8
 406 0196 991C      		rol r9
 407               	.LVL33:
 408 0198 C901      		movw r24,r18
 409               	.LVL34:
 410 019a 7C01      		movw r14,r24
 411               	.LVL35:
 412 019c AA24      		clr r10
 413 019e BB24      		clr r11
 414               	.LVL36:
 415 01a0 00C0      		rjmp .L23
 416               	.LVL37:
 417               	.L27:
 418               	.LBB57:
 419               	.LM49:
 420 01a2 C301      		movw r24,r6
 421 01a4 62E0      		ldi r22,lo8(2)
 422 01a6 41E0      		ldi r20,lo8(1)
 423 01a8 0E94 0000 		call _ZN8SPIFlash7commandEhb
 424               	.LM50:
 425 01ac 892D      		mov r24,r9
 426 01ae 0E94 0000 		call _Z11spiTransmith
 427               	.LM51:
 428 01b2 882D      		mov r24,r8
 429 01b4 0E94 0000 		call _Z11spiTransmith
 430               	.LM52:
 431 01b8 80E0      		ldi r24,lo8(0)
 432 01ba 0E94 0000 		call _Z11spiTransmith
 433               	.LM53:
 434 01be 8701      		movw r16,r14
 435 01c0 F1E0      		ldi r31,lo8(257)
 436 01c2 EF16      		cp r14,r31
 437 01c4 F1E0      		ldi r31,hi8(257)
 438 01c6 FF06      		cpc r15,r31
 439 01c8 00F0      		brlo .L24
 440 01ca 00E0      		ldi r16,lo8(256)
 441 01cc 11E0      		ldi r17,hi8(256)
 442               	.L24:
 443 01ce 6201      		movw r12,r4
 444 01d0 CA0C      		add r12,r10
 445 01d2 DB1C      		adc r13,r11
 446 01d4 C0E0      		ldi r28,lo8(0)
 447 01d6 D0E0      		ldi r29,hi8(0)
 448 01d8 00C0      		rjmp .L25
 449               	.L26:
 450               	.LBB58:
 451               	.LM54:
 452 01da F601      		movw r30,r12
 453 01dc 8191      		ld r24,Z+
 454 01de 6F01      		movw r12,r30
 455 01e0 0E94 0000 		call _Z11spiTransmith
 456               	.LM55:
 457 01e4 2196      		adiw r28,1
 458               	.L25:
 459 01e6 C017      		cp r28,r16
 460 01e8 D107      		cpc r29,r17
 461 01ea 00F0      		brlo .L26
 462               	.LBE58:
 463               	.LBB59:
 464               	.LBB60:
 465               	.LM56:
 466 01ec 8091 6200 		lds r24,98
 467 01f0 8F60      		ori r24,lo8(15)
 468 01f2 8093 6200 		sts 98,r24
 469               	.LBE60:
 470               	.LBE59:
 471               	.LM57:
 472 01f6 A00E      		add r10,r16
 473 01f8 B11E      		adc r11,r17
 474               	.LM58:
 475 01fa E01A      		sub r14,r16
 476 01fc F10A      		sbc r15,r17
 477 01fe 0894      		sec
 478 0200 811C      		adc r8,__zero_reg__
 479 0202 911C      		adc r9,__zero_reg__
 480               	.LVL38:
 481               	.L23:
 482               	.LM59:
 483 0204 E114      		cp r14,__zero_reg__
 484 0206 F104      		cpc r15,__zero_reg__
 485 0208 01F4      		brne .L27
 486               	.LBE57:
 487               	.LBE56:
 488               	.LM60:
 489 020a 832D      		mov r24,r3
 490 020c 922D      		mov r25,r2
 491               	/* epilogue start */
 492 020e DF91      		pop r29
 493 0210 CF91      		pop r28
 494               	.LVL39:
 495 0212 1F91      		pop r17
 496 0214 0F91      		pop r16
 497               	.LVL40:
 498 0216 FF90      		pop r15
 499 0218 EF90      		pop r14
 500               	.LVL41:
 501 021a DF90      		pop r13
 502 021c CF90      		pop r12
 503 021e BF90      		pop r11
 504 0220 AF90      		pop r10
 505               	.LVL42:
 506 0222 9F90      		pop r9
 507               	.LVL43:
 508 0224 8F90      		pop r8
 509               	.LVL44:
 510 0226 7F90      		pop r7
 511 0228 6F90      		pop r6
 512               	.LVL45:
 513 022a 5F90      		pop r5
 514 022c 4F90      		pop r4
 515               	.LVL46:
 516 022e 3F90      		pop r3
 517               	.LVL47:
 518 0230 2F90      		pop r2
 519               	.LVL48:
 520 0232 0895      		ret
 521               	.LFE18:
 523               	.global	_ZN8SPIFlash9readBytesEjPhj
 525               	_ZN8SPIFlash9readBytesEjPhj:
 526               	.LFB14:
 527               	.LM61:
 528               	.LVL49:
 529 0234 CF92      		push r12
 530 0236 DF92      		push r13
 531 0238 EF92      		push r14
 532 023a FF92      		push r15
 533 023c 0F93      		push r16
 534 023e 1F93      		push r17
 535 0240 CF93      		push r28
 536 0242 DF93      		push r29
 537               	/* prologue: function */
 538               	/* frame size = 0 */
 539 0244 FC01      		movw r30,r24
 540 0246 8B01      		movw r16,r22
 541 0248 6A01      		movw r12,r20
 542 024a 7901      		movw r14,r18
 543               	.LBB61:
 544               	.LM62:
 545 024c 812F      		mov r24,r17
 546               	.LVL50:
 547 024e 8295      		swap r24
 548 0250 8695      		lsr r24
 549 0252 8695      		lsr r24
 550 0254 8370      		andi r24,lo8(3)
 551 0256 8083      		st Z,r24
 552               	.LM63:
 553 0258 000F      		lsl r16
 554 025a 111F      		rol r17
 555 025c 000F      		lsl r16
 556 025e 111F      		rol r17
 557               	.LVL51:
 558               	.LM64:
 559 0260 CF01      		movw r24,r30
 560 0262 6BE0      		ldi r22,lo8(11)
 561 0264 40E0      		ldi r20,lo8(0)
 562               	.LVL52:
 563 0266 0E94 0000 		call _ZN8SPIFlash7commandEhb
 564               	.LVL53:
 565               	.LM65:
 566 026a 812F      		mov r24,r17
 567               	.LVL54:
 568 026c 0E94 0000 		call _Z11spiTransmith
 569               	.LM66:
 570 0270 802F      		mov r24,r16
 571 0272 0E94 0000 		call _Z11spiTransmith
 572               	.LM67:
 573 0276 80E0      		ldi r24,lo8(0)
 574 0278 0E94 0000 		call _Z11spiTransmith
 575               	.LM68:
 576 027c 80E0      		ldi r24,lo8(0)
 577 027e 0E94 0000 		call _Z11spiTransmith
 578 0282 C0E0      		ldi r28,lo8(0)
 579 0284 D0E0      		ldi r29,hi8(0)
 580               	.LVL55:
 581 0286 00C0      		rjmp .L30
 582               	.L31:
 583               	.LBB62:
 584               	.LM69:
 585 0288 80E0      		ldi r24,lo8(0)
 586 028a 0E94 0000 		call _Z11spiTransmith
 587 028e F601      		movw r30,r12
 588 0290 EC0F      		add r30,r28
 589 0292 FD1F      		adc r31,r29
 590 0294 8083      		st Z,r24
 591               	.LM70:
 592 0296 2196      		adiw r28,1
 593               	.L30:
 594 0298 CE15      		cp r28,r14
 595 029a DF05      		cpc r29,r15
 596 029c 00F0      		brlo .L31
 597               	.LBE62:
 598               	.LBB63:
 599               	.LBB64:
 600               	.LM71:
 601 029e 8091 6200 		lds r24,98
 602 02a2 8F60      		ori r24,lo8(15)
 603 02a4 8093 6200 		sts 98,r24
 604               	.LBE64:
 605               	.LBE63:
 606               	.LBE61:
 607               	.LM72:
 608 02a8 C701      		movw r24,r14
 609               	.LVL56:
 610               	/* epilogue start */
 611 02aa DF91      		pop r29
 612 02ac CF91      		pop r28
 613               	.LVL57:
 614 02ae 1F91      		pop r17
 615               	.LVL58:
 616 02b0 0F91      		pop r16
 617               	.LVL59:
 618 02b2 FF90      		pop r15
 619 02b4 EF90      		pop r14
 620 02b6 DF90      		pop r13
 621 02b8 CF90      		pop r12
 622               	.LVL60:
 623 02ba 0895      		ret
 624               	.LFE14:
 626               	.global	_ZN8SPIFlash12readDeviceIdEv
 628               	_ZN8SPIFlash12readDeviceIdEv:
 629               	.LFB13:
 630               	.LM73:
 631               	.LVL61:
 632 02bc CF92      		push r12
 633 02be DF92      		push r13
 634 02c0 EF92      		push r14
 635 02c2 FF92      		push r15
 636 02c4 0F93      		push r16
 637 02c6 1F93      		push r17
 638               	/* prologue: function */
 639               	/* frame size = 0 */
 640 02c8 FC01      		movw r30,r24
 641               	.LBB65:
 642               	.LBB66:
 643               	.LBB67:
 644               	.LM74:
 645 02ca 52E6      		ldi r21,lo8(98)
 646 02cc C52E      		mov r12,r21
 647 02ce D12C      		mov r13,__zero_reg__
 648 02d0 D601      		movw r26,r12
 649 02d2 2C91      		ld r18,X
 650 02d4 81E0      		ldi r24,lo8(1)
 651 02d6 90E0      		ldi r25,hi8(1)
 652               	.LVL62:
 653 02d8 0080      		ld r0,Z
 654 02da 00C0      		rjmp 2f
 655 02dc 880F      	1:	lsl r24
 656 02de 991F      		rol r25
 657 02e0 0A94      	2:	dec r0
 658 02e2 02F4      		brpl 1b
 659 02e4 8F70      		andi r24,lo8(15)
 660 02e6 8095      		com r24
 661 02e8 8223      		and r24,r18
 662 02ea 8C93      		st X,r24
 663               	.LBE67:
 664               	.LBE66:
 665               	.LM75:
 666 02ec 8FE9      		ldi r24,lo8(-97)
 667 02ee 0E94 0000 		call _Z11spiTransmith
 668               	.LVL63:
 669               	.LM76:
 670 02f2 80E0      		ldi r24,lo8(0)
 671 02f4 0E94 0000 		call _Z11spiTransmith
 672 02f8 E82E      		mov r14,r24
 673               	.LVL64:
 674 02fa FF24      		clr r15
 675               	.LVL65:
 676 02fc 00E0      		ldi r16,lo8(0)
 677 02fe 10E0      		ldi r17,hi8(0)
 678               	.LVL66:
 679               	.LM77:
 680 0300 102F      		mov r17,r16
 681 0302 0F2D      		mov r16,r15
 682 0304 FE2C      		mov r15,r14
 683 0306 EE24      		clr r14
 684               	.LVL67:
 685               	.LM78:
 686 0308 80E0      		ldi r24,lo8(0)
 687 030a 0E94 0000 		call _Z11spiTransmith
 688 030e 90E0      		ldi r25,lo8(0)
 689 0310 A0E0      		ldi r26,lo8(0)
 690 0312 B0E0      		ldi r27,hi8(0)
 691 0314 E82A      		or r14,r24
 692 0316 F92A      		or r15,r25
 693 0318 0A2B      		or r16,r26
 694 031a 1B2B      		or r17,r27
 695               	.LM79:
 696 031c 102F      		mov r17,r16
 697 031e 0F2D      		mov r16,r15
 698 0320 FE2C      		mov r15,r14
 699 0322 EE24      		clr r14
 700               	.LVL68:
 701               	.LM80:
 702 0324 80E0      		ldi r24,lo8(0)
 703 0326 0E94 0000 		call _Z11spiTransmith
 704               	.LBB68:
 705               	.LBB69:
 706               	.LM81:
 707 032a F601      		movw r30,r12
 708 032c 9081      		ld r25,Z
 709 032e 9F60      		ori r25,lo8(15)
 710 0330 9083      		st Z,r25
 711 0332 282F      		mov r18,r24
 712 0334 30E0      		ldi r19,lo8(0)
 713 0336 40E0      		ldi r20,lo8(0)
 714 0338 50E0      		ldi r21,hi8(0)
 715 033a 2E29      		or r18,r14
 716 033c 3F29      		or r19,r15
 717 033e 402B      		or r20,r16
 718 0340 512B      		or r21,r17
 719               	.LBE69:
 720               	.LBE68:
 721               	.LBE65:
 722               	.LM82:
 723 0342 B901      		movw r22,r18
 724 0344 CA01      		movw r24,r20
 725               	/* epilogue start */
 726 0346 1F91      		pop r17
 727 0348 0F91      		pop r16
 728               	.LVL69:
 729 034a FF90      		pop r15
 730               	.LVL70:
 731 034c EF90      		pop r14
 732               	.LVL71:
 733 034e DF90      		pop r13
 734 0350 CF90      		pop r12
 735 0352 0895      		ret
 736               	.LFE13:
 738               	.global	_ZN8SPIFlash10initializeEv
 740               	_ZN8SPIFlash10initializeEv:
 741               	.LFB12:
 742               	.LM83:
 743               	.LVL72:
 744 0354 1F93      		push r17
 745 0356 CF93      		push r28
 746 0358 DF93      		push r29
 747               	/* prologue: function */
 748               	/* frame size = 0 */
 749 035a EC01      		movw r28,r24
 750               	.LBB70:
 751               	.LBB71:
 752               	.LBB72:
 753               	.LM84:
 754 035c 8091 6200 		lds r24,98
 755               	.LVL73:
 756 0360 8F60      		ori r24,lo8(15)
 757 0362 8093 6200 		sts 98,r24
 758               	.LBE72:
 759               	.LBE71:
 760               	.LM85:
 761 0366 CE01      		movw r24,r28
 762 0368 0E94 0000 		call _ZN8SPIFlash6wakeupEv
 763 036c 10E0      		ldi r17,lo8(0)
 764               	.LVL74:
 765               	.L38:
 766               	.LBB73:
 767               	.LM86:
 768 036e CE01      		movw r24,r28
 769 0370 0E94 0000 		call _ZN8SPIFlash12readDeviceIdEv
 770 0374 6751      		subi r22,lo8(81943)
 771 0376 7044      		sbci r23,hi8(81943)
 772 0378 8140      		sbci r24,hlo8(81943)
 773 037a 9040      		sbci r25,hhi8(81943)
 774 037c 01F0      		breq .L36
 775 037e 80E0      		ldi r24,lo8(0)
 776 0380 00C0      		rjmp .L37
 777               	.L36:
 778               	.LM87:
 779 0382 CE01      		movw r24,r28
 780 0384 61E0      		ldi r22,lo8(1)
 781 0386 41E0      		ldi r20,lo8(1)
 782 0388 0E94 0000 		call _ZN8SPIFlash7commandEhb
 783               	.LM88:
 784 038c 80E0      		ldi r24,lo8(0)
 785 038e 0E94 0000 		call _Z11spiTransmith
 786               	.LBB74:
 787               	.LBB75:
 788               	.LM89:
 789 0392 8091 6200 		lds r24,98
 790 0396 8F60      		ori r24,lo8(15)
 791 0398 8093 6200 		sts 98,r24
 792               	.LBE75:
 793               	.LBE74:
 794               	.LM90:
 795 039c 1F5F      		subi r17,lo8(-(1))
 796 039e 1430      		cpi r17,lo8(4)
 797 03a0 01F4      		brne .L38
 798 03a2 81E0      		ldi r24,lo8(1)
 799               	.L37:
 800               	/* epilogue start */
 801               	.LBE73:
 802               	.LBE70:
 803               	.LM91:
 804 03a4 DF91      		pop r29
 805 03a6 CF91      		pop r28
 806               	.LVL75:
 807 03a8 1F91      		pop r17
 808               	.LVL76:
 809 03aa 0895      		ret
 810               	.LFE12:
 812               	.global	spiFlash
 813               	.global	spiFlash
 814               		.section .bss
 817               	spiFlash:
 818 0000 00        		.skip 1,0
 939               	.Letext0:
DEFINED SYMBOLS
                            *ABS*:00000000 flash.cc
C:\cygwin64\tmp/ccBg4t0j.s:2      *ABS*:0000003f __SREG__
C:\cygwin64\tmp/ccBg4t0j.s:3      *ABS*:0000003e __SP_H__
C:\cygwin64\tmp/ccBg4t0j.s:4      *ABS*:0000003d __SP_L__
C:\cygwin64\tmp/ccBg4t0j.s:5      *ABS*:00000034 __CCP__
C:\cygwin64\tmp/ccBg4t0j.s:6      *ABS*:00000000 __tmp_reg__
C:\cygwin64\tmp/ccBg4t0j.s:7      *ABS*:00000001 __zero_reg__
C:\cygwin64\tmp/ccBg4t0j.s:18     .text:00000000 _ZN8SPIFlash6selectEv
C:\cygwin64\tmp/ccBg4t0j.s:49     .text:00000022 _ZN8SPIFlash8unselectEv
C:\cygwin64\tmp/ccBg4t0j.s:69     .text:0000002e _ZN8SPIFlash10readStatusEv
C:\cygwin64\tmp/ccBg4t0j.s:127    .text:0000006e _ZN8SPIFlash4busyEv
C:\cygwin64\tmp/ccBg4t0j.s:144    .text:00000076 _ZN8SPIFlash7commandEhb
C:\cygwin64\tmp/ccBg4t0j.s:213    .text:000000cc _ZN8SPIFlash6wakeupEv
C:\cygwin64\tmp/ccBg4t0j.s:241    .text:000000e0 _ZN8SPIFlash5sleepEv
C:\cygwin64\tmp/ccBg4t0j.s:269    .text:000000f4 _ZN8SPIFlash9chipEraseEh
C:\cygwin64\tmp/ccBg4t0j.s:301    .text:0000010c _ZN8SPIFlash12blockErase4KEj
C:\cygwin64\tmp/ccBg4t0j.s:362    .text:00000154 _ZN8SPIFlash10writeBytesEjPhj
C:\cygwin64\tmp/ccBg4t0j.s:525    .text:00000234 _ZN8SPIFlash9readBytesEjPhj
C:\cygwin64\tmp/ccBg4t0j.s:628    .text:000002bc _ZN8SPIFlash12readDeviceIdEv
C:\cygwin64\tmp/ccBg4t0j.s:740    .text:00000354 _ZN8SPIFlash10initializeEv
C:\cygwin64\tmp/ccBg4t0j.s:817    .bss:00000000 spiFlash

UNDEFINED SYMBOLS
_Z11spiTransmith
__do_clear_bss
