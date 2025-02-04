# Multihash Table N-Gram Language Model #

## Summary ##

This project implements a _multihash table n-gram language model_ class in C++. An _n-gram_ is a set of _n_ words which occur together in natural language (e.g., "ice cream"). Applications of n-gram language models include predictive text, machine translation, syntax-dependent spell check, speech/handwriting recognition, spam filters, and plagiarism detection applications (Jurafsky & Martin, 2008). This implementation parses a plain-text file of natural written language, stores the parsed data, and accesses it efficiently with O(1) constant runtime complexity. The multihash table data structure in this project is implemented as a main hash table with two subtables and additional metadata. One subtable is a hash table, and the other is a dynamic array sorted by n-gram frequency. See below for more detail on n-gram language models, multihash tables, and the data structures and functions for this implementation.

## Directory Tree ##

This project has the following directory tree:
```
.  
├── CMakeLists.txt  
├── CMakeLists.txt.in  
├── README.md  
├── app  
│   └── main.cpp  
├── build  
│   ├── run_app  
│   └── run_tests  
├── code  
│   ├── LM.cpp  
│   └── LM.h  
├── corpus  
│   ├── GettysburgAddress.txt  
│   ├── Macbeth.txt  
│   ├── MyAntoniaChaps1-9.txt  
│   ├── OneBigram.txt  
│   └── TaleOfTwoCitiesChapter1.txt  
├── results  
│   ├── GettysburgAddress.csv  
│   └── MyAntoniaChaps1-9.csv  
└── tests  
    └── test_LM.cpp  
```

| File                                          | Description                                               |
| --------------------------------------------- | --------------------------------------------------------- |
| CMakeLists.txt                                | Compilation configuration file                            |
| CMakeLists.txt.in                             | Compilation configuration file                            |
| __README.md__                                 | __This ReadMe markdown file__                             |
| app\                                          | Application directory                                     |  
| &nbsp;&nbsp;&nbsp;main.cpp                    | Demo application code                                     | 
| build\                                        | Compilation file directory                                |
| &nbsp;&nbsp;&nbsp;run_app                     | Executable: Demonstrates class                            | 
| &nbsp;&nbsp;&nbsp;run_tests                   | Executable: Performs unit tests                           | 
| code\                                         | Class code directory                                      |   
| &nbsp;&nbsp;&nbsp;LM.cpp                      | Language model class source file                          |
| &nbsp;&nbsp;&nbsp;LM.h                        | Language model class header file                          |
| corpus\                                       | Sample corpus texts directory \*                          |   
| &nbsp;&nbsp;&nbsp;GettysburgAddress.txt       | Lincoln's 'Gettysburg Address' UTF-8 text file            |
| &nbsp;&nbsp;&nbsp;Macbeth.txt                 | Shakespeare's 'Macbeth' UTF-8 text file                   |
| &nbsp;&nbsp;&nbsp;MyAntoniaChaps1-9.txt       | Cather's 'My Antonia,' Chap. 1-9, UTF-8 text file         |
| &nbsp;&nbsp;&nbsp;OneBigram.txt               | UTF-8 text test file with only two words: 'one bigram'    |
| &nbsp;&nbsp;&nbsp;TaleOfTwoCitiesChapter1.txt | Dickens' 'A Tale of Two Cities,' Chap. 1, UTF-8 text file |
| results\                                      | Output directory for CSV files of n-grams & counts        |   
| &nbsp;&nbsp;&nbsp;GettysburgAddress.csv       | 'Gettysburg Address' CSV output file                      |
| &nbsp;&nbsp;&nbsp;MyAntoniaChaps1-9.csv       | 'My Antonia,' Chap. 1-9, CSV output file                  |
| tests\                                        | Unit tests directory                                      |
| &nbsp;&nbsp;&nbsp;test_LM.cpp                 | Unit tests code                                           |

\* Note: All of the corpus texts are public domain works from Project Gutenberg.

