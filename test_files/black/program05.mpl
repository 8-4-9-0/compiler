program p05;
var n : integer;
begin
    writeln('input the number (0-255)');
    readln(n);
    if n >= 16 then begin
        if n >= 64 then begin
            if n >= 128 then writeln('End: route 1')
            else writeln('End: route 2')
        end
        else begin
            if n >= 32 then writeln('End: route 3')
            else writeln('End: route 4')
        end
    end
    else begin
        if n >= 4 then begin
            if n >= 8 then writeln('End: route 5')
            else writeln('End: route 6')
        end
        else begin
            if n >= 2 then writeln('End: route 7')
            else writeln('End: route 8')
        end
    end
end.