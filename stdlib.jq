def map(f) = [.[] | f];

def first = .[0];
# def last = .[count-1];
def next = .[count];

# ([])[] would be a decent definition of "empty"
# except ([]) is defined as syntactic sugar for empty
def empty = {}[];

def sort = [group(.) | .[]];