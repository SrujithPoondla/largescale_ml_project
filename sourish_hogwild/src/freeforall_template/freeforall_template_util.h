#ifndef _freeforall_template_util_h
#define _freeforall_template_util_h
#include "simple_random.h"
#include "file_scanner.h"


// Exposed as helper functions
void  permute(simple_random &rand, int *d, int n);
// an allocation functionc
int*  init_permutation(int nSize);



// This is weird but what we need
// we allocate a max size, but we pretend to be smaller.
class permutation_buffer  {
  simple_random &rand;
  int *buffer;
  int max_size, current_size;
 public:
 permutation_buffer(simple_random &_rand, int _max_size) : rand(_rand), max_size(_max_size) {
    assert(max_size > 0);
    buffer = init_permutation(max_size);
  }
  void permute_buffer(int new_size) {
    assert(new_size <= max_size); // no allocations
    if(current_size != new_size) {
      for(register int i = new_size - 1; i >= 0; i--) buffer[i] = i;
    }
    current_size = new_size;
    permute(rand, buffer, current_size);
  }
  int *get_permutation(int &_current_size) { _current_size = current_size; return buffer; }
};

template<class E>
struct
permute_thread_info {
  scanner<E> *_scanner;
  permutation_buffer *pb;
  permute_thread_info(permutation_buffer *_pb, scanner<E> *s) :  _scanner(s),pb(_pb) { } 
};


template<class E> 
void*
permute_thread( void* p ) {
  struct permute_thread_info<E> *pti = (struct permute_thread_info<E>*) p;
  int read_items = 0;
  pti->_scanner->retrieve_next_segment(read_items); 
  pti->pb->permute_buffer(read_items);
  return NULL;
}



#endif
