INCLUDES=src/freeforall_template src src/instances/cuts src/instances/sparse_svm\
	src/instances/multicuts src/instances/matrix_factor
INCLUDE_STR=$(addprefix -I ,$(INCLUDES))
CXX=g++ -Wall $(INCLUDE_STR)
LIBS=-lpthread -lrt
#CXX=/s/intel_cc/bin/icc 

UNAME=$(shell uname)
LIB_FLAGS=-shared -fPIC
ifeq ($(UNAME), Darwin)
LIBS=-lpthread # WARNING: Need to sync with the above manually
LIB_FLAGS=-dynamiclib -fPIC
endif

EXEC_DIR=bin
FRONTEND_DIR=src/frontends

UTIL_SRC=src/timer.cxx src/timer.h src/examples.cxx src/examples.h src/fvector.cxx src/fvector.h\
	src/simple_random.cxx src/simple_random.h src/frontend_util.cxx src/frontend_util.h src/model.cxx src/model.h\
	src/lock_util.c src/lock_util.h src/file_scanner_util.cxx src/file_scanner_util.h\
	src/freeforall_template/freeforall_template_util.cxx src/test_handler.h src/test_handler.hxx

DEPS=src/freeforall_template/freeforall_template.hxx
ENGINE_SRC=src/hogwild.cxx src/freeforall.cxx 
FREEFORALL=src/freeforall.cxx 
HOGWILD=src/hogwild.cxx

BINARIES=$(EXEC_DIR)/cut_generic $(EXEC_DIR)/cut_generic_global $(EXEC_DIR)/cut_generic_atomic $(EXEC_DIR)/cut_generic_debug\
	$(EXEC_DIR)/cut_roundrobin $(EXEC_DIR)/cut_roundrobin_busy\
	$(EXEC_DIR)/multicut_generic $(EXEC_DIR)/multicut_roundrobin $(EXEC_DIR)/multicut_roundrobin_busy\
	$(EXEC_DIR)/multicut_generic_atomic $(EXEC_DIR)/multicut_generic_global $(EXEC_DIR)/multicut_generic_debug\
	$(EXEC_DIR)/tracenorm_generic $(EXEC_DIR)/tracenorm_atomic $(EXEC_DIR)/tracenorm_roundrobin $(EXEC_DIR)/tracenorm_generic_debug\
	$(EXEC_DIR)/tracenorm_roundrobin_busy $(EXEC_DIR)/tracenorm_roundrobin_busy_debug\
	$(EXEC_DIR)/svm_generic $(EXEC_DIR)/svm_generic_debug $(EXEC_DIR)/svm_roundrobin $(EXEC_DIR)/svm_roundrobin_busy\
	$(EXEC_DIR)/translate

ifneq ($(UNAME), Darwin)
BINARIES+=\
	$(EXEC_DIR)/svm_generic_atomic_delay $(EXEC_DIR)/svm_generic_atomic\
	$(EXEC_DIR)/svm_generic_delay $(EXEC_DIR)/svm_delay_roundrobin $(EXEC_DIR)/svm_delay_roundrobin_busy
endif

all: $(BINARIES)

$(EXEC_DIR)/translate: $(UTIL_SRC) src/translate.cxx
	$(CXX) -O4  -o $@ $(filter %.cxx %c, $^) ${LIBS}

############
# Hog Wild
############
$(EXEC_DIR)/hogwild: $(UTIL_SRC) $(HOGWILD) $(FRONTEND_DIR)/hogwild_frontend.cxx
	$(CXX) -O4  -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/hogwild_debug: $(UTIL_SRC) $(HOGWILD) $(FRONTEND_DIR)/hogwild_frontend.cxx
	$(CXX)  -D__DEBUG -g -o $@ $(filter %.cxx %c, $^) ${LIBS}

