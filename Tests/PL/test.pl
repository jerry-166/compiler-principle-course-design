program demo;
  var i, n, sum : integer;
  procedure addto;
    var k : integer;
  begin
    while i <= n do
    begin
      sum := sum + i;
      i := i + 1
    end
  end;
begin
  call read(n);
  i := 1;
  sum := 0;
  call addto;
  call write(sum)
end.
