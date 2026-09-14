program p04;
var i, j, k: integer;
begin
    i := 0;
    j := 0;
    k := 0;
    while true do begin
        writeln('i = ', i);
        i := i + 1;
        while true do begin
            writeln('  j = ', j);
            j := j + 1;
            while true do begin
                writeln('    k = ', k);
                k := k + 1;
                if k >= 3 then break
            end;
            k := 0;
            if j >= 4 then break
        end;
        j := 0;
        if i >= 5 then break
    end
end.