#################
## Freeforall
#################
$(EXEC_DIR)/freeforall: $(UTIL_SRC) $(FREEFORALL) $(FRONTEND_DIR)/freeforall_frontend.cxx
	$(CXX) -O4  -o $@ $(filter %.cxx %c, $^) ${LIBS}

# Global lock
$(EXEC_DIR)/freeforall_global: $(UTIL_SRC) $(FREEFORALL) $(FRONTEND_DIR)/freeforall_frontend.cxx
	$(CXX) -O4  -D_GLOBAL_LOCK -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/freeforall_atomic: $(UTIL_SRC) $(FREEFORALL) $(FRONTEND_DIR)/freeforall_frontend.cxx
	$(CXX) -O4  -D_ATOMIC_LOCKING -o $@ $(filter %.cxx %c, $^) ${LIBS}

# Global Read/write lock
$(EXEC_DIR)/freeforall_global_rw: $(UTIL_SRC) $(FREEFORALL) $(FRONTEND_DIR)/freeforall_frontend.cxx
	$(CXX) -O4  -D_GLOBAL_RWLOCK -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/freeforall_debug: $(UTIL_SRC) $(FREEFORALL) $(FRONTEND_DIR)/freeforall_frontend.cxx
	$(CXX)  -D__DEBUG -g -o $@ $(filter %.cxx %c, $^) ${LIBS}

#####################
## Freeforall SVM 
#####################
$(EXEC_DIR)/freeforall_svm: $(UTIL_SRC) src/sparse_svm.cxx $(FRONTEND_DIR)/sparsesvm_frontend.cxx
	$(CXX) -O4  -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/freeforall_svm_global: $(UTIL_SRC) $(FREEFORALL) $(FRONTEND_DIR)/freeforall_frontend.cxx
	$(CXX) -D_GLOBAL_LOCK -O4  -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/freeforall_svm_indep: $(UTIL_SRC) src/sparse_svm_independent.cxx $(FRONTEND_DIR)/sparsesvm_frontend.cxx
	$(CXX) -O4  -o $@ $(filter %.cxx %c, $^) ${LIBS}


$(EXEC_DIR)/freeforall_svm_indep_update: $(UTIL_SRC) src/sparse_svm_independent.cxx $(FRONTEND_DIR)/sparsesvm_frontend.cxx
	$(CXX) -O4 -D_GLOBAL_UPDATE  -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/freeforall_svm_indep_debug: $(UTIL_SRC) src/sparse_svm_independent.cxx $(FRONTEND_DIR)/sparsesvm_frontend.cxx
	$(CXX) -g -D__DEBUG -o $@ $(filter %.cxx %c, $^) ${LIBS}


#####################
## SVM Sparse
#####################

$(EXEC_DIR)/svm_generic: $(UTIL_SRC) src/instances/sparse_svm/sparse_svm_generic.cxx $(DEPS) $(FRONTEND_DIR)/svm_sparse_frontend.cxx
	$(CXX) -O4  -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/svm_generic_atomic: $(UTIL_SRC) src/instances/sparse_svm/sparse_svm_generic.cxx $(DEPS) $(FRONTEND_DIR)/svm_sparse_frontend.cxx
	$(CXX) -O4 -D_ATOMIC_LOCKING -o $@ $(filter %.cxx %c, $^) ${LIBS}


$(EXEC_DIR)/svm_roundrobin: $(UTIL_SRC) src/instances/sparse_svm/sparse_svm_generic.cxx $(DEPS) $(FRONTEND_DIR)/svm_sparse_frontend.cxx
	$(CXX) -O4  -D_ROUND_ROBIN -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/svm_roundrobin_busy: $(UTIL_SRC) src/instances/sparse_svm/sparse_svm_generic.cxx $(DEPS) $(FRONTEND_DIR)/svm_sparse_frontend.cxx
	$(CXX) -O4  -D_ROUND_ROBIN -D_BUSY_WAIT -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/svm_generic_roundrobin: $(UTIL_SRC) src/instances/sparse_svm/sparse_svm_generic.cxx $(DEPS) $(FRONTEND_DIR)/svm_sparse_frontend.cxx
	$(CXX) -O4  -o $@ $(filter %.cxx %c, $^) -lpthread

