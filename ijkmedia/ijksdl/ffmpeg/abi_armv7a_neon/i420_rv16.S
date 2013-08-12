 @*****************************************************************************
 @ i420_rv16.S : ARM NEONv1 I420 to RV16 chroma conversion
 @*****************************************************************************
 @ Copyright (C) 2011 Sébastien Toque
 @                    Rémi Denis-Courmont
 @
 @ This program is free software; you can redistribute it and/or modify it
 @ under the terms of the GNU Lesser General Public License as published by
 @ the Free Software Foundation; either version 2.1 of the License, or
 @ (at your option) any later version.
 @
 @ This program is distributed in the hope that it will be useful,
 @ but WITHOUT ANY WARRANTY; without even the implied warranty of
 @ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 @ GNU Lesser General Public License for more details.
 @
 @ You should have received a copy of the GNU Lesser General Public License
 @ along with this program; if not, write to the Free Software Foundation,
 @ Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 @****************************************************************************/

	.syntax unified
	.fpu neon
	.text

/* ARM */
#define O1	r0
#define O2	r1
#define WIDTH	r2
#define HEIGHT	r3
#define Y1	r4
#define Y2	r5
#define U	r6
#define V	r7
#define YPITCH	r8
#define OPAD	r10
#define YPAD	r11
#define COUNT	ip
#define OPITCH	lr

/* NEON */
#define coefY	D0
#define coefRV	D1
#define coefGU	D2
#define coefGV	D3
#define coefBU	D4
#define Rc	Q3
#define Gc	Q4
#define Bc	Q5

#define u	D24
#define v	D25
#define y1	D18
#define y2	D19

#define chro_r	Q6
#define chro_g	Q7
#define chro_b	Q8
#define lumi1	Q15
#define lumi2	Q10
#define red16_1		Q9
#define green16_1	Q10
#define blue16_1	Q11
#define red16_2		Q12
#define green16_2	Q13
#define blue16_2	Q14

#define red1	D25
#define green1	D26
#define blue1	D27
#define red2	D29
#define green2	D30
#define blue2	D31

#define out1l	D24
#define out1h	D25
#define out2l	D28
#define out2h	D29

coefficients:
    .short  -15872
    .short    4992
    .short  -18432

	.align 2
	.global i420_rv16_neon
	.type	i420_rv16_neon, %function
i420_rv16_neon:
	push		{r4-r8,r10-r11,lr}
	vpush		{q4-q7}

	/* load arguments */
	ldmia		r0,	{O1, OPITCH}
	ldmia		r1,	{Y1, U, V, YPITCH}

	/* round the width to be a multiple of 16 */
	ands		OPAD, WIDTH, #15
	sub			WIDTH, WIDTH, OPAD
	addne		WIDTH, WIDTH, #16

	/* init constants (scale value by 64) */
	vmov.u8		coefY, #74
	vmov.u8		coefRV, #115
	vmov.u8		coefGU, #14
	vmov.u8		coefGV, #34
	vmov.u8		coefBU, #135
	adr			OPAD, coefficients
	vld1.s16	{d6[], d7[]}, [OPAD]!
	vld1.s16	{d8[], d9[]}, [OPAD]!
	vld1.s16	{d10[], d11[]}, [OPAD]!

	/* init padding */
	cmp			HEIGHT,	#0
	sub			OPAD,	OPITCH,	WIDTH, lsl #1
	sub			YPAD,	YPITCH,	WIDTH

loop_row:
	movsgt	COUNT,	WIDTH
	add		O2,	O1,	OPITCH
	add		Y2,	Y1,	YPITCH
	/* exit if all rows have been processed */
	vpople	{q4-q7}
	pople	{r4-r8,r10-r11,pc}

