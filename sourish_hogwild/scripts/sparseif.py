import sys

def read_topics(topicName):
    d = dict()
    with open(topicName, 'r') as f:
        for l in f:
            v = l.split(' ')
            d[v[1]] = v[0]
    
    return d

def read_vector(vectorName, labelDict, start_uid, outf):
    uniqs = dict()
    uid   = start_uid
    with open(vectorName, 'r') as f:
        for l in f:
            split_line = l.split()
            label = -1
            tid   = split_line[0]
            assert(tid not in uniqs)
            uniqs[tid] = uid
            uid = uniqs[tid]

            if labelDict[tid] == 'CCAT': label = 1
            outf.write("{} {} {}\n".format(uid, -1, label))
            
            for x in split_line[1:]: # two spaces.
                [index, value] = x.split(':')
                outf.write("{id} {index} {value}\n".format(id=uid, index=index, value=value))
            uid += 1
    f.close()
    return uid

def train_sets(start_uid, train_fname):
    outf = open(train_fname, 'w+')
    d    = read_topics('rcv1-v2.topics.qrels')
    fs   = ['lyrl2004_vectors_test_pt0.dat', 'lyrl2004_vectors_test_pt1.dat', 'lyrl2004_vectors_test_pt2.dat', 'lyrl2004_vectors_test_pt3.dat']
    uid  = start_uid
    for fname in fs:
        print("Starting {}".format(fname))
        uid = read_vector(fname, d, uid, outf)
    outf.close()
    return uid

def test_set(uid, test_fname):
    outf = open(test_fname, 'w+') 
    d    = read_topics('rcv1-v2.topics.qrels')
    read_vector('lyrl2004_vectors_train.dat', d, uid, outf)
    outf.close()

if len(sys.argv) == 3:
    uid = train_sets(1, sys.argv[1])
    uid = test_set(uid, sys.argv[2])
else:
    print("{} <output train file> <output test file>".format(sys.argv[0]))