$(EXEC_DIR)/svm_generic_debug: $(UTIL_SRC) src/instances/sparse_svm/sparse_svm_generic.cxx $(DEPS) $(FRONTEND_DIR)/svm_sparse_frontend.cxx
	$(CXX) -g -D__DEBUG  -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/svm_generic_delay: $(UTIL_SRC) src/instances/sparse_svm/sparse_svm_generic.cxx $(DEPS) $(FRONTEND_DIR)/svm_sparse_frontend.cxx
	$(CXX) -O4  -D_DELAY_ONLY -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/svm_generic_atomic_delay: $(UTIL_SRC) src/instances/sparse_svm/sparse_svm_generic.cxx $(DEPS) $(FRONTEND_DIR)/svm_sparse_frontend.cxx
	$(CXX) -O4 -D_DELAY_ONLY -D_ATOMIC_LOCKING -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/svm_delay_roundrobin: $(UTIL_SRC) src/instances/sparse_svm/sparse_svm_generic.cxx $(DEPS) $(FRONTEND_DIR)/svm_sparse_frontend.cxx
	$(CXX) -O4  -D_DELAY_ONLY -D_ROUND_ROBIN -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/svm_delay_roundrobin_busy: $(UTIL_SRC) src/instances/sparse_svm/sparse_svm_generic.cxx $(DEPS) $(FRONTEND_DIR)/svm_sparse_frontend.cxx
	$(CXX) -O4  -D_DELAY_ONLY -D_BUSY_WAIT -D_ROUND_ROBIN -o $@ $(filter %.cxx %c, $^) ${LIBS}

#####################
## Cut (Generic) 
#####################
$(EXEC_DIR)/cut_generic: $(UTIL_SRC) src/instances/cuts/cut_mainloop.h src/instances/cuts/cut_mainloop.cxx $(DEPS) $(FRONTEND_DIR)/generic_frontend.cxx
	$(CXX) -O4  -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/cut_generic_global: $(UTIL_SRC) src/instances/cuts/cut_mainloop.cxx $(DEPS) $(FRONTEND_DIR)/generic_frontend.cxx
	$(CXX) -D_GLOBAL_LOCK -O4  -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/cut_generic_atomic: $(UTIL_SRC) src/instances/cuts/cut_mainloop.cxx $(DEPS) $(FRONTEND_DIR)/generic_frontend.cxx
	$(CXX) -D_ATOMIC_LOCKING -O4  -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/cut_generic_debug: $(UTIL_SRC) src/instances/cuts/cut_mainloop.cxx $(DEPS) $(FRONTEND_DIR)/generic_frontend.cxx
	$(CXX) -g  -D__DEBUG -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/cut_roundrobin: $(UTIL_SRC) src/instances/cuts/cut_mainloop.cxx $(DEPS) $(FRONTEND_DIR)/generic_frontend.cxx
	$(CXX) -O4  -D_ROUND_ROBIN -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/cut_roundrobin_busy: $(UTIL_SRC) src/instances/cuts/cut_mainloop.cxx $(DEPS) $(FRONTEND_DIR)/generic_frontend.cxx
	$(CXX) -O4 -D_BUSY_WAIT -D_ROUND_ROBIN -o $@ $(filter %.cxx %c, $^) ${LIBS}


#####################
# Multicut (Generic)
#####################
$(EXEC_DIR)/multicut_generic: $(UTIL_SRC) src/instances/multicuts/multicut_mainloop.cxx $(DEPS) $(FRONTEND_DIR)/multicut_frontend.cxx
	$(CXX) -O4  -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/multicut_generic_debug: $(UTIL_SRC) src/instances/multicuts/multicut_mainloop.cxx $(DEPS) $(FRONTEND_DIR)/multicut_frontend.cxx
	$(CXX) -g -D__DEBUG  -o $@ ${LIBS} $(filter %.cxx %c, $^) 

