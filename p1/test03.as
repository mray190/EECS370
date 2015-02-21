		lw		0		1		one			Load 1 into reg 1
		lw		0		5		one			Load 1 into reg 5
		lw		0		2		two			Load 2 into reg 2
		beq		1		5		1			If reg 1 and reg 2 are equal, goto address 5, else goto address 4
		add 	1		2		3			Load reg 3 with reg 1 + reg 2
		add 	3		1		4			Load reg 4 with reg 3 + reg 1
		beq		4		1		1			If reg 1 and reg 4 are equal, goto address 9, else goto address 8
		add 	1		5		6			Load reg 6 with reg 1 + reg 5
		halt
one		.fill	1
two		.fill	2