## Compilation and Execution ##
To compile the code and create executables in a Linux operating system environment:
1. Change to the _build_ directory: ```cd build```
2. Run _cmake_ to create the compilation configuration files: ```.. cmake```
3. Run _make_ to compile the executables: ```make```

This will create two executables in the _build_ directory: 
* ```run_app```: Demonstrates the language model class with sample texts
* ```run_tests```: Runs unit tests on the class

To run these these executables:
1. Change to the _build_ directory: ```cd build```
2. Enter ```./run_app``` or ```./run_tests``` accordingly


## N-Gram Language Models Background ##

Predictive text, machine translation, syntax-dependent spell check, speech/handwriting recognition, spam filters, and plagiarism detection applications require a reliable language model (Jurafsky & Martin, 2008). Although complex syntax parse trees/forests are utilized in some specific contexts, probabilistic _n-gram_ language models which use the frequency of certain combinations of _n_ words (i.e., collocations) occurring in natural language can be surprisingly effective in many such applications. As an example, given the word “ice,” a word likely to follow it might be “cream,” so “ice cream” would be a frequent 2-gram (or bigram) for the headword “ice.”

## Challenges of Implementing N-Gram Language Models ##

To create an n-gram language model (LM), it is necessary to process large corpora of texts, calculating the frequency of each n-gram. Then, to apply the model, it is necessary to quickly obtain those frequencies either to predict the next word, or in the case of speech/handwriting recognition, to decide which interpretation of the input is most likely. Not only must each unique word in the corpus be mapped to the number of times it occurs in the corpus, it is also necessary to track the frequency of the potentially large sets of the $n-1$ words (i.e., collocate) that follow any headword. Thus, each headword must also be mapped to a sublist of the collocates that follow it.

What’s more, every time a particular n-gram is encountered in the corpus, the count must be incremented, so it is necessary to search through a headword’s sublist for the corresponding n-gram each time. If there are $k_i$ unique n-grams for any given headword $i$ and there are $j_i$ occurrences of each unique n-gram containing headword $i$ in the corpus, then there must be ${j_i}^{k_i}$ searches and updates in building just one headword’s collocate frequency sublist. If there are $m$ unique headwords in the corpus, then $\sum_{i}^{m}{{j_i}^{k_i}}$ searches and updates must be performed when building the model. If each individual search has linear time complexity, the runtime can quickly add up for a large corpus. Therefore, a linked list is not an ideal data structure for the sublist.

A third challenge is that after the model is built, the most frequent n-grams for any given headword must be accessed quickly when applying the model. Thus, even if the sublist were sorted alphabetically to optimize search and update during the model training phase, the sublist must then be sorted by frequency to facilitate quick access to the frequency data. Furthermore, if the user wishes to improve the model by adding additional n-grams from another corpus, the sublist must again be sorted alphabetically for training and then resorted by frequency once again for accessing the frequency data. As there are _m_ sublists, sorting and resorting can be an expensive operation for a large corpus.

A fourth challenge is that the number of unique words and unique n-grams contained in the corpus is unknown before processing, so the top list and sublists must be able to expand dynamically as the model is trained. Although this expansion can be achieved with linked lists and binary search trees, as explained above, linked lists have linear search time, and binary search trees cannot be simultaneously sorted by two different criteria.

## Proposed Solution ##

To address these challenges, I implemented a multihash table data structure with embedded hash subtables. A hash table is an abstract data structure that places and locates data based on a hash function which transforms a value from the data (e.g., a name or ID number) into a key or index. Hash tables are time-efficient with average constant time complexity for insert, search, and remove. Whereas a hash table allows only one value per key, a multihash allows multiple values per key (Gregoire, 2018, p. 132; “Multimap,” 2022), and for this implementation, those multiple values are contained in embedded hash subtables. Thus, while based on the principles of a hash table, a multihash structure with embedded hash subtables requires different structures and member functions than a hash table. In other words, it is not simply a matter of using a hash table class. 

