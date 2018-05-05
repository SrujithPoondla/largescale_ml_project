from experiments_settings import *
import scaleup_experiments as se
import delay_experiments as de
import scaleup_experiments_atomic as sea
import delay_experiments_atomic as dea


se.create_scaleup()
se.print_big_matrix_strings()
if RUN_DELAY_EXPERIMENTS: de.main_delay_experiment()

if RUN_ATOMIC_EXPERIMENTS:
    sea.print_matrix_strings()
    if RUN_DELAY_EXPERIMENTS: dea.main_delay_experiment()
