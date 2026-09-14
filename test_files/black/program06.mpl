program p06;
var a, b, c, d, e : integer;
begin
    a := 2;
    b := -4;
    c := 8;
    d := -16;
    e := 32;
    writeln(((e * (d + c)) div a) - (e * b div d) * (c + a));　/* = -208 */
    a := ((e * (d + c)) div a) - (e * b div d) * (c + a);
    writeln(a); 
end.