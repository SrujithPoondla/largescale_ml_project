import sys, struct
## Binary verisons
def binary_edge_parser(fname, strict=False):
    base_offset = struct.calcsize('i')
    struct_size = struct.calcsize('iid')
        
    with open(fname, 'rb') as f:
        buf       = f.read()        
        (nItems,) = struct.unpack_from('i', buf)
        anticipated_size = nItems*struct_size + base_offset 
        actual_size      = len(buf)
        print "Found {} binary structures in {} bytes (expected {})".format(nItems, actual_size, anticipated_size)
        if strict:
            assert( actual_size == anticipated_size )
        else:
            if actual_size != anticipated_size: print "[Warning] Error in nItems read. Permissive."
            nItems = (actual_size - base_offset)/struct_size
            assert(nItems >= 0)

        for i in xrange(nItems):
            (x,y,rating,) = struct.unpack_from('iid', buf, offset=base_offset + i*struct_size)
            yield (x,y,rating)

def text_edge_parser(fname, delim):
    with open(fname, 'r') as f:
        for line in f:
            splits = line.split(delim)
            x = splits[0]
            y = splits[1]        
            rating = splits[2]
            yield (x,y,rating)

def choose_generator(fname, delim):
    if delim is not None:
        return lambda : text_edge_parser(fname,delim)
    else:
        return lambda : binary_edge_parser(fname)


def compute_model_degree(fname, delim):
    row_degree_dict = dict()
    col_degree_dict = dict()

    for (x,y,rating) in (choose_generator(fname,delim))():
        if x not in row_degree_dict: row_degree_dict[x] = 0
        if y not in col_degree_dict: col_degree_dict[y] = 0            

        row_degree_dict[x] += 1
        col_degree_dict[y] += 1

    return (row_degree_dict, col_degree_dict)

def compute_matrix_stats(fname, node_outname, edge_outname, delim=' '):
    (row_degree_dict,col_degree_dict) = compute_model_degree(fname, delim=delim)
    print "Node Degrees computed. Writing"
    with open(node_outname, 'w') as fout:
        for v in col_degree_dict.values():
            fout.write(str(v) + '\n')
        for v in row_degree_dict.values():
            fout.write(str(v) + '\n')
            
    print "Edge Degrees computed. Writing"    
    # Lines.
    with open(edge_outname, 'w') as fout:
        for (x,y,rating) in  (choose_generator(fname,delim))():
            edge_degree = row_degree_dict[x] + col_degree_dict[y] - 1
            fout.write(str(edge_degree) + '\n')


if __name__ == "__main__":
    compute_matrix_stats(sys.argv[1], sys.argv[2], sys.argv[3])
