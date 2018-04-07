import sys

def read_vector(vectorName, node_to_edge):
    with open(vectorName, 'r') as f:
        for l in f:
            split_line = l.split()
            tid        = split_line[0]            
            for x in split_line[1:]: # two spaces.
                [index, value] = x.split(':')
                if index not in node_to_edge: node_to_edge[index] = set()
                node_to_edge[index].add(tid)

def train_sets():
    fs   = ['lyrl2004_vectors_test_pt0.dat', 'lyrl2004_vectors_test_pt1.dat', 'lyrl2004_vectors_test_pt2.dat', 'lyrl2004_vectors_test_pt3.dat']
    node_to_edge = dict()
    for fname in fs:
        print("Starting {}".format(fname))
        read_vector(fname, node_to_edge)
    return node_to_edge

def main(out_node_degree, out_edge_degree):
    node_to_edge = train_sets()
    print("Printing Node Degree to {}".format(out_node_degree))
    #with open(out_node_degree, 'w') as f:
    #    f.write( '\n'.join( [str(len(x)) for x in node_to_edge.values()] ) )
    print("Creating Frozen version")
    frozen_node_to_edge = dict([ (k,frozenset(v)) for (k,v) in node_to_edge.iteritems() ])

    print("Printing Edge Degree to {}".format(out_node_degree))
    with open(out_edge_degree, 'w') as out_f:
        fs   = ['lyrl2004_vectors_test_pt0.dat', 'lyrl2004_vectors_test_pt1.dat', 'lyrl2004_vectors_test_pt2.dat', 'lyrl2004_vectors_test_pt3.dat']
        for fname in fs:
            with open(fname, 'r') as f:
                for l in f:
                    split_line = l.split()
                    tid        = split_line[0]            
                    index_sets = [frozen_node_to_edge[x.split(':')[0]] for x in split_line[1:]]
                    adjacents  = reduce(lambda x,y: x.union(y), index_sets)
                    out_f.write(str(len(adjacents) - 1) + '\n')

    

if len(sys.argv) == 3:
    main(sys.argv[1], sys.argv[2])
else:
    print("{} <output node info> <output edge info>".format(sys.argv[0]))
