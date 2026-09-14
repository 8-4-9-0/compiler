program p01;
var a, b, c, d : array[100] of integer;
begin
    a[50] := 5;
    b[22] := 50;
    c[73] := 22;
    d[2] := 73;
    writeln(a[b[c[d[2]]]])
end.