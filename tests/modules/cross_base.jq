def _cross_table: [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15];
def cross_lookup($i): _cross_table | .[$i % 16];
