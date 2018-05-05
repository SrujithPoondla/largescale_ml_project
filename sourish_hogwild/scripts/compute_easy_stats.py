#import compute_graph_stats as gs
import compute_matrix_stats as ms

matrix_datasets = [#("Netflix", "/raid/chrisre/train_set.binary.dat"),
                   ("Jumbo", "/raid/chrisre/HUGE.train.nr=10000000.alpha=1.rrank=10.oversample=10.noise_var=0.001.factor_type=0.data"),
                   #("KDD", "/raid/chrisre/kdd.cup.trainIDx1.binary.txt")
                   ]
def run_matrix_stats():
    for (name,source) in matrix_datasets:
        print("Starting {}".format(name))
        node_out =  "{}.stats.nodes.out".format(name)
        edge_out =  "{}.stats.edges.out".format(name)
        ms.compute_matrix_stats(source,node_out, edge_out, delim=None)

graph_datasets = [("abdomen", "/raid/chrisre/abdomen.binary.day"),
                  ("dblife", "/raid/chrisre/dblife.binary.examples.dat")]

def run_graph_stats():
    for (name, source) in graph_datasets:
        print("Starting {}".format(name))
        node_out =  "{}.stats.nodes.out".format(name)
        edge_out =  "{}.stats.edges.out".format(name)
        ms.compute_matrix_stats(source,node_out, edge_out, delim=None)
        

if __name__ == "__main__":
    run_matrix_stats()
    #run_graph_stats()
