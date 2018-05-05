import sys
import compute_matrix_stats as ms

def compute_node_degree(fname, delim):
    node_degree_dict = dict()
    for (x,y, rating) in (ms.choose_generator(fname,delim))():
        if x not in node_degree_dict: node_degree_dict[x] = 0
        if y not in node_degree_dict: node_degree_dict[y] = 0            
        
        node_degree_dict[x] += 1
        node_degree_dict[y] += 1
        
    return node_degree_dict


def compute_graph_stats(fname, node_outname, edge_outname, delim='\t'):
    node_degree_dict = compute_node_degree(fname, delim=delim)
    print("Node Degrees computed. Writing")
    with open(node_outname, 'w') as fout:
        vs = node_degree_dict.values()
        for v in vs:
            fout.write(str(v) + '\n')

    print("Edge Degrees computed. Writing")    
    # Lines.
    with open(edge_outname, 'w') as fout:
        for (x,y,rating) in  (choose_generator(fname,delim))():
            edge_degree = node_degree_dict[x] + node_degree_dict[y] - 1
            fout.write(str(edge_degree) + '\n')

if __name__ == "__main__":
    compute_graph_stats(sys.argv[1], sys.argv[2], sys.argv[3])