loop_col:

	/* Common U & V */

	vld1.u8	{u}, [U,:64]!
	vld1.u8	{v}, [V,:64]!

	/* Y Top Row */
	vld2.u8	{y1,y2}, [Y1,:128]!

	vmull.u8	Q14, v, coefRV
	vmull.u8	Q11, u, coefGU
	vmull.u8	Q13, u, coefBU
	vmlal.u8	Q11, v, coefGV

	vmull.u8	lumi2, y2, coefY
	vmull.u8	lumi1, y1, coefY
	vadd.s16	chro_r, Rc, Q14
	vadd.s16	chro_b, Bc, Q13
	vsub.s16	chro_g, Gc, Q11

	pld	[U]
	pld	[V]

	/* chrominance + luminance */
	vqadd.s16	red16_2, lumi2, chro_r
	vqadd.s16	green16_2, lumi2, chro_g
	vqadd.s16	blue16_2, lumi2, chro_b
	vqadd.s16	red16_1, lumi1, chro_r
	vqadd.s16	green16_1, lumi1, chro_g
	vqadd.s16	blue16_1, lumi1, chro_b

	/* clamp (divide by 64) */
	vqrshrun.s16	green2, green16_2, #6
	vqrshrun.s16	blue2, blue16_2, #6
	vqrshrun.s16	red2, red16_2, #6
	vqrshrun.s16	green1, green16_1, #6
	vqrshrun.s16	red1, red16_1, #6
	vqrshrun.s16	blue1, blue16_1, #6

	pld	[Y1]

	/* pack into RGB565 */
	vshl.u8	out2l, green2, #3 // low 2a
	vsri.u8	out2h, green2, #5 // high 2
	vshl.u8	out1l, green1, #3 // low 1a
	vsri.u8	out1h, green1, #5 // high 1
	vsri.u8	out2l, blue2, #3 // low 2b
	vsri.u8	out1l, blue1, #3 // low 1b

	/* Y Bottom Row */
	vld2.u8	{y1,y2}, [Y2,:128]!

	/* Top Row output */
	vzip.u8	out1h, out2h
	vmull.u8	lumi2, y2, coefY
	vzip.u8	out1l, out2l
	vmull.u8	lumi1, y1, coefY
	vst2.u8	{out1l, out1h}, [O1,:128]!
	vst2.u8	{out2l, out2h}, [O1,:128]!

	/* chrominance + luminance */
	vqadd.s16	green16_2, lumi2, chro_g
	vqadd.s16	red16_2, lumi2, chro_r
	vqadd.s16	blue16_2, lumi2, chro_b
	vqadd.s16	red16_1, lumi1, chro_r
	vqadd.s16	green16_1, lumi1, chro_g
	vqadd.s16	blue16_1, lumi1, chro_b

	/* clamp (divide by 64) */
	vqrshrun.s16	green2, green16_2, #6
	vqrshrun.s16	blue2, blue16_2, #6
	vqrshrun.s16	red2, red16_2, #6
	vqrshrun.s16	green1, green16_1, #6
	vqrshrun.s16	red1, red16_1, #6
	vqrshrun.s16	blue1, blue16_1, #6

	pld	[Y1]

	/* pack into RGB565 */
	vshl.u8	out2l, green2, #3 // low 2a
	vsri.u8	out2h, green2, #5 // high 2
	vshl.u8	out1l, green1, #3 // low 1a
	vsri.u8	out1h, green1, #5 // high 1
	vsri.u8	out2l, blue2, #3 // low 2b
	vsri.u8	out1l, blue1, #3 // low 1b

	vzip.u8	out1h, out2h
	vzip.u8	out1l, out2l
	vst2.u8	{out1l, out1h}, [O2,:128]!
	vst2.u8	{out2l, out2h}, [O2,:128]!

	/* next columns (x16) */
	subs	COUNT,	COUNT,	#16
	bgt		loop_col

	/* next rows (x2) */
	subs	HEIGHT,	#2
	add		O1,	O2,	OPAD
	add		Y1,	Y2,	YPAD
	add		U,	U,	YPAD,	lsr #1
	add		V,	V,	YPAD,	lsr #1
	b		loop_row
