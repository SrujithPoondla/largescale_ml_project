###################
# SVM Experiments Data
#

DATA_RCV_SVM_TRAIN = "/home/sourish/Hogwild/data/RCV.binary.train.dat"
DATA_RCV_SVM_TEST = "/home/sourish/Hogwild/data/RCV.test.dat"


###################
# Cut Experiments Data
#

DATA_ABDOMEN_EDGES = "/home/sourish/Hogwild/data/abdomen.binary.dat"
DATA_ABDOMEN_CUT_SOURCES = "/home/sourish/Hogwild/data/one.txt"
DATA_ABDOMEN_CUT_SINKS = "/home/sourish/Hogwild/data/two.txt"


###################
# Multicut Experiments Data
#

DATA_DBLIFE_MULTICUT_EDGES = "/home/sourish/Hogwild/data/dblife.binary.examples.dat"
DATA_DBLIFE_MULTICUT_TERMINALS = "/home/sourish/Hogwild/data/T.txt"


####################
# Matrix Factorization Data
#

DATA_MATRIX_HUGE_TRAIN = "/home/sourish/Hogwild/data/HUGE.train.nr=10000000.alpha=1.rrank=10.oversample=10.noise_var=0.001.factor_type=0.data"
DATA_MATRIX_HUGE_PROBE = "/home/sourish/Hogwild/data/HUGE.probe.nr=10000000.alpha=1.rrank=10.oversample=10.noise_var=0.001.factor_type=0.data"
DATA_MATRIX_KDDCUP_TRAIN = "/home/sourish/Hogwild/data/kdd.cup.trainIDx1.binary.txt"
DATA_MATRIX_KDDCUP_PROBE = "/home/sourish/Hogwild/data/validationIdx1.dat"
DATA_MATRIX_NETFLIX_TRAIN = "/home/sourish/Hogwild/data/netflix.train.dat"
DATA_MATRIX_NETFLIX_PROBE = "/home/sourish/Hogwild/data/netflix.probe.dat"



####################
# On/Off Switches
#

RUN_SVM_RCV = True
RUN_MULTICUT_DBLIFE = True
RUN_MATRIX_NETFLIX = True
RUN_CUT_ABDOMEN = False
RUN_MATRIX_KDDCUP = False
RUN_MATRIX_HUGE = False

RUN_DELAY_EXPERIMENTS = True
RUN_ATOMIC_EXPERIMENTS = True



####################
# Paramters
#

MAX_THREADS = 48


