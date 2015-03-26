		lw		0		1		n
		lw		0		2		r
		add		1		2 		3
		nand	1 		2		4
		add		4		1 		5
		sw		0		5		0
		sw		0		5		1
		noop
		noop
		add		5		1 		6
		halt
n 		.fill	12
r		.fill	10
