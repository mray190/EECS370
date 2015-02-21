		lw		0		1		n
		lw		0		2		r
		lw		0		7		NOne
		lw		0		6		start
		jalr	6		5
		halt
nCr		sw		4		5		stack
		lw		0		5		one
		add		4		5		4
		beq		0		2		base
		beq		5		2		base2
		beq		1		2		base
		add		1 		7		1
		sw		4		2		stack
		add		4		5		4
		sw		4		1		stack
		add		4		5		4
		lw		0		6		start
		jalr	6		5
		add		4		7		4
		lw		4		1		stack
		add		4		7		4
		lw		4		2		stack
		add		2		7		2
		lw		0		6		start
		jalr	6		5
		add		4		7		4
		lw		4		5		stack
		jalr	5		6
base	add		3		5		3
		add		4		7		4
		lw		4		5		stack
		jalr	5		6
base2	add		3		1		3
		add		4		7		4
		lw		4		5		stack
		jalr	5		6
n 		.fill	4
r		.fill	2
start	.fill	nCr
one		.fill	1
NOne 	.fill	-1
stack	.fill	0