$(EXEC_DIR)/multicut_roundrobin: $(UTIL_SRC) src/instances/multicuts/multicut_mainloop.cxx $(DEPS) $(FRONTEND_DIR)/multicut_frontend.cxx
	$(CXX) -O4  -D_ROUND_ROBIN -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/multicut_roundrobin_busy: $(UTIL_SRC) src/instances/multicuts/multicut_mainloop.cxx $(DEPS) $(FRONTEND_DIR)/multicut_frontend.cxx
	$(CXX) -O4  -D_ROUND_ROBIN -D_BUSY_WAIT -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/multicut_generic_atomic: $(UTIL_SRC) src/instances/multicuts/multicut_mainloop.cxx $(DEPS) $(FRONTEND_DIR)/multicut_frontend.cxx
	$(CXX) -O4 -D_ATOMIC_LOCKING  -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/multicut_generic_global: $(UTIL_SRC) src/instances/multicuts/multicut_mainloop.cxx $(DEPS) $(FRONTEND_DIR)/multicut_frontend.cxx
	$(CXX) -O4 -D_GLOBAL_LOCK  -o $@ $(filter %.cxx %c, $^) ${LIBS}

###################################
# Matrix Factorization (Generic)
###################################
$(EXEC_DIR)/tracenorm_generic: $(UTIL_SRC) src/instances/matrix_factor/tracenorm_factor.h src/instances/matrix_factor/tracenorm_factor.cxx $(DEPS) $(FRONTEND_DIR)/tracenorm_frontend.cxx
	$(CXX) -O4  -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/tracenorm_generic_debug: $(UTIL_SRC) src/instances/matrix_factor/tracenorm_factor.h src/instances/matrix_factor/tracenorm_factor.cxx $(DEPS) $(FRONTEND_DIR)/tracenorm_frontend.cxx
	$(CXX) -g  -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/tracenorm_atomic: $(UTIL_SRC) src/instances/matrix_factor/tracenorm_factor.h src/instances/matrix_factor/tracenorm_factor.cxx $(DEPS) $(FRONTEND_DIR)/tracenorm_frontend.cxx
	$(CXX) -O4  -D_ATOMIC_LOCKING -o $@ $(filter %.cxx %c, $^) ${LIBS}


$(EXEC_DIR)/tracenorm_roundrobin: $(UTIL_SRC) src/instances/matrix_factor/tracenorm_factor.h src/instances/matrix_factor/tracenorm_factor.cxx $(DEPS) $(FRONTEND_DIR)/tracenorm_frontend.cxx
	$(CXX) -g  -D_ROUND_ROBIN -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/tracenorm_roundrobin_busy: $(UTIL_SRC) src/instances/matrix_factor/tracenorm_factor.h src/instances/matrix_factor/tracenorm_factor.cxx $(DEPS) $(FRONTEND_DIR)/tracenorm_frontend.cxx
	$(CXX) -O4 -D_BUSY_WAIT -D_ROUND_ROBIN -o $@ $(filter %.cxx %c, $^) ${LIBS}

$(EXEC_DIR)/tracenorm_roundrobin_busy_debug: $(UTIL_SRC) src/instances/matrix_factor/tracenorm_factor.h src/instances/matrix_factor/tracenorm_factor.cxx $(DEPS) $(FRONTEND_DIR)/tracenorm_frontend.cxx
	$(CXX) -g -D_BUSY_WAIT -D_ROUND_ROBIN -o $@ $(filter %.cxx %c, $^) ${LIBS}

# clean
clean:
	@rm -f *~ *.o src/*.o
	@make -C cython_wrappers clean

realclean: clean
	@rm -f $(BINARIES) *~
