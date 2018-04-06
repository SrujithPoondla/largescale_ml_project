import plot_experiments as pe


#### Individual Experiments 
base_dir = '/home/sourish/Hogwild/output/'

abdomen_time_error = [('Hogwild 10', base_dir + 'abdomen.cut.bin.cut_generic.splits.10.step_initial.0.1.step_decay.0.9.log'),
                     ('Round Robin 10', base_dir + 'abdomen.cut.bin.cut_roundrobin_busy.splits.10.step_initial.0.1.step_decay.0.9.log'),
                     ('Single Thread', base_dir + 'abdomen.cut.bin.cut_generic.splits.1.step_initial.0.1.step_decay.0.9.log')]

dblife_time_error  = [('Hogwild 10', base_dir + 'dblife.multicut.bin.multicut_generic.splits.10.step_initial.0.1.step_decay.0.95.log'),
                      ('Single Thread', base_dir + 'dblife.multicut.bin.multicut_generic.splits.1.step_initial.0.1.step_decay.0.95.log'),
                      ('Round Robin 10',  base_dir + 'dblife.multicut.bin.multicut_roundrobin_busy.splits.10.step_initial.0.1.step_decay.0.95.log')]

svm_time_error     = [('Hogwild 10', base_dir + 'svm.bin.svm_generic.splits.10.step_initial.0.5.step_decay.0.8.log'),
                      ('Single Thread', base_dir + 'svm.bin.svm_generic.splits.1.step_initial.0.5.step_decay.0.8.log'),
                      ('Round Robin 10',  base_dir + 'svm.bin.svm_delay_roundrobin_busy.splits.10.step_initial.0.5.step_decay.0.8.log')]

# list of the above
time_error_experiments = [('Abdomen (cut)', abdomen_time_error, '/home/sourish/Hogwild/graphs/abdomen_time_error.pdf'),
                          ('DBLife (multicut)', dblife_time_error, '/home/sourish/Hogwild/graphs/dblife_time_error.pdf'),
                          ('RCV1 (svm)', svm_time_error, '/home/sourish/Hogwild/graphs/svm_time_error.pdf')]

def produce_time_error_plots():
    for (title, graph_desc, out_name) in time_error_experiments:
        pe.produce_time_train_rmse_graph(title, graph_desc, out_name)


### Speed up graphs
abdomen_speedup = [('Hogwild', base_dir + 'abdomen.cut.bin.cut_generic.splits.{}.step_initial.0.1.step_decay.0.9.log'),
                     ('Round Robin', base_dir + 'abdomen.cut.bin.cut_roundrobin_busy.splits.{}.step_initial.0.1.step_decay.0.9.log')]

dblife_speedup  = [('Hogwild', base_dir + 'dblife.multicut.bin.multicut_generic.splits.{}.step_initial.0.1.step_decay.0.95.log'),
                      ('Round Robin 10',  base_dir + 'dblife.multicut.bin.multicut_roundrobin_busy.splits.{}.step_initial.0.1.step_decay.0.95.log')]

svm_speedup     = [('Hogwild', base_dir + 'svm.bin.svm_generic.splits.{}.step_initial.0.5.step_decay.0.8.log'),
                      ('Round Robin 10',  base_dir + 'svm.bin.svm_delay_roundrobin_busy.splits.{}.step_initial.0.5.step_decay.0.8.log')]

matrix_speedup  = [('Hogwild', base_dir + 'huge.matrix.bin.tracenorm_generic.splits.{}.step_initial.0.1.step_decay.0.95.log')]


speedup_experiments  = [('Abdomen (cut)', abdomen_speedup, '/home/sourish/Hogwild/graphs/abdomen_speedup.pdf'),
                        ('DBLife (multicut)', dblife_speedup, '/home/sourish/Hogwild/graphs/dblife_speedup.pdf'),
                        ('RCV1 (svm)', svm_speedup, '/home/sourish/Hogwild/graphs/svm_speedup.pdf'),
                        ('HUGE (tracenorm)', matrix_speedup, '/home/sourish/Hogwild/graphs/tracenorm_speedup.pdf')]

def produce_speedup_plots(nThreads=10):
    for (title, descs, out_name) in speedup_experiments:
        baselines = descs[0][1].format(1)
        descs     = [ (name, [t_fname.format(i) for i in range(1,nThreads+1)]) for (name, t_fname) in descs]
        pe.plot_speed_graphs(title, baselines, descs, out_name)


## Delay Absolute
delay_values   = [pow(10, i) for i in range(6)] # there are values to 7
delay_template = [('Hogwild (10 Threads)', base_dir + 'delay.experiment.bin.svm_generic_delay.splits.10.delay.{}.log'),
                  ('Round Robin (10 Threads)', base_dir + 'delay.experiment.bin.svm_delay_roundrobin.splits.10.delay.{}.log')]

def produce_delay_absolute():
    descs     = [ (name, [(d,t_fname.format(d)) for d in delay_values]) for (name, t_fname) in delay_template]
    pe.plot_delay_speed_graph('Delay Average Epoch Time (SVM)', descs, '/home/sourish/Hogwild/graphs/delay_svm_absolute.pdf')


splits_delay_template = [('Hogwild (10 Threads)'    , base_dir + 'delay.experiment.bin.svm_generic_delay.splits.{}.delay.{}.log'),
                         ('Round Robin (10 Threads)', base_dir + 'delay.experiment.bin.svm_delay_roundrobin_busy.splits.{}.delay.{}.log')]

def produce_delay_speedup():
    descs     = [ (name, [(d,t_fname.format(1,d), t_fname.format(10,d)) for d in delay_values]) for (name, t_fname) in splits_delay_template]
    pe.plot_delay_speedup('Delay Speedup (SVM)', descs, '/home/sourish/Hogwild/graphs/delay_svm_speedup.pdf')


def main():
    produce_time_error_plots()
    produce_speedup_plots()
    produce_delay_speedup()
    
main()
