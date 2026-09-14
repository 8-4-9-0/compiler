program p07;
var a, b, c : array[100] of integer;
    p, q, r, s : integer;
begin
    a[24] := 0;
    b[82] := 6;
    c[43] := 5;
    p := 1; 
    q := 2;
    r := 3;
    s := 4;
    writeln((p+r)*(s*c[s*r*q*p*q-(r+q)])+q); /* = 82 */
    writeln(s*r*q*p*q-(r+q)); /* = 43 */
    /* ↓ a[24] := 1337 */
    a[((b[(p+r)*(s*c[s*r*q*p*q-(r+q)])+q]*c[s*r*q*p*q-(r+q)]+q) div s+b[(p+r)*(s*c[s*r*q*p*q-(r+q)])+q])*q-(r+p)] := 1337;
    writeln(a[((b[(p+r)*(s*c[s*r*q*p*q-(r+q)])+q]*c[s*r*q*p*q-(r+q)]+q) div s+b[(p+r)*(s*c[s*r*q*p*q-(r+q)])+q])*q-(r+p)])
end.