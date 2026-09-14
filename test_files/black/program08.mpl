program p08;
procedure sum(n, s : integer);
begin
    s := 0;
    while n > 0 do begin
        s := s + n;
        n := n - 1
    end
end;
procedure average(ave, n : integer);
var s, n2 : integer;
begin
    n2 := n;
    call sum(n2, s);
    ave := s div n
end;
var a : integer;
begin
    call average(a, 10);
    writeln(a)
end.