    .equ    dest,   1
    .equ    ch,	    2

    .text
    ldzwq   0,	%dest

read:
	getc    %ch
#   if %ch < '0' then we are done
    subq    '0',    %ch,    %0
    jb	    done
#   if %ch > '9' then we are done
    subq    '9',    %ch,    %0
    ja	    done

    subq    '0',    %ch,    %ch
    imulq   10,	    %dest,  %dest
    addq    %ch,    %dest,  %dest
    jmp	    read

done:
    halt    %dest
