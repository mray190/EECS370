		lw		0		1		one			Load 1 into reg 1
		lw		0		2		three		Load 3 into reg 2
		sw		0		2		one			Load reg 1 into memory address 'one' AKA label one becomes 3
		lw		0		3		one			Load 3 (from label 'one') into reg 3 
		noop
		add 	1		3		4			Load reg 1 & reg 2 into reg 4
		halt
one		.fill	1
three	.fill	3
