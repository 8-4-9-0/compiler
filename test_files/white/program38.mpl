program p01;
var a : integer; b : char; c : boolean; d : array[10] of integer; e : array[10] of char; f : array[10] of boolean; g, h: integer;
procedure proc01(aa : integer; bb : char);
var cc : boolean;
begin
    if aa = 0 then return;
    cc := false;
    writeln('test', aa : 2, bb, cc)