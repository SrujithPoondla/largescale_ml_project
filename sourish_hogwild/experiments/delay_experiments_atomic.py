from experiments_settings import *



command_string = "{exec_string} --splits {splits} --delay {delay} --epochs 7 --binary 1 --step_decay 0.95 %s %s > {output_name}" % (DATA_RCV_SVM_TRAIN, DATA_RCV_SVM_TEST)

def delay_experiment(exec_string, splits,delay):
    output_name = "/home/sourish/Hogwild/output/delay.experiment.{exec_string}.splits.{splits}.delay.{delay}.log".format(exec_string=exec_string.replace('/','.'), splits=splits, delay=delay)
    print(command_string.format(exec_string=exec_string, splits=splits, delay=delay, output_name=output_name));

def main_delay_experiment():
    if not RUN_SVM_RCV: return
    for exec_string in ["/home/sourish/Hogwild/bin/svm_generic_atomic_delay"]:
        for exponent in range(7):
            delay = pow(10, exponent)
            delay_experiment(exec_string, MAX_THREADS, delay)
            delay_experiment(exec_string, 1, delay)

"""
def delay_scaleup():
    for exec_string in ["/home/sourish/Hogwild/bin/svm_generic_atomic_delay"]:
        delay = 500000
        for nThreads in range(1,11):
            delay_experiment(exec_string, nThreads, delay)
"""


if __name__ == "__main__":
    main_delay_experiment()