More specifically, for this implementation, the upper-level multihash table contains each unique headword in the corpus with the headword hashed as the key. Each headword maps to an embedded subtable of each of the unique sets of collocates following the headword, with the collocate hashed as the key. For example, for the bigram “ice cream,” “ice” would be hashed as a key and indexed in the headword table, and that node would contain a subtable containing the collocate “cream” hashed as the key mapped to a value of the number of occurrences of that particular bigram. Because hash table search, insert, and remove have average constant time (Leiserson, 2005; Sedgewick, 1998), using a multihash with an embedded hash table will optimize runtime in the model training phase, as compared to the linear time of inserting into linked lists and the logarithmic time of balanced trees.

To address the third challenge of accessing the frequency data quickly while maintaining the ability to add more n-grams to the model, when a collocate is added to a subtable, a pointer to that collocate is also added to a dynamic array in each headword node. When model training is completed, the array of pointers is sorted by frequency. This allows quick access to the most frequent n-grams, permits the later expansion of the model, and avoids taking up additional storage with duplicated data.

To address the fourth challenge, the initial hash table sizes are estimated from the raw corpus file size and are automatically expanded when necessary. In addition, following Sedgewick's (1998) recommendations, prime number hash table sizes are used to reduce collisions (i.e., instances where the hash function returns the same index for two different elements), linear probing (i.e., trying the next available index in the table) is used to handle any collisions, and the tables are resized when the load exceeds or falls below specified thresholds. For the hash function, the Fowler-Noll-Vo 1a non-cryptographic hash function (FNV-1a, Fowler et al., 2019) was selected because, although not perfect, it results in relatively fewer collisions for alphanumeric keys, is relatively fast, and is relatively simple to implement compared to other common hash functions (Estébanez et al., 2014; Henke et al., 2008; Karasek et al., 2011, van den Bosch, 2015). Collectively, these measures help to ensure average constant runtime complexity.

The multihash table n-gram language model (LM) class has the following data structures and member functions:

### Data Structures ###

*   `struct headword_node`: Multihash node containing headword as key; collocate subtable, collocate-node pointer subtable, and count as values; and deleted node flag  
*   `struct collocate_node`: Subtable node containing collocate as key, count as value, and deleted node flag
*   `struct head_table`: Multihash table containing array of headword nodes, hash function setting, occupied count, and capacity     
*   `struct collocate_table`: Subtable containing array of collocate nodes, hash function setting, occupied count, and capacity

### Public Member Functions ###

*   `LM()`: Constructs bigram language model object
*   `LM(int n)`: Constructs language model object with specified n-gram size (e.g., 2-gram, 3-gram, etc.)
*   `bool Train(string filename)`: Trains LM with an input plain-text file, parsing the text and inserting n-grams in multihash table and subtables; replaces any previous existing model
*   `map<string, int> NGrams()`: Returns a map of all n-grams and their respective counts
*   `vector<string> Collocates(string headword, unsigned int x)`: Returns a vector of the _x_ most frequent collocates for a given headword
*   `float Frequency(string ngram)`: Returns the frequency of a specified n-gram; if a unigram is entered for a bigram or higher model, headword frequency is returned
*   `map<string, int> CollocateCounts(string headword)`: Returns a map of collocates and their counts for a given headword
*   `int UniqueUnigramCount()`: Returns the total number of unique words (i.e., unigrams) 
*   `int UniqueNGramsCount()`: Returns the total number of unique n-grams
*   `int NGramsTotal()`: Returns the total number of n-grams processed (including duplicates)
*   `int TotalTokens()`: Returns total number of tokens (i.e., unigrams/headwords) processed
*   `bool CSV(string filename)`: Generates a CSV file of all n-grams and counts; returns false if failed
*   `bool Remove(string ngram)`: Removes a specified n-gram
*   `bool Grow(string filename)`: Expands model with additional input text file

### Private Member Functions ###

