program p03;
var i, j, k: integer;
begin
    i := 0;
    j := 0;
    k := 0;
    while i < 5 do begin
        writeln('i = ', i);
        i := i + 1;
        while j < 4 do begin
            writeln('  j = ', j);
            j := j + 1;
            while k < 3 do begin
                writeln('    k = ', k);
                k := k + 1
            end;
            k := 0
        end;
        j := 0
    end
end.