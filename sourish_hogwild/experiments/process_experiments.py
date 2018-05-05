import re, collections, os.path
line_re    = re.compile('\[Epoch Stats\] epoch: (?P<epoch_number>[0-9]+) epoch_time: (?P<epoch_time>[0-9.]+) total_time: (?P<total_time>[0-9.]+)s perm_time (?P<perm_time>[0-9.e\-]+) train_rmse: (?P<train_rmse>[0-9.e\-]+) test_rmse: (?P<test_rmse>[0-9.e\-]+)')
header_re  = re.compile(r"# \[freeforall_generic\]  nepochs=(?P<total_epochs>[0-9]+) nSplits=(?P<splits>[0-9]+) nShufflers=([0-9]+)")


epochEntry = collections.namedtuple('epochEntry', 'epoch epoch_time total_time perm_time train_error test_error')
##########################################3
# This returns a pair of the number of splits and list of epochEntry (in order by epoch number)
# parsed from the provided file name
# It does basic sanity checking and type conversion
def process_experiment_file(fname):
    last_epoch = None
    nEpochs    = None
    nSplits    = None
    ret_value  = []
    with open(fname, 'r') as f:
        for line in f:
            #print line
            hm = header_re.match(line)
            if hm is not None:
                nEpochs = int(hm.group('total_epochs'))
                nSplits = int(hm.group('splits'))

            m = line_re.match(line)
            if m is not None:
                this_epoch = int(m.group('epoch_number'))

                # Sanity Check on epoch parsing to make
                # sure we didn't miss a line
                if last_epoch is None:
                    assert( this_epoch == 1 )
                    last_epoch = this_epoch
                else:
                    assert( (last_epoch + 1) == this_epoch)
                    last_epoch = this_epoch
                
                ret_value.append(epochEntry(epoch      = int(m.group('epoch_number')),
                                            epoch_time = float(m.group('epoch_time')),
                                            total_time = float(m.group('total_time')),
                                            perm_time  = float(m.group('perm_time')),
                                            train_error= float(m.group('train_rmse')),
                                            test_error = float(m.group('test_rmse'))))
    if (last_epoch is None) or (last_epoch != nEpochs):
        print("Last Epoch={} nEpochs={} {} fname={}".format(last_epoch, nEpochs, (last_epoch != nEpochs), fname))
        assert(False)
    return (nSplits, ret_value)

# Train versus error type graphs
def time_train_rmse_values(fname):
    (nSplits, epochs) = process_experiment_file(fname)
    def extract_values(e):
        return (e.total_time, e.train_error)
    return map(extract_values, epochs)


def epoch_average(epochs):
    def extract_values(e):
        return e.epoch_time
    s   = sum(map(extract_values, epochs))
    avg = s / len(epochs)
    return float(avg)
    
def speedup_graph_helper(fnames):
    vs  = []
    for fname in fnames:
        (nSplits, epochs) = process_experiment_file(fname)
        vs.append( (nSplits, epoch_average(epochs)) )
    return vs
    
def speedup_graphs(baseline, comparisons):
    (nSplits, epochs) = process_experiment_file(baseline)
    baseline_speed    = epoch_average(epochs)
    
    comp_speeds     = speedup_graph_helper(comparisons)
    return [ (comp_speeds[i][0], baseline_speed/comp_speeds[i][1]) for i in range(len(comp_speeds)) ]
    
def average_epoch_time(comparison_pairs):    
    comp_speeds       = speedup_graph_helper(map(lambda x:x[1], comparison_pairs))
    return [ (comparison_pairs[i][0], comp_speeds[i][1]) for i in range(len(comp_speeds)) ]

# each triple is (delay, baseline, comparison)
def generic_speedups(comparison_triples):
    baseline_speeds   = speedup_graph_helper(map(lambda x:x[1], comparison_triples))
    comp_speeds       = speedup_graph_helper(map(lambda x:x[2], comparison_triples))
    return [ (comparison_triples[i][0], baseline_speeds[i][1]/comp_speeds[i][1]) for i in range(len(comp_speeds)) ]