*   `int EstimateTokenCount(string filename)`: For the purpose of setting initial table size, estimates number of words (i.e., tokens) based on input file size
*   `unsigned int PowerOfTwoExponent(unsigned int x)`: Return exponent of smallest power of 2 greater than or equal to x
*   `unsigned int PrimeSize(unsigned int x)`: Return smallest prime greater than the smallest power of 2 greater or equal to x for x > minimum table size - 1, or return minimum table size otherwise (see Sedgewick, 1998)
*   `shared_ptr<head_table> InitHeadTable(int capacity)`: Initializes headword multihash table 
*   `shared_ptr<collocate_table> InitCollocateTable(int capacity)`: Initializes collocate hash subtable
*   `shared_ptr<headword_node> InitHeadword(string headword)`: Initializes headword node 
*   `shared_ptr<collocate_node> InitCollocate(string collocate)`: Initializes collocate node
*   `int InsertNGram(string headword, string collocate)`: Inserts headword or increments count if already exists; returns bucket index
*   `int InsertCollocate(shared_ptr<headword_node> headword, string collocate)`: Inserts collocate into headword’s subtable or increments count if already exists; returns bucket index
*   `shared_ptr<headword_node> FindHeadword(string headword)`: Find headword node
*   `shared_ptr<collocate_node> FindCollocate(shared_ptr<headword_node> headword, string collocate)`: Find collocate node
*   `void Reset()`: Deletes existing model and resets counts
*   `bool DeleteHeadword(shared_ptr<headword_node> headword)`: Deletes headword by setting deleted node flag to true and corrects counts
*   `bool DeleteCollocate(shared_ptr<headword_node> headword, shared_ptr<collocate_node> collocate)`: Deletes collocate (and headword if appropriate) by setting deleted node flag(s) to true and corrects counts
*   `float GetHeadTableLoad(shared_ptr<head_table> table)`: Returns head table load
*   `float GetCollocateTableLoad(shared_ptr<collocate_table> table)`: Returns collocate table load 
*   `void ResizeHeadTable(shared_ptr<head_table> table)`: Resizes head table
*   `void ResizeCollocateTable(shared_ptr<headword_node> headword, shared_ptr<collocate_table> table)`: Resizes collocate table
*   `int InsertHeadNode(shared_ptr<head_table> table, shared_ptr<headword_node> node)`: Insert existing headword node into new headword table
*   `int InsertCollocateNode(shared_ptr<collocate_table> table, shared_ptr<collocate_node> node)`: Insert existing collocate node into new subtable 
*   `void SortCounts()`: Sorts all frequency subtables after building or expanding LM
*   `vector<string> SplitNGram(string ngram)`: Separates n-gram string into headword and collocate

## Unit Testing ##

Unit tests using Google's C++ testing framework were performed to ensure that the implemented class performed as expected. The following tests were implemented and passed:

*   `InitLM`: Both the default and parameterized constructors initialize data members as expected
*   `InitCollocate`: Initializes collocate node's data members as expected
*   `PrimeSize`: Returns expected prime numbers for several inputs
*   `InitCollocateTable`: Initializes collocate hash subtable as expected
*   `HashFunction`: Collocate subtable hash function assigns index as expected
*   `InitHeadword`: Initializes headword node's data members as expected
*   `InitHeadTable`: Initializes headword hash table node's data members as expected
*   `InsertNGram`: Inserts several bigrams in head table and collocate and frequency subtables as expected
*   `EstimateTokenCount`: Within 5% error margin, correctly estimates number of tokens based on text file's size
*   `Train`: Training bigram model with empty, small, and large text files results in expected bigram and token counts (implicitly tests that table sizes are automatically increased before number of entries exceeds capacity)
*   `UniqueUnigramCount`: Returns expected number of unique unigrams for a given text
*   `UniqueNGramsCount`: Returns expected number of unique trigrams for a given text
*   `UniqueNGramsCount`: Returns expected number of unique trigrams for a given text
*   `NGramsTotal`: Returns expected number of trigrams for a given text
*   `TotalTokens`: Returns expected number of tokens for a given text in a 4-gram language model
*   `FindHeadword`: Locates expected node 
*   `FindCollocate`: Locates expected node 
*   `NGrams`: Several bigrams and their respective counts match expected values for a given text
*   `Collocates`: Specified number of most frequent collocates for a particular headword are returned for a given text
*   `Frequency`: Several bigrams and their respective frequencies match expected values for a given text
*   `CollocateCounts`: Several collocates and their respective counts match expected values for a particular headword in a given text
*   `CSV`: A CSV file is output with expected number of elements
*   `Remove`: Unigram, bigram, and token counts are as expected after several bigrams are removed; improperly input strings do not affect counts; when last element in a table is removed, the entire table is reset; when the load for a table drops below specified threshold, the table size is reduced
*   `Grow`: Expanding bigram language model with both the same text file and a different text file results in expected token and bigram counts and changes in frequency; bigrams unique to each text are present in model

