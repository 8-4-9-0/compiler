program p01;
var a : integer; b : char; c : boolean; d : array[10] of integer; e : array[10] of char; f : array[10] of boolean; g, h: integer;
procedure proc01(aa : integer; bb : char);
var cc : boolean;
begin
    if aa = 0 then return;
    cc := false;
    writeln('test', aa : 2, bb, cc);
    return;
end;
begin
    a := 1;
    b := 'a';
    c := true;
    c := not false;
    c := true or false and true;
    d[0] := 1;
    d[a-1] := d[0];
    e[0] := 'b';
    f[0] := false;
    while true do begin
        while a < 5 do begin
            a := -1;
            a := 1;
            a := a + 1;
            a := a - 1;
            a := a * 1;
            a := a div 1;
            a := (((a + 1) - 1) * 1) div 1;
            a := a + 1
        end;
        if a = 5 then begin
            break
        end else if a <> 5 then begin
            break
        end else if a < 5 then begin
            break
        end else if a <= 5 then begin
            break
        end else if a > 5 then begin
            break
        end else if a >= 5 then begin
            break
        end
    end;
    call