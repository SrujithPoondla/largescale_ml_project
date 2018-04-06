import matplotlib              
import matplotlib.pyplot as plt
matplotlib.use('pdf')

import collections
import process_experiments as pexp
import os.path

def produce_time_train_rmse_graph(title, desc, outname):
    #graph_data = tuple([x for (name, fname) in desc for x in list(pexp.time_train_rmse_values(fname)) ])
    plots = []
    plt.figure()
    for (name, fname) in desc:
        if not os.path.isfile(fname): continue
        q = pexp.time_train_rmse_values(fname)
        x = map(lambda x:x[0], q)
        y = map(lambda x:x[1], q)
        plots.append(plt.plot(x,y,label=name))
    plt.xlabel('time')
    plt.ylabel('error')
    plt.title(title)
    plt.legend()
    plt.savefig(outname)

    


def plot_speed_graphs(title, baseline, descs, outname):
    plots = []
    plt.figure()
    for (name, fnames) in descs:
        fnames = [f for f in fnames if os.path.isfile(f)]
        if not fnames: continue
        q = pexp.speedup_graphs(baseline, fnames)
        x = map(lambda x:x[0], q)
        y = map(lambda x:x[1], q)
        plots.append(plt.plot(x,y,label=name))
    plt.xlabel('# of workers')
    plt.ylabel('Speedup')
    plt.title(title)
    plt.legend()
    plt.savefig(outname)

def plot_delay_speed_graph(title, descs, outname):        
    plots = []
    plt.figure()
    for (name, fname_pairs) in descs:
        fname_pairs = [f for f in fname_pairs if os.path.isfile(f)]
        if not fname_pairs: continue
        q = pexp.average_epoch_time(fname_pairs)
        x = map(lambda x:x[0], q)
        y = map(lambda x:x[1], q)
        plots.append(plt.plot(x,y,label=name))
    plt.xlabel('Delay')
    plt.xscale('log')
    
    plt.ylabel('Average Epoch Time')
    plt.title(title)
    plt.legend()
    plt.savefig(outname)



def plot_delay_speedup(title, descs, outname):        
    plots = []
    plt.figure()
    for (name, fname_triples) in descs:
        #print fname_triples
        fname_triples = [f for f in fname_triples if os.path.isfile(f[1]) and os.path.isfile(f[2])]
        if not fname_triples: continue
        q = pexp.generic_speedups(fname_triples)
        x = map(lambda x:x[0], q)
        y = map(lambda x:x[1], q)
        plots.append(plt.plot(x,y,label=name))
    plt.xlabel('Delay')
    plt.xscale('log')
    
    plt.ylabel('Epoch Speedup')
    plt.title(title)
    plt.legend()
    plt.savefig(outname)