## Future Improvements ##

Future improvements of the n-gram language model class implemented here could include refinements of parsing capabilities, including double-width characters and parsing punctuation as tokens. Including start and end of sentence tokens would also make it possible to implement automatic generation of text similar to the corpus a model is trained with.

## References ##

Estébanez, C., Saez, Y., Recio, G., & Isasi, P. (2014). Performance of the most common non‐cryptographic hash functions. _Software: Practice and Experience, 44_(6), 681-698. [https://doi.org/10.1002/spe.2179](https://doi.org/10.1002/spe.2179)

Fowler, G., Noll, L. C., Vo, K.-P., Eastlake, D., & Hansen, T. (2019, May 29). _The FNV non-cryptographic hash algorithm._ Internet Engineering Task Force. [https://datatracker.ietf.org/doc/html/draft-eastlake-fnv-17.html](https://datatracker.ietf.org/doc/html/draft-eastlake-fnv-17.html)

Google. (n.d.). GoogleTest. [https://google.github.io/googletest](https://google.github.io/googletest/)

Gregoire, M. (2018). _Professional C++_ (4th ed.). Wiley.

Henke, C., Schmoll, C., & Zseby, T. (2008). Empirical evaluation of hash functions for multipoint measurements. _ACM SIGCOMM Computer Communication Review, 38_(3), 39-50. [https://doi.org/10.1145/1384609.1384614](https://doi.org/10.1145/1384609.1384614)

Jurafsky, D., & Martin, J. H. (2008).  _Speech and language processing: An introduction to speech recognition, computational linguistics and natural language processing_ (2nd ed.). Prentice Hall.

Karasek, J., Burget, R., & Morský, O. (2011, August). Towards an automatic design of non-cryptographic hash function. In _2011 34th International Conference on Telecommunications and Signal Processing (TSP)_ (pp. 19-23). IEEE. [https://doi.org/10.1109/TSP.2011.6043785](https://doi.org/10.1109/TSP.2011.6043785)

Leiserson, C. E. (2005). Introduction to algorithms: Lecture 7: Hashing, hash functions \[Video lecture\]. MIT Open Courseware. [https://ocw.mit.edu/courses/electrical-engineering-and-computer-science/6-046j-introduction-to-algorithms-sma-5503-fall-2005/video-lectures/lecture-7-hashing-hash-functions](https://ocw.mit.edu/courses/electrical-engineering-and-computer-science/6-046j-introduction-to-algorithms-sma-5503-fall-2005/video-lectures/lecture-7-hashing-hash-functions)

Multimap. (2022, March 11). In _Wikipedia_. [https://en.wikipedia.org/w/index.php?title=Multimap&oldid=1076479596](https://en.wikipedia.org/w/index.php?title=Multimap&oldid=1076479596)

Project Gutenberg. (n.d.) [https://www.gutenberg.org/](https://www.gutenberg.org)

Sedgewick, R. (1998). _Algorithms in C++: Parts 1-4: Fundamentals, data structures, sorting, searching_ (3rd ed.). Addison-Wesley.

van den Bosch, N. (2015). _A comparison of of hashing algorithms_ [Bachelor's thesis]. Leiden University. [https://theses.liacs.nl/pdf/2014-2015NickvandenBosch.pdf](https://theses.liacs.nl/pdf/2014-2015NickvandenBosch.pdf)
