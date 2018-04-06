#ifndef _file_scanner_h
#define _file_scanner_h
#include <cstdio>
#include <assert.h>
#include <iostream>

template<class T>
class scanner {
public:
  virtual bool       completed_scan()  = 0;
  virtual const T*   get_active_segment(int &elements_read)   = 0;
  virtual void       retrieve_next_segment(int &elements_read) = 0;
  virtual ~scanner() { };
  virtual int get_max_size() = 0;
};

// It's up to the user that T is a reasonable struct.
template<class T>
class file_scanner : public scanner<T> {
  const char*_filename;
  T* buffers[2];
  int valid_elements[2];
  FILE* f;
  int nExamples, active_buffer, buffer_size;
  // LastValid indicates that the segment that is active is the last 
  // segment to be read.
  bool bLastValid, bCompleted_scan;
  void restart_scan() { fseek(f, sizeof(int), SEEK_SET); }
 public:
  // buffer size is the number of T sized items
  // We assume the buffer has the awesome format:
  // nElements (as an int)
  // <data>
  file_scanner(const char *filename, int _buffer_size) : buffer_size(_buffer_size) {        
    f                = fopen(filename, "rb");
    assert(f != NULL);        
    assert(fread(&nExamples, sizeof(int), 1, f)==1); 
    buffers[0]       = new T[buffer_size];
    buffers[1]       = new T[buffer_size];    
    std::cerr << "[file_scanner] created buffers of size " << sizeof(T) * buffer_size << std::endl;
    bCompleted_scan   = false;
    bLastValid        = false;
    active_buffer     = 1; // so it reads into buffer 0.
    int _tmp;
    retrieve_next_segment(_tmp);
  }

  int get_max_size() { return buffer_size; }
  bool completed_scan() { return bCompleted_scan; }
  ~file_scanner() {
    fclose(f);
    delete buffers[0]; delete buffers[1];
  }
  // Caller's responsibility to fill the buffer.
  const T* get_active_segment(int &elements_read) {    
    elements_read   = valid_elements[active_buffer];
    // update the scan completed flag
    bCompleted_scan = bLastValid;
    bLastValid      = false;
    return buffers[active_buffer];
  }
  
  // Changes the active buffer index. 
  // Assumes on entry the file pointer is not to an feof.
  // Ensures on exit that the file pointer is not an feof.
  // Sets bLastValid on each complete iteration through the file.
  void
    retrieve_next_segment(int &elements_read) {
    active_buffer                 = (active_buffer == 0) ? 1 : 0;
    elements_read                 = fread(buffers[active_buffer], sizeof(T), buffer_size, f);
    bLastValid                    = feof(f);
    
    if(elements_read < buffer_size) {restart_scan();}    
    valid_elements[active_buffer] = elements_read;
  }  
  
};

template<class T>
class
memory_scanner : public scanner<T> {
  T* buf; int buffer_size;
 public:
  int get_max_size() { return buffer_size; }
  memory_scanner(int _buffer_size, T* buffer) : buf(buffer),buffer_size(_buffer_size)  {         }
  ~memory_scanner() { } 
  const T* get_active_segment(int &elements_read) { elements_read = buffer_size; return buf; }
  bool completed_scan()        { return true; }
  void retrieve_next_segment(int &elements_read) { elements_read = buffer_size ; }  
};

template<class T, class STATE>
void
scanner_map(scanner<T> *s, STATE &state, void (*foo)(STATE &x, const T &e)) {
  do {
    int nExamples = 0;
    const T* buf = s->get_active_segment(nExamples);
    for(int i = nExamples - 1; i >= 0; i--) {foo(state, buf[i]);}
    s->retrieve_next_segment(nExamples);
  } while(!s->completed_scan());
}

// template<struct T>
// scanner<T> get_scanner() {  }
/*
void 
sample_code() {
  do {
    T* buf = get_active_scan(nExamples);
    retrieve_next_segment();
  } while(!completed_scan());
}
*/
#endif
