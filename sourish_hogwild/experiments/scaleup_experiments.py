from experiments_settings import *

###################
# SVM Experiments
#
def rcv1_string(svm_exec, mu, splits, step_initial, step_decay):
    output_name = "/home/sourish/Hogwild/output/svm.{the_exec}.splits.{splits}.step_initial.{step_initial}.step_decay.{step_decay}.log".format(the_exec=svm_exec.replace('/', '.'), splits=splits, step_initial=step_initial, step_decay=step_decay)
    return "{the_exec} --mu {mu} --binary 1 --splits {splits} --stepinitial {step_initial} --step_decay {step_decay} {train} {test} > {output_name}".format(the_exec=svm_exec, mu=mu, splits=splits, step_initial=step_initial, step_decay=step_decay, train=DATA_RCV_SVM_TRAIN, test=DATA_RCV_SVM_TEST, output_name=output_name)

svm_execs = ["/home/sourish/Hogwild/bin/svm_generic", "/home/sourish/Hogwild/bin/svm_delay_roundrobin_busy"]
svm_mu    = 0.01

def print_svmparm_strings():
    for svm_exec in svm_execs:
        for splits in range(1,11):
            for step_initial in [0.5, 0.1, 0.01]:
                    for step_decay in [0.8, 0.9, 1.0]:
                        print(rcv1_string(svm_exec, svm_mu, splits,step_initial,step_decay))

###################
# Cut Experiments
#

def abdomen_cut_binary(cut_exec, mu, splits, step_initial, step_decay):
    output_name = "/home/sourish/Hogwild/output/abdomen.cut.{the_exec}.splits.{splits}.step_initial.{step_initial}.step_decay.{step_decay}.log".format(the_exec=cut_exec.replace('/', '.'), splits=splits, step_initial=step_initial, step_decay=step_decay)
    return "{the_exec} --mu {mu} --binary 1 --splits {splits} --stepinitial {step_initial} --step_decay {step_decay} {edges} {sources} {sinks} > {output_name}".format(the_exec=cut_exec, mu=mu, splits=splits, step_initial=step_initial, step_decay=step_decay, edges=DATA_ABDOMEN_EDGES, sources=DATA_ABDOMEN_CUT_SOURCES, sinks=DATA_ABDOMEN_CUT_SINKS, output_name=output_name)

cut_execs = ["/home/sourish/Hogwild/bin/cut_generic", "/home/sourish/Hogwild/bin/cut_roundrobin_busy"]
def print_abdomen_strings():
    for cut_exec in cut_execs:
        for splits in range(1,11):
            for step_initial in [0.1]:
                    for step_decay in [0.9, 1.0]:
                        print(abdomen_cut_binary(cut_exec, svm_mu, splits,step_initial,step_decay))

#############
# Multicut

def dblife_multi_cut_binary(cut_exec, mu, splits, step_initial, step_decay):
    output_name = "/home/sourish/Hogwild/output/dblife.multicut.{the_exec}.splits.{splits}.step_initial.{step_initial}.step_decay.{step_decay}.log".format(the_exec=cut_exec.replace('/','.'), splits=splits, step_initial=step_initial, step_decay=step_decay)
    return "{the_exec} --mu {mu} --binary 1 --splits {splits} --stepinitial {step_initial} --step_decay {step_decay} {edges} {terminals} > {output_name}".format(the_exec=cut_exec, mu=mu, splits=splits, step_initial=step_initial, step_decay=step_decay, edges=DATA_DBLIFE_MULTICUT_EDGES, terminals=DATA_DBLIFE_MULTICUT_TERMINALS, output_name=output_name)

multicut_execs = ["/home/sourish/Hogwild/bin/multicut_generic", "/home/sourish/Hogwild/bin/multicut_roundrobin_busy"]

def print_dblife_strings():
    for cut_exec in multicut_execs:
        for splits in range(1,11):
            for step_initial in [0.1]:
                    for step_decay in [0.95, 1.0]:
                        print(dblife_multi_cut_binary(cut_exec, svm_mu, splits,step_initial,step_decay))

####################
# Matrix Factorization
#
def matrix_binary(cut_exec, mu, splits, step_initial, step_decay, train_file, probe_file, moniker):
    output_name = "/home/sourish/Hogwild/output/{moniker}.matrix.{the_exec}.splits.{splits}.step_initial.{step_initial}.step_decay.{step_decay}.log".format(the_exec=cut_exec.replace('/','.'), splits=splits, step_initial=step_initial, step_decay=step_decay, moniker=moniker)
    return "{the_exec} --mu {mu} --maxrank 30 --epochs 20 --stepinitial {step_initial} --step_decay {step_decay} --splits {splits} --file_scan 1 {train_file} {probe_file} > {output_name}".format(the_exec=cut_exec, mu=mu, splits=splits, step_initial=step_initial, step_decay=step_decay, output_name=output_name, train_file=train_file, probe_file=probe_file)

matrix_datasets = []

if RUN_MATRIX_NETFLIX: matrix_datasets.append(("netflix", DATA_MATRIX_NETFLIX_TRAIN, DATA_MATRIX_NETFLIX_PROBE))
if RUN_MATRIX_KDDCUP: matrix_datasets.append(("kddcup", DATA_MATRIX_KDDCUP_TRAIN, DATA_MATRIX_KDDCUP_PROBE))
if RUN_MATRIX_HUGE: matrix_datasets.append(("huge", DATA_MATRIX_HUGE_TRAIN, DATA_MATRIX_HUGE_PROBE))


tracenorm_execs = ["/home/sourish/Hogwild/bin/tracenorm_generic", "/home/sourish/Hogwild/bin/tracenorm_roundrobin_busy"]
def print_matrix_strings():
    for cut_exec in tracenorm_execs:
        for splits in range(1,11):
            for step_initial in [0.001]:
                    for step_decay in [0.95]:
                        for (moniker, train_set, probe_set) in matrix_datasets:
                            print(matrix_binary(cut_exec, svm_mu, splits,step_initial,step_decay, train_set, probe_set, moniker))

def print_big_matrix_strings():
    for cut_exec in tracenorm_execs:
        splits = 10
        for step_initial in [0.001]:
            for step_decay in [0.95]:
                for (moniker, train_set, probe_set) in matrix_datasets:
                    print(matrix_binary(cut_exec, svm_mu, splits,step_initial,step_decay, train_set, probe_set, moniker))

def create_scaleup():
    if RUN_SVM_RCV: print_svmparm_strings()
    if RUN_MULTICUT_DBLIFE: print_dblife_strings()
    if RUN_CUT_ABDOMEN: print_abdomen_strings()
    print_matrix_strings()

