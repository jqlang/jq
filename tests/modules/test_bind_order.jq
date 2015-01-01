import "test_bind_order0" as t;
import "test_bind_order1" as t;
import "test_bind_order2" as t;
def check: if [t::sym0,t::sym1,t::sym2] == [0,1,2] then true else false end;
