#ifndef LM_H__
#define LM_H__

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem> // Requires C++17
#include <list>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <cmath>

using namespace std;

// Multihash Table N-Gram Language Model

// Data Structures

// Subtable collocate node
struct collocate_node{
  // Key: Collocate (i.e., n-1 words after headword)
  string collocate;
  // Value: Number of occurrences of this n-gram
  unsigned int count;
  // Deleted node flag (for open addressing)
  bool deleted;
};

// Define collocate table type as vector of pointers to collocate nodes
typedef vector<shared_ptr<collocate_node>> ctable;

// Collocate subtable
struct collocate_table{
  // Total capacity of table
  unsigned int capacity;
  // Currently occupied nodes, excluding deleted nodes
  unsigned int occupied;
  // Hash function
  unsigned int (*hash_function)(string key, unsigned int capacity);
  // Pointer to vector of collocate nodes 
  shared_ptr<ctable> table;
};

// Multihash node containing headword as key; collocate subtable, 
// collocate-node pointer array, and count as values; and deleted node flag
struct headword_node{
  // Key: Headword
  string headword;
  // Value 1: Number of occurrences in corpus
  unsigned int count;
  // Value 2: Collocates subtable
  shared_ptr<collocate_table> collocates;
  // Value 3: Frequencies subtable
  shared_ptr<ctable> frequencies;
  // Deleted node flag (for open addressing)
  bool deleted;
};

// Define headword table type as vector of pointers to headword nodes
typedef vector<shared_ptr<headword_node>> hwtable;

// Multihash table containing array of headword nodes, hash function setting, occupied count, and capacity
struct head_table{
  // Total capacity of table
  unsigned int capacity;
  // Currently occupied nodes, excluding deleted nodes
  unsigned int occupied;
  // Hash function
  unsigned int (*hash_function)(string key, unsigned int capacity);
  // Pointer to vector of headword nodes 
  shared_ptr<hwtable> table;
};

class LM{
public:
  // Constructs 2-gram language model object by default
  LM();

  // Constructs language model object with specified n-gram size (e.g., 2-gram, 3-gram, etc.)
  LM(int n);
  
  // Deconstructor;
  ~LM();
  
  // Trains LM with an input text, parsing the text 
  // and inserting n-grams in multihash table and subtables
  // Replaces previous model; returns false if failed
  bool Train(string filename);

  // Returns a map of all n-grams and their respective counts
  map<string, int> NGrams();
  
  // Returns a vector of the x most frequent collocates for a given headword
  vector<string> Collocates(string headword, unsigned int x);

  // Returns the frequency of a specified n-gram; 
  // if unigram entered for bigram or higher model, headword frequency returned
  float Frequency(string ngram);

  // Returns a map of collocates and their counts for a given headword
  map<string, int> CollocateCounts(string headword);

  // Returns the total number of unique words (i.e., unigrams): 
  int UniqueUnigramCount();

  // Returns the total number of unique n-grams
  int UniqueNGramsCount();

  // Returns the total number of n-grams processed (including duplicates)
  int NGramsTotal();

  // Returns total number of tokens (i.e., unigrams/headwords) processed
  int TotalTokens();

  // Generate a CSV file of all n-grams and counts; returns false if failed
  bool CSV(string filename);

  // Removes a specified n-gram
  bool Remove(string ngram);

  // Expand model with additional input text
  // Returns false if failed
  bool Grow(string filename);
   
private:
  // Private data members
  // N-gram size (e.g., 2-gram, 3-gram, etc.)
  unsigned int ngram_size;

  // Headword table
  shared_ptr<head_table> main_table;

  // Count of unique n-grams stored in model
  unsigned int unique_ngrams_count;

  // Total n-grams processed (including duplicates)
  unsigned int ngrams_total;

  // Total tokens (i.e., unigrams/headwords) processed (including duplicates)
  unsigned int tokens;

  // Constants
  // Vector of smallest primes > 2^exponent, 0 <= exponent < 32 for setting table sizes
  // in order to reduce collisions (see Sedgewick, 1998)
  const vector<unsigned int> PRIMES = {2,3,5,11,17,37,67,131,257,521,1031,2053,4099,8209,16411,32771,65537,131101,262147,524309,1048583,2097169,4194319,8388617,16777259,33554467,67108879,134217757,268435459,536870923,1073741827,2147483659};
  // Corresponding vector exponents
  // const vector<unsigned int> EXPONENTS = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};

  // Minimum hash table size
  const unsigned int MIN_SIZE = 7;

  // Linear regression model slope coefficient for estimating the number of words (i.e., tokens)
  // in a UTF-8 text file based on the file size in bytes (i.e., unigrams = BYTES_TO_TOKENS*bytes)
  const float BYTES_TO_TOKENS = 0.175;

  // Private member functions

  // For the purpose of setting initial table size, estimates
  // number of words (i.e., tokens) based on input file size 
  int EstimateTokenCount(string filename);

  // Return exponent of smallest power of 2 greater than or equal to x
  unsigned int PowerOfTwoExponent(unsigned int x);

  // Return smallest prime greater than the smallest power of 2 
  // greater or equal to x for x > minimum table size - 1, or return minimum table size otherwise (see Sedgewick, 1998)
  unsigned int PrimeSize(unsigned int x);
  
  // Initializes headword multihash table 
  shared_ptr<head_table> InitHeadTable(int capacity);
  
  // Initializes collocate hash subtable
  shared_ptr<collocate_table> InitCollocateTable(int capacity);
  
  // Initializes headword node 
  shared_ptr<headword_node> InitHeadword(string headword);
  
  // Initializes collocate node
  shared_ptr<collocate_node> InitCollocate(string collocate);
  
  // Inserts n-gram headword and collocate or increments count if already exists
  // Returns bucket index or -1 if failed
  int InsertNGram(string headword, string collocate);
  
  // Inserts collocate into headword’s subtable or increments count if already exists
  // Returns bucket index or -1 if failed
  int InsertCollocate(shared_ptr<headword_node> headword, string collocate);

  // Find and return headword node or null if not found
  shared_ptr<headword_node> FindHeadword(string headword);
  
  // Find and return collocate node or null if not found
  shared_ptr<collocate_node> FindCollocate(shared_ptr<headword_node> headword, string collocate);

  // Deletes existing model and resets counts
  void Reset();

  // Deletes headword by setting deleted node flag to true and correcting counts
  // Returns true if suceeded
  bool DeleteHeadword(shared_ptr<headword_node> headword);
  
  // Deletes collocate (and headword if appropriate) by setting deleted node flag(s) to true and corrects counts
  // Returns true if succeeded
  bool DeleteCollocate(shared_ptr<headword_node> headword, shared_ptr<collocate_node> collocate);
  
  // Returns head table load
  float GetHeadTableLoad(shared_ptr<head_table> table);
  
  // Returns collocate table load 
  float GetCollocateTableLoad(shared_ptr<collocate_table> table);
  
  // Resizes head table
  void ResizeHeadTable(shared_ptr<head_table> table);
  
  // Resizes collocate table 
  void ResizeCollocateTable(shared_ptr<headword_node> headword, shared_ptr<collocate_table> table);

  // Insert existing headword node into new headword table
  int InsertHeadNode(shared_ptr<head_table> table, shared_ptr<headword_node> node);

  // Insert existing collocate node into new subtable
  int InsertCollocateNode(shared_ptr<collocate_table> table, shared_ptr<collocate_node> node);

  // Sorts all frequency subtables after building or expanding LM
  void SortCounts();

  // Separates n-gram string into headword and collocate returned as vector
  vector<string> SplitNGram(string ngram);
};

#endif // LM_H__