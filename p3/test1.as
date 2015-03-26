		lw		0		1		n
		lw		0		1		r
		add		1		2 		3
		nand	1 		2		4
		add		4		1 		5
		beq		2		2		2
		sw		0		5		0
		sw		0		5		1
		beq		2		1 		3
		add		5		5 		5
		sw		0		5		3
		sw		0		5		4
		halt
n 		.fill	12
r		.fill	10
