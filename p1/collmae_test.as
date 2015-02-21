        lw      0       2       mcand           load reg2 with 5
        lw      0       3       mplier          load reg3 with 3
        lw      0       4       mask            load reg4 with bitmask number
        lw      0       5       end             load reg5 with 65536
start   beq     5       4       done            if reg6 is equal to 65536 (reg5) then go to done, otherwise proceed with loop
        nand    2       4       6               nand mcand with mask to get masked
        nand    6       6       6               nand masked with masked
        beq     6       0       bit             if masked is 0, go to bit
        add     1       3       1               if masked is 1, add mplier to result
bit     add     3       3       3               add mplier to mplier
        add     4       4       4               add mask to mask to increment shift
        beq     0       0       start           return to the top of the loop always
done    halt
mcand   .fill   3
mplier  .fill   4
mask    .fill   1
end     .fill   65536
