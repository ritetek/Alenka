/*
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


using namespace std; 
 
#ifndef ADD_H_GUARD
#define ADD_H_GUARD

#include <thrust/device_vector.h>
#include <thrust/sequence.h>
#include <thrust/count.h>
#include <thrust/copy.h>
#include <thrust/iterator/counting_iterator.h>
#include <thrust/iterator/constant_iterator.h>
#include <thrust/iterator/discard_iterator.h>
#include <thrust/adjacent_difference.h>
#include <thrust/partition.h>
#include <thrust/reduce.h>
#include <thrust/fill.h>
#include <thrust/scan.h>
#include <thrust/device_ptr.h>
#include <thrust/unique.h>
#include <thrust/gather.h>
#include <thrust/sort.h>
#include <thrust/functional.h>
#include <boost/unordered_map.hpp>
#include <queue>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <stack>
#include "strings.h"
#include "itoa.h"
#include <ctime>

typedef double float_type;
typedef long long int int_type;
typedef thrust::device_vector<int_type>::iterator ElementIterator_int;
typedef thrust::device_vector<float_type>::iterator ElementIterator_float;
typedef thrust::device_vector<unsigned int>::iterator   IndexIterator;


extern size_t int_size;
extern size_t float_size;
extern unsigned long long int hash_seed;
extern queue<string> op_type;
extern queue<string> op_value;
extern queue<int_type> op_nums;
extern queue<float_type> op_nums_f;
extern queue<string> col_aliases;
extern unsigned int curr_segment;
extern unsigned long long int total_count;
extern unsigned int total_segments;
extern unsigned int total_max;
extern unsigned int process_count;
extern long long int totalRecs;
extern bool fact_file_loaded;
extern char map_check;
extern unsigned int oldCount;
extern void* alloced_tmp;
extern unsigned int alloced_sz;

template<typename T>
  struct uninitialized_host_allocator
    : std::allocator<T>
{
  // note that construct is annotated as
    __host__ __device__
  void construct(T *p)
  {
    // no-op
  }
};


template <typename HeadFlagType>
struct head_flag_predicate
        : public thrust::binary_function<HeadFlagType,HeadFlagType,bool>
{
    __host__ __device__
    bool operator()(HeadFlagType left, HeadFlagType right) const
    {
        return !left;
    }
};

struct float_to_long
{

    __host__ __device__
    long long int operator()(const float_type x)
    {
        if ((long long int)((x+EPSILON)*100.0) > (long long int)(x*100.0))
            return (long long int)((x+EPSILON)*100.0);
        else return (long long int)(x*100.0);


    }
};


#ifdef _WIN64
typedef unsigned __int64 uint64_t;
#endif

uint64_t MurmurHash64A ( const void * key, int len, unsigned int seed );
uint64_t MurmurHash64B ( const void * key, int len, unsigned int seed );
int_type reverse_op(int_type op_type);
size_t getFreeMem();



class CudaSet
{
public:
    std::vector<thrust::host_vector<int_type, uninitialized_host_allocator<int_type> > > h_columns_int;
    std::vector<thrust::host_vector<float_type, uninitialized_host_allocator<float_type> > > h_columns_float;
    std::vector<char*> h_columns_char;	
	unsigned int prealloc_char_size;

    std::vector<thrust::device_vector<int_type> > d_columns_int;
    std::vector<thrust::device_vector<float_type> > d_columns_float;
    std::vector<char*> d_columns_char;	

    std::vector<unsigned int> char_size;
    thrust::device_vector<unsigned int> prm_d;
    std::vector<unsigned int*> prm; //represents an op's permutation of original data vectors
    std::vector<unsigned int> prm_count;	// counts of prm permutations	
    std::vector<char> prm_index; // A - all segments values match, N - none match, R - some may match 
    std::map<unsigned int, unsigned int> type_index;
    std::map<int,bool> uniqueColumns;


    unsigned int mColumnCount;
    unsigned int mRecCount;
    std::map<string,int> columnNames;
    std::map<string, FILE*> filePointers;
    bool *grp;
    queue<string> columnGroups;
    bool not_compressed; // 1 = host recs are not compressed, 0 = compressed
    FILE *file_p;
    unsigned int *seq;
    bool keep;
    unsigned int segCount, maxRecs;
    string name;
    char* load_file_name;
    unsigned int oldRecCount;
    bool source, text_source;
	

    unsigned int* type; // 0 - integer, 1-float_type, 2-char
    bool* decimal; // column is decimal - affects only compression
    unsigned int* grp_type; // type of group : SUM, AVG, COUNT etc
    unsigned int* cols; // column positions in a file
    unsigned int grp_count;
    bool partial_load;
    

    CudaSet(queue<string> &nameRef, queue<string> &typeRef, queue<int> &sizeRef, queue<int> &colsRef, int_type Recs);
    CudaSet(queue<string> &nameRef, queue<string> &typeRef, queue<int> &sizeRef, queue<int> &colsRef, int_type Recs, char* file_name);
    CudaSet(unsigned int RecordCount, unsigned int ColumnCount);
    CudaSet(CudaSet* a, CudaSet* b, int_type Recs, queue<string> op_sel, queue<string> op_sel_as);
    ~CudaSet();
    void allocColumnOnDevice(unsigned int colIndex, unsigned int RecordCount);	    
    void decompress_char_hash(unsigned int colIndex, unsigned int segment, unsigned int i_cnt);	
    bool isUnique(unsigned int colIndex);
    void add_hashed_strings(string field, unsigned int segment, unsigned int i_cnt);
    void resize(unsigned int addRecs);
	void reserve(unsigned int Recs);
    void deAllocColumnOnDevice(unsigned int colIndex);
    void allocOnDevice(unsigned int RecordCount);
    void deAllocOnDevice();
    void resizeDeviceColumn(unsigned int RecCount, unsigned int colIndex);
    void resizeDevice(unsigned int RecCount);
    bool onDevice(unsigned int i);
    CudaSet* copyDeviceStruct();
    unsigned long long int readSegmentsFromFile(unsigned int segNum, unsigned int colIndex);
    void decompress_char(FILE* f, unsigned int colIndex, unsigned int segNum);
    void CopyToGpu(unsigned int offset, unsigned int count);
    void CopyColumnToGpu(unsigned int colIndex,  unsigned int segment);
    void CopyColumnToGpu(unsigned int colIndex);
    void CopyColumnToGpu(unsigned int colIndex,  unsigned int offset, unsigned int count);
    void CopyColumnToHost(int colIndex, unsigned int offset, unsigned int RecCount);
    void CopyColumnToHost(int colIndex);
    void CopyToHost(unsigned int offset, unsigned int count);
    float_type* get_float_type_by_name(string name);
    int_type* get_int_by_name(string name);
    float_type* get_host_float_by_name(string name);
    int_type* get_host_int_by_name(string name);
    void GroupBy(std::stack<string> columnRef, unsigned int int_col_count);
    void addDeviceColumn(int_type* col, int colIndex, string colName, int_type recCount);
    void addDeviceColumn(float_type* col, int colIndex, string colName, int_type recCount);
    void addHostColumn(int_type* col, int colIndex, string colName, int_type recCount, int_type old_reccount, bool one_line);
    void addHostColumn(float_type* col, int colIndex, string colName, int_type recCount, int_type old_reccount, bool one_line);
    void writeHeader(char* file_name, unsigned int col); 
    void Store(char* file_name, char* sep, unsigned int limit, bool binary);
    void compress_char(string file_name, unsigned int index, unsigned int mCount);
    void LoadFile(char* file_name, char* sep );
    int LoadBigFile(const char* file_name, const char* sep);
    void free();
    bool* logical_and(bool* column1, bool* column2);
    bool* logical_or(bool* column1, bool* column2);
    bool* compare(int_type s, int_type d, int_type op_type);
    bool* compare(float_type s, float_type d, int_type op_type);
    bool* compare(int_type* column1, int_type d, int_type op_type);
    bool* compare(float_type* column1, float_type d, int_type op_type);
    bool* compare(int_type* column1, int_type* column2, int_type op_type);
    bool* compare(float_type* column1, float_type* column2, int_type op_type);
    bool* compare(float_type* column1, int_type* column2, int_type op_type);
    float_type* op(int_type* column1, float_type* column2, string op_type, int reverse);
    int_type* op(int_type* column1, int_type* column2, string op_type, int reverse);
    float_type* op(float_type* column1, float_type* column2, string op_type, int reverse);
    int_type* op(int_type* column1, int_type d, string op_type, int reverse);
    float_type* op(int_type* column1, float_type d, string op_type, int reverse);
    float_type* op(float_type* column1, float_type d, string op_type,int reverse);

protected:

    void initialize(queue<string> &nameRef, queue<string> &typeRef, queue<int> &sizeRef, queue<int> &colsRef, int_type Recs, char* file_name);
    void initialize(queue<string> &nameRef, queue<string> &typeRef, queue<int> &sizeRef, queue<int> &colsRef, int_type Recs);
    void initialize(unsigned int RecordCount, unsigned int ColumnCount);
    void initialize(CudaSet* a, CudaSet* b, int_type Recs, queue<string> op_sel, queue<string> op_sel_as);
};

extern map<string,CudaSet*> varNames; //  STL map to manage CudaSet variables
extern map<string,string> setMap; //map to keep track of column names and set names

void allocColumns(CudaSet* a, queue<string> fields);
unsigned int largest_prm(CudaSet* a);
void gatherColumns(CudaSet* a, CudaSet* t, string field, unsigned int segment, unsigned int& count);
unsigned int getSegmentRecCount(CudaSet* a, unsigned int segment);
void copyColumns(CudaSet* a, queue<string> fields, unsigned int segment, unsigned int& count);
void setPrm(CudaSet* a, CudaSet* b, char val, unsigned int segment);
void mygather(unsigned int tindex, unsigned int idx, CudaSet* a, CudaSet* t, unsigned int offset, unsigned int g_size);
void mycopy(unsigned int tindex, unsigned int idx, CudaSet* a, CudaSet* t, unsigned int offset, unsigned int g_size);
unsigned int load_queue(queue<string> c1, CudaSet* right, bool str_join, string f2, unsigned int &rcount);
unsigned int max_char(CudaSet* a);
unsigned int max_tmp(CudaSet* a);    
void reset_offsets();

#endif


#  define CUDA_SAFE_CALL_NO_SYNC( call) do {                                \
    cudaError err = call;                                                    \
    if( cudaSuccess != err) {                                                \
        fprintf(stderr, "Cuda error in file '%s' in line %i : %s.\n",        \
                __FILE__, __LINE__, cudaGetErrorString( err) );              \
        exit(EXIT_FAILURE);                                                  \
    } } while (0)
#  define CUDA_SAFE_CALL( call) do {                                        \
    CUDA_SAFE_CALL_NO_SYNC(call);                                            \
    cudaError err = cudaThreadSynchronize();                                 \
    if( cudaSuccess != err) {                                                \
        fprintf(stderr, "Cuda error in file '%s' in line %i : %s.\n",        \
                __FILE__, __LINE__, cudaGetErrorString( err) );              \
        exit(EXIT_FAILURE);                                                  \
    } } while (0)

#define EPSILON    (1.0E-8)


