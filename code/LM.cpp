#include "LM.h"

// FNV-1a Non-Cryptographic Hash Function
// Fowler, G., Noll, L. C., Vo, K.-P., Eastlake, D., & Hansen, T. (2019, May 29). 
// The FNV non-cryptographic hash algorithm. Internet Engineering Task Force.
// https://datatracker.ietf.org/doc/html/draft-eastlake-fnv-17.html
// http://isthe.com/chongo/tech/comp/fnv/
unsigned int FNV1a(string key, unsigned int capacity){
    // 32-bit FNV Prime Coefficient = 2**24 + 2**8 + 0x93 = 16,777,619
    const unsigned int FNV_prime = 0x01000193;

    // Initialize hash to 32-bit offset basis = 2,166,136,261
    unsigned int hash = 0x811C9DC5;

    // For each character in string
    for (unsigned int i = 0; i < key.length(); i++){
        // XOR hash by character
        hash ^= key[i];
        // Multiply hash by FNV prime
        hash *= FNV_prime;
    }
    // Reduce hash modulo capacity to get bin index   
    return hash % capacity;
}

// Count comparison function for collocate nodes
bool CompareCNodes(shared_ptr<collocate_node> a, shared_ptr<collocate_node> b){
    return (a->count < b->count);
}

// Overload < operator for collocate nodes
bool operator<(shared_ptr<collocate_node> a, shared_ptr<collocate_node> b){
    return (a->count - b->count) < 0;
}

// Public Member Functions
// Constructs 2-gram language model object by default
LM::LM(){
    // Initialize n-gram size, unique n-grams count, total n-grams, and tokens data members
    ngram_size = 2; 
    unique_ngrams_count = 0;
    ngrams_total = 0;
    tokens = 0;
    // Initialize main headword table pointer to null
    main_table = shared_ptr<head_table>(NULL);
}

// Constructs language model object with specified n-gram size (e.g., 2-gram, 3-gram, etc.)
LM::LM(int n){
    // Initialize n-gram size, unique n-grams count, total n-grams, and tokens data members
    ngram_size = n;
    unique_ngrams_count = 0;
    ngrams_total = 0;
    tokens = 0;
    // Initialize main headword table pointer to null
    main_table = shared_ptr<head_table>(NULL);
}

// Deconstructor;
LM::~LM(){
    // Shared pointers used, so nothing needed here
}

// Trains LM with an input text, parsing the text 
// and inserting n-grams in multihash table and subtables
// Replaces previous model; returns false if failed
bool LM::Train(string filename){
    // Get estimate of number of tokens
    int estimate = EstimateTokenCount(filename);

    // If estimation failed, do nothing
    if (estimate < 0){
        return false;
    }

    // If LM previously trained
    if (main_table){
        // Delete data and reset counts
        Reset();
    }

    // Initialize headword table at least twice as large as estimate 
    // (Capacity will be adjusted to a larger prime number in InitHeadTable)
    main_table = InitHeadTable(estimate*2);

    // Set text file path
    string path = "../corpus/" + filename;

    // Initialize input stream
    ifstream stream;

    // Open file
    stream.open(path);

    // Check if open failed
    if(stream.fail()){
        return false;
    }
    else{
        // Initialize number of tokens to n-1 
        // because n-sized window used to step through text 
        // means that n-1 tokens will not be counted
        tokens = ngram_size - 1;

        // Initialize list of strings to temporarily hold current n-gram
        list<string> ngram;
        
        // Step through text one word at a time, skipping white space (space, tab, newline)
        string word;             // holds current word
        unsigned int count = 0;  // count of words processed
        while(stream >> word){
            // Increment count
            count++;

            // Remove punctuation (except hyphen)
            // Iterate over word
            for (unsigned int i = 0; i < word.size(); i++){
                // If character is punctuation and not hyphen
                if (ispunct(word.at(i)) && word.at(i) != '-'){
                    // Remove it
                    word.erase(i, 1);
                    // Decrement iterator (so no characters are skipped)
                    i--;
                }
            }

            // Add word to end of list
            ngram.push_back(word);

            // If count is at least ngram size
            if (count >= ngram_size){
                // Get first word of ngram 
                string headword = ngram.front();
                // Remove first word in ngram list
                ngram.pop_front();
                // Iterate through remaining word(s) in list and set as collocate
                string collocate; 
                for (auto i = ngram.cbegin(); i != ngram.cend(); i++){
                    // Append word at front of list to collocate adding space between words
                    if (collocate.size() == 0){
                        collocate = *i;
                    }
                    else {
                        collocate += " " + *i;
                    }
                }

                // If load exceeds 1/2, double table size
                if (GetHeadTableLoad(main_table) > 0.5){
                    ResizeHeadTable(main_table);
                }

                // Insert ngram into multihash table
                int status = InsertNGram(headword, collocate);

                // Check if insert failed
                if (status < 0){
                    // Failed
                    // Close file
                    stream.close();
                    return false;
                }
            }
        }
        // Close file
        stream.close();

        // If not unigram LM
        if (ngram_size > 1){
            // Sort the collocate frequency subtables
            SortCounts();
        }
        return true;
    }
    return false;
}

// Returns a map of all n-grams and their respective counts
map<string, int> LM::NGrams(){
    // Initialize map object
    map<string, int> ngram_map;
    // Confirm that headword table exists
    if(main_table){
        // If unigram LM
        if (ngram_size == 1){
            // For each headword in table
            for (unsigned int i = 0; i < main_table->table->size(); i++){
                // If node is not empty and not deleted
                if (main_table->table->at(i) && !main_table->table->at(i)->deleted){
                    // Add headword and count to ngram map    
                    ngram_map.insert({main_table->table->at(i)->headword, main_table->table->at(i)->count});
                }
            }
        }
        else {
            // Initialize temporary ngram string variable
            string ngram;
            // For each headword in table
            for (unsigned int i = 0; i < main_table->table->size(); i++){
                // If node is not empty and not deleted
                if (main_table->table->at(i) && !main_table->table->at(i)->deleted){    
                    // For each element in collocate frequencies subtable
                    for (unsigned int j = 0; j < main_table->table->at(i)->frequencies->size(); j++){
                        // Update ngram to current headword + space + current collocate
                        ngram = main_table->table->at(i)->headword + " " + main_table->table->at(i)->frequencies->at(j)->collocate;
                        // Add ngram and count to ngram map
                        ngram_map.insert({ngram, main_table->table->at(i)->frequencies->at(j)->count});
                    }
                }
            }
        }        
    }
    return ngram_map;
}

// Returns a vector of the x most frequent collocates for a given headword
vector<string> LM::Collocates(string headword, unsigned int x){
    // Initialize output vector
    vector<string> collocates;
    // Confirm head table exists and not unigram LM
    if (main_table && (ngram_size > 1)){
        // Look for headword
        shared_ptr<headword_node> headword_node = FindHeadword(headword);
        // If found
        if (headword_node){
            // Get the smaller of x and collocate frequencies subtable size
            unsigned int bound = min(x, (unsigned int)(headword_node->frequencies->size()));
            // For each element in collocate frequencies subtable
            for (unsigned int i = 0; i < bound; i++){
                // Add collocate to output vector
                collocates.push_back(headword_node->frequencies->at(i)->collocate);
            }
        }
    }
    return collocates;
}

// Returns the frequency of a specified n-gram;
// if unigram entered for bigram or higher model, headword frequency returned
float LM::Frequency(string ngram){
    // Initialize frequency
    float frequency = 0.0;
    // Confirm head table exists and n-gram is not empty
    if (main_table && (ngram.size() > 0)){
        // Extract headword and collocate from n-gram string
        vector<string> grams = SplitNGram(ngram);
        // Initialize headword
        string headword = grams.at(0);
        // Initialize collocate
        string collocate;
        // If bigram or larger
        if (grams.size() == 2){
            // Update collocate
            collocate = grams.at(1);
        }
        // Look for headword
        shared_ptr<headword_node> headword_node = FindHeadword(headword);
        // If found
        if (headword_node){
            // If unigram LM
            if (ngram_size == 1){
                // Get count and divide by total number of n-grams processed (including duplicates)
                frequency = (float)headword_node->count / ngrams_total;
            }
            // If no collocate included in n-gram parameter, but bigram or larger LM
            else if (grams.size() == 1){
                // Get count and divide by total number of tokens (i.e., unigrams/headwords) processed
                frequency = (float)headword_node->count / tokens;
            }
            else {
                // Look for collocate
                shared_ptr<collocate_node> collocate_node = FindCollocate(headword_node, collocate);
                // If found
                if (collocate_node){
                    // Get count and divide by total number of n-grams processed (including duplicates)
                    frequency = (float)collocate_node->count / ngrams_total;
                }               
            }
        }
    }
    return frequency;
}

// Returns a map of collocates and their counts for a given headword
map<string, int> LM::CollocateCounts(string headword){
    // Initialize map object
    map<string, int> collocate_map;
    // Confirm head table exists and not unigram LM
    if (main_table && (ngram_size > 1)){
        // Look for headword
        shared_ptr<headword_node> headword_node = FindHeadword(headword);
        // If found
        if (headword_node){
            // For each element in collocate frequencies subtable
            for (unsigned int i = 0; i < headword_node->frequencies->size(); i++){
                // Add collocate and count to output map
                collocate_map.insert({headword_node->frequencies->at(i)->collocate, headword_node->frequencies->at(i)->count});
            }
        }
    }
    return collocate_map;
}

// Returns the total number of unique words (i.e., unigrams): 
int LM::UniqueUnigramCount(){
    // Because n-sized window used to step through text,
    // there will be n-1 more unique words than headwords
    return main_table->occupied + (ngram_size - 1);
}

// Returns the total number of unique n-grams
int LM::UniqueNGramsCount(){
    return unique_ngrams_count;
}

// Returns the total number of n-grams processed (including duplicates)
int LM::NGramsTotal(){
    return ngrams_total;
}

// Returns total number of tokens (i.e., unigrams/headwords) processed
int LM::TotalTokens(){
    return tokens;
}

// Generate a CSV file of all n-grams and counts; returns false if failed
bool LM::CSV(string filename){
    // If main table not empty
    if (main_table){

        // Get map of all n-grams and counts
        map<string, int> ngrams_map = NGrams();

        // Set CSV file path
        string path = "../results/" + filename + ".csv";

        // Initialize output stream
        ofstream outfile(path, ios::trunc);

        // Check if open failed
        if(outfile.fail()){
            return false;
        }
        else{
            // Write column headings to first line
            outfile << "ngram,count\n";

            // Iterate over n-grams map
            for (const auto& [ngram, freq] : ngrams_map) {
                // Write each n-gram and count separated by a comma on a new line
                outfile << ngram << "," << ngrams_map[ngram] << "\n";
            }

            // Close file
            outfile.close();

            return true;
        }
    }
    return false;
}

// Removes a specified n-gram
bool LM::Remove(string ngram){
    // Confirm head table exists and n-gram is not empty
    if (main_table && (ngram.size() > 0)){
        // Extract headword and collocate from n-gram string
        vector<string> grams = SplitNGram(ngram);
        // Check that n-gram dimension correponds with LM:
        // If unigram LM, but supplied n-gram is bigram or higher,
        // or if bigram or higher LM, but supplied n-gram is unigram,
        if ((ngram_size == 1 && grams.size() > 1) || (ngram_size > 1 && grams.size() == 1)){
            // Do not proceed
            return false;
        }
        else{
            // Initialize headword
            string headword = grams.at(0);
            // Initialize collocate
            string collocate;
            // If bigram or larger
            if (grams.size() == 2){
                // Update collocate
                collocate = grams.at(1);
            }
            // Look for headword
            shared_ptr<headword_node> headword_node = FindHeadword(headword);
            // If found
            if (headword_node){
                // If unigram LM
                if (ngram_size == 1){
                    return DeleteHeadword(headword_node);
                }
                else{
                    // Look for collocate
                    shared_ptr<collocate_node> collocate_node = FindCollocate(headword_node, collocate);
                    // If found
                    if (collocate_node){
                        return DeleteCollocate(headword_node, collocate_node);
                    }
                    else {
                        // No such n-gram in LM
                        return false;
                    }               
                }
            }
        }
    }
    return false;
}

// Expand model with additional input text
bool LM::Grow(string filename){
    // If model not already trained with initial data
    if (!main_table){
        // Model should be trained first
        return false;
    }

    // Get estimate of number of tokens in new file
    int estimate = EstimateTokenCount(filename);

    // If estimation failed, do nothing
    if (estimate < 0){
        return false;
    }

    // Initialize new headword table with capacity at least twice as large as estimate 
    // plus the capacity of the original table 
    // (Capacity will be adjusted to a larger prime number in InitHeadTable)
    shared_ptr<head_table> new_table = InitHeadTable(estimate*2 + main_table->capacity);

    // Transfer contents of original table into new table

    // For each bucket in original table
    for (unsigned int i = 0; i < main_table->capacity; i++){      
        // If non-empty bucket
        if (main_table->table->at(i)){
            // Insert node into new table
            InsertHeadNode(new_table, main_table->table->at(i));            
        }
    }
    // Point main_table pointer to new headword table
    main_table = new_table;

    // Set text file path
    string path = "../corpus/" + filename;

    // Initialize input stream
    ifstream stream;

    // Open file
    stream.open(path);

    // Check if open failed
    if(stream.fail()){
        return false;
    }
    else{
        // Increment tokens by n-1 because n-sized window used to step through text 
        // means that n-1 tokens will not be counted
        tokens += ngram_size - 1;

        // Initialize list of strings to temporarily hold current n-gram
        list<string> ngram;
        
        // Step through text one word at a time, skipping white space (space, tab, newline)
        string word;             // holds current word
        unsigned int count = 0;  // count of words processed
        while(stream >> word){
            // Increment count
            count++;

            // Remove punctuation (except hyphen)
            // Iterate over word
            for (unsigned int i = 0; i < word.size(); i++){
                // If character is punctuation and not hyphen
                if (ispunct(word.at(i)) && word.at(i) != '-'){
                    // Remove it
                    word.erase(i, 1);
                    // Decrement iterator (so no characters are skipped)
                    i--;
                }
            }

            // Add word to end of list
            ngram.push_back(word);

            // If count is at least ngram size
            if (count >= ngram_size){
                // Get first word of ngram 
                string headword = ngram.front();
                // Remove first word in ngram list
                ngram.pop_front();
                // Iterate through remaining word(s) in list and set as collocate
                string collocate; 
                for (auto i = ngram.cbegin(); i != ngram.cend(); i++){
                    // Append word at front of list to collocate adding space between words
                    if (collocate.size() == 0){
                        collocate = *i;
                    }
                    else {
                        collocate += " " + *i;
                    }
                }

                // If load exceeds 1/2, double table size
                if (GetHeadTableLoad(main_table) > 0.5){
                    ResizeHeadTable(main_table);
                }

                // Insert ngram into multihash table
                int status = InsertNGram(headword, collocate);

                // Check if insert failed
                if (status < 0){
                    // Failed
                    // Close file
                    stream.close();
                    return false;
                }
            }
        }
        // Close file
        stream.close();

        // If not unigram LM
        if (ngram_size > 1){
            // Sort the collocate frequency subtables
            SortCounts();
        }
        return true;
    }
    return false;
}

// Private Member Functions

// For the purpose of setting initial table size, estimates
// number of words (i.e., tokens) based on input file size 
int LM::EstimateTokenCount(string filename){
    // Reference:
    // https://www.delftstack.com/howto/cpp/get-file-size-cpp/
    // https://stackoverflow.com/questions/56827878/how-to-get-the-file-size-in-bytes-with-c17
    
    // Set text file path
    string path = "../corpus/" + filename;
    
    // Initialize error code object
    error_code ec{};

    // Get file size (requires C++17)
    auto size = std::filesystem::file_size(path, ec);
    
    // If no error
    if (ec == error_code{}){
        // Estimate number of tokens using empirically-derived
        // least-squared linear regression model:
        // unigrams = BYTES_TO_TOKENS*bytes
        return BYTES_TO_TOKENS*size;
    }
    else {
        // // Output error message
        // cout << "Error accessing file: '" << path << "' " << ec.message() << endl;
        // Failed, so return -1
        return -1;
    }
}

// Return exponent of smallest power of 2 greater than or equal to x
unsigned int LM::PowerOfTwoExponent(unsigned int x){
    // Find exponent such that 2^exponent is smallest power of 2 larger than or equal to x.

    // Initialize exponent
    unsigned int exponent = 0;
    if (x < 2){
        return exponent;
    }
    else {
        // Calculate ceiling(lg(x)) = floor(lg(x-1)) + 1, x > 1
        // by bit shifting right until no ones (in binary) remain
        // because floor(lg(x)) is the most significant bit place of x
        
        // x-1
        x--;
        // Bit shift right until no ones remain = floor(lg(x-1))
        while (x >>= 1){
            exponent++;
        }
        // Increment to get ceiling(lg(x))
        exponent++;
        return exponent;
    }
}

// Return smallest prime greater than the smallest power of 2 
// greater or equal to x for x > minimum table size - 1, or return minimum table size otherwise (see Sedgewick, 1998)
unsigned int LM::PrimeSize(unsigned int x){
    // If below minimum size
    if (x < MIN_SIZE){
        return MIN_SIZE;
    }
    // Get exponent of smallest power of 2 greater than or equal to x
    unsigned int exponent = PowerOfTwoExponent(x);
    // Return smallest prime greater than 2^exponent from constant PRIMES vector
    return PRIMES[exponent];
}

// Initializes headword multihash table 
shared_ptr<head_table> LM::InitHeadTable(int capacity){
    // Allocate new table on heap
    shared_ptr<head_table> table (new head_table);
    // Set capacity to smallest prime greater than 
    // or equal to 2^ceil(lg(capacity)), capacity > 6
    table->capacity = PrimeSize(capacity);
    // Initialize occupied count to 0
    table->occupied = 0;
    // Set hash function to FNV-1a
    table->hash_function = FNV1a;
    //Initialize table vector
    table->table = shared_ptr<hwtable>(new hwtable(table->capacity));
    return table;
}

// Initializes collocate hash subtable
shared_ptr<collocate_table> LM::InitCollocateTable(int capacity){
    // Allocate new table on heap
    shared_ptr<collocate_table> table (new collocate_table);
    // Set capacity to smallest prime greater than the smallest power of two
    // greater than or equal to minimum table whichever is smaller
    table->capacity = PrimeSize(capacity);
    // Initialize occupied count to 0
    table->occupied = 0;
    // Set hash function to FNV-1a
    table->hash_function = FNV1a;
    //Initialize table vector
    table->table = shared_ptr<ctable>(new ctable(table->capacity));
    return table;
}

// Initializes headword node 
shared_ptr<headword_node> LM::InitHeadword(string headword){
    // Allocate new node on heap
    shared_ptr<headword_node> node (new headword_node);
    // Set key
    node->headword = headword;
    // Initialize number of occurrences in corpus
    node->count = 1;
    // Initialize minimum-sized collocates subtable
    node->collocates = InitCollocateTable(MIN_SIZE - 1);
    // Initialize table vector
    node->frequencies = shared_ptr<ctable>(new ctable(0));
    // Set as undeleted
    node->deleted = false;
    return node;
}

// Initializes collocate node
shared_ptr<collocate_node> LM::InitCollocate(string collocate){
    // Allocate new node on heap
    shared_ptr<collocate_node> node (new collocate_node);
    // Set key
    node->collocate = collocate;
    // Initialize count of occurrences 
    node->count = 1;
    // Set as undeleted
    node->deleted = false;
    return node;
}

// Inserts n-gram headword and collocate or increments count if already exists 
// Returns bucket number or -1 if failed
int LM::InsertNGram(string headword, string collocate){
    // Confirm headword multihash table exists
    if (main_table){
        // Get bucket index using hash function
        unsigned int bucket = main_table->hash_function(headword, main_table->capacity);

        // Initialize probed count
        unsigned int probed = 0;

        // Until all possible buckets have been probed
        while (probed < main_table->capacity){

            // If bucket is empty or contains a previously deleted node
            if (!main_table->table->at(bucket) || main_table->table->at(bucket)->deleted){
            
                // Insert new node at current bucket index
                main_table->table->at(bucket) = InitHeadword(headword);

                // Increment occupied, total n-grams, and total tokens
                main_table->occupied++;
                ngrams_total++;
                tokens++;

                // If unigram LM
                if (ngram_size == 1){
                    // Increment unique n-grams count
                    unique_ngrams_count++;
                }
                else {
                    // Insert collocate
                    InsertCollocate(main_table->table->at(bucket), collocate);
                }

                return bucket;
            }
            else{
                // If headword node already exists
                if (main_table->table->at(bucket)->headword == headword){
                    // Increment headword count
                    main_table->table->at(bucket)->count++;
                    // Increment total n-grams and tokens processed
                    ngrams_total++;
                    tokens++;

                // If not unigram LM
                if (ngram_size > 1){
                    // Insert collocate
                    InsertCollocate(main_table->table->at(bucket), collocate);
                }

                    return bucket;
                }
                else {
                    // Try the next bucket
                    // Increment bucket index modulo capacity
                    bucket = (bucket + 1) % main_table->capacity;
                    
                    // Increment probed count
                    probed++;
                }
            }
        }
    }
    // Insert failed, so return -1
    return -1;
}

// Inserts collocate into headword’s subtable or increments count if already exists
int LM::InsertCollocate(shared_ptr<headword_node> headword, string collocate){
    // Confirm collocates subtable exists
    if (headword->collocates){
        // If collocate table load exceeds 1/2, double size
        if (GetCollocateTableLoad(headword->collocates) > 0.5){
            ResizeCollocateTable(headword, headword->collocates);
        }  

        // Get bucket index using hash function
        unsigned int bucket = headword->collocates->hash_function(collocate, headword->collocates->capacity);

        // Initialize probed count
        unsigned int probed = 0;

        // Until all possible buckets have been probed
        while (probed < headword->collocates->capacity){

            // If bucket is empty or contains a previously deleted node
            if (!headword->collocates->table->at(bucket) || headword->collocates->table->at(bucket)->deleted){

                // Insert new node at current bucket index
                headword->collocates->table->at(bucket) = InitCollocate(collocate);

                // Append collocate node pointer to frequency array
                headword->frequencies->push_back(headword->collocates->table->at(bucket));

                // Increment occupied and unique n-grams count
                headword->collocates->occupied++;
                unique_ngrams_count++;

                return bucket;
            }
            else{
                // If collocate node already exists
                if (headword->collocates->table->at(bucket)->collocate == collocate){
                    // Increment count
                    headword->collocates->table->at(bucket)->count++;

                    return bucket;
                }
                else {
                    // Try the next bucket
                    // Increment bucket index modulo capacity
                    bucket = (bucket + 1) % headword->collocates->capacity;
                    
                    // Increment probed count
                    probed++;
                }
            }
        }
    }
    // Insert failed, so return -1
    return -1;    
}

// Find and return headword node or null if not found
shared_ptr<headword_node> LM::FindHeadword(string headword){
    // Confirm headword multihash table exists
    if (main_table){
        // Get bucket index using hash function
        unsigned int bucket = main_table->hash_function(headword, main_table->capacity);

        // Initialize probed count
        unsigned int probed = 0;

        // Until all possible buckets have been probed
        while (probed < main_table->capacity){

            // If bucket is empty or contains a previously deleted node
            if (!main_table->table->at(bucket) || main_table->table->at(bucket)->deleted){
    
                // Headword not in table, so return null pointer
                return shared_ptr<headword_node>(NULL);
            }
            else{
                // If headword node matches
                if (main_table->table->at(bucket)->headword == headword){
                    // Return pointer to that node
                    return main_table->table->at(bucket);
                }
                else {
                    // Look in the next bucket
                    // Increment bucket index modulo capacity
                    bucket = (bucket + 1) % main_table->capacity;
                    
                    // Increment probed count
                    probed++;
                }
            }
        }
    }
    // Hash table empty, so return null pointer
    return shared_ptr<headword_node>(NULL);
}

// Find and return collocate node or null if not found
shared_ptr<collocate_node> LM::FindCollocate(shared_ptr<headword_node> headword, string collocate){
    // Get bucket index using hash function
    unsigned int bucket = headword->collocates->hash_function(collocate, headword->collocates->capacity);

    // Initialize probed count
    unsigned int probed = 0;

    // Until all possible buckets have been probed
    while (probed < headword->collocates->capacity){

        // If bucket is empty or contains a previously deleted node
        if (!headword->collocates->table->at(bucket) || headword->collocates->table->at(bucket)->deleted){
            // Collocate not in subtable, so return null node
            return shared_ptr<collocate_node>(NULL);
        }
        else{
            // If collocate node matches
            if (headword->collocates->table->at(bucket)->collocate == collocate){
                // Return pointer to that node
                return headword->collocates->table->at(bucket);
            }
            else {
                // Look in the next bucket
                // Increment bucket index modulo capacity
                bucket = (bucket + 1) % headword->collocates->capacity;
                
                // Increment probed count
                probed++;
            }
        }
    }
    return shared_ptr<collocate_node>(NULL);
}

// Deletes existing model and resets counts
void LM::Reset(){
    // Delete entire table and reset counts
    main_table = shared_ptr<head_table>(NULL);
    unique_ngrams_count = 0;
    ngrams_total = 0;
    tokens = 0;
}

// Deletes headword by setting deleted node flag to true and correcting counts
// Returns true if succeeded
bool LM::DeleteHeadword(shared_ptr<headword_node> headword){
    // If only headword in LM
    if (main_table->occupied == 1){
        // Delete entire table and reset counts
        Reset();
    }
    else{
        // Mark headword as deleted
        headword->deleted = true;
        // Decrement occupied and unique n-grams counts
        main_table->occupied--;
        unique_ngrams_count--;
        // Decrement tokens and total n-grams by deleted headword's count
        tokens -= headword->count;
        ngrams_total -= headword->count;
        
        // If head table load is below 1/8, halve
        if (GetHeadTableLoad(main_table) < 0.125){
            ResizeHeadTable(main_table);
        }  
    }
    return true;
}

// Deletes collocate (and headword if appropriate) by setting deleted node flag(s) to true and corrects counts
// Returns true if succeeded
bool LM::DeleteCollocate(shared_ptr<headword_node> headword, shared_ptr<collocate_node> collocate){    
    // If collocate is only remaining collocate of headword
    if (headword->collocates->occupied == 1){
        // Mark collocate as deleted
        collocate->deleted = true;
        // Decrement occupied count
        headword->collocates->occupied--;
        // Decrement tokens by deleted collocate's count
        tokens -= collocate->count;
        return DeleteHeadword(headword);
    }
    else {
        // Get index of collocate in frequencies subtable
        unsigned int i = 0;     // iterator variable
        unsigned int size = headword->frequencies->size();  // size of subtable
        int pos = -1;  // index of collocate in subtable
        while ((pos < 0 && i < size)){
            // If collocate node matches ith element in subtable
            if (headword->frequencies->at(i) == collocate){
                pos = i;
            }
            // Increment i
            i++;
        }
        // If found
        if (pos >= 0){
            // Remove collocate from frequencies subtable
            headword->frequencies->erase(headword->frequencies->begin() + pos);
            // Mark collocate as deleted
            collocate->deleted = true;
            // Decrement occupied and unique n-grams counts
            headword->collocates->occupied--;
            unique_ngrams_count--;
            // Decrement tokens by twice the deleted collocate's count
            // (for both the collocate and each time the headword appeared with the collocate)
            tokens -= collocate->count * 2;
            // Decrement total n-grams by the deleted collocate's count
            ngrams_total -= collocate->count;

            // If collocate table load is below 1/8, halve
            if (GetCollocateTableLoad(headword->collocates) < 0.125){
                ResizeCollocateTable(headword, headword->collocates);
            }
            return true;
        }
        else {
            // Collocate unexpectedly not found in frequencies subtable
            return false;
        }
    }
}

// Returns head table load
float LM::GetHeadTableLoad(shared_ptr<head_table> table){
    // If non-null table
    if (table){
        // Load = undeleted nodes / capacity
        float load = (float)table->occupied / table->capacity;
        return load;
    }
    return 0.0;
}

// Returns collocate table load 
float LM::GetCollocateTableLoad(shared_ptr<collocate_table> table){
    // If non-null table
    if (table){
        // Load = undeleted nodes / capacity
        float load = (float)table->occupied / table->capacity;
        return load;
    }
    return 0.0;
}

// Resizes head table
void LM::ResizeHeadTable(shared_ptr<head_table> table){
    // Get load
    float load = GetHeadTableLoad(table);
    // Initialize capacity
    unsigned int capacity = table->capacity;
    // If load is < 1/8 and capacity is not at minimum size
    if (load < 0.125 && capacity > MIN_SIZE){
        // Set capacity to power of two which is two steps below current capacity
        // When InitHeadTable is called, a table with a prime number 
        // capacity approximately half of current size will be created
        capacity = pow(2, PowerOfTwoExponent(capacity)-2);
    }
    // If load is < 1/2
    else if (load < 0.5){
        // Table does not need to be resized
        return;
    }
    else{
        // Table needs to be doubled
        // By leaving capacity as is, when InitHeadTable is called, a table with a prime number 
        // capacity approximately double current capacity will be created
    }
    // Initialize new table with new capacity (automatically adjusted in InitHeadTable)
    shared_ptr<head_table> new_table = InitHeadTable(capacity);
        
    // For each bucket in original table
    for (unsigned int i = 0; i < table->capacity; i++){      
        // If non-empty bucket
        if (table->table->at(i)){
            // Insert node into new table
            InsertHeadNode(new_table, table->table->at(i));            
        }
    }
    // Point main_table pointer to new headword table
    main_table = new_table;
}

// Resizes collocate table 
void LM::ResizeCollocateTable(shared_ptr<headword_node> headword, shared_ptr<collocate_table> table){
    // Get load
    float load = GetCollocateTableLoad(table);
    // Initialize capacity
    unsigned int capacity = table->capacity;
    // If load is < 1/8 and capacity is not at minimum size
    if (load < 0.125 && capacity > MIN_SIZE){
        // Set capacity to power of two which is two steps below current capacity
        // When InitCollocateTable is called, a table with a prime number 
        // capacity approximately half of current size will be created
        capacity = pow(2, PowerOfTwoExponent(capacity)-2);
    }
    // If load is < 1/2
    else if (load < 0.5){
        // Table does not need to be resized
        return;
    }
    else{
        // Table needs to be doubled
        // By leaving capacity as is, when InitCollocateTable is called, a table with a prime number 
        // capacity approximately double current capacity will be created
    }
    // Initialize new table with new capacity (automatically adjusted in InitCollocateTable)
    shared_ptr<collocate_table> new_table = InitCollocateTable(capacity);
 
    // For each bucket in original table
    for (unsigned int i = 0; i < table->capacity; i++){      
        // If non-empty bucket
        if (table->table->at(i)){
            // Insert node into new table
            InsertCollocateNode(new_table, table->table->at(i));            
        }
    }
    // Point headword to new collocates subtable
    headword->collocates = new_table;
}

// Insert existing headword node into new headword table
int LM::InsertHeadNode(shared_ptr<head_table> table, shared_ptr<headword_node> node){
    // Get bucket index using hash function
    unsigned int bucket = table->hash_function(node->headword, table->capacity);

    // Initialize probed count
    unsigned int probed = 0;

    // Until all possible buckets have been probed
    while (probed < table->capacity){

        // If bucket is empty or contains a previously deleted node
        if (!table->table->at(bucket) || table->table->at(bucket)->deleted){

            // Insert new node at current bucket index
            table->table->at(bucket) = node;

            // If node being inserted is not deleted
            if (!node->deleted){
                // Increment occupied and ngrams count
                table->occupied++;
            }
            return bucket;
        }
        else{
            // Try the next bucket
            // Increment bucket index modulo capacity
            bucket = (bucket + 1) % table->capacity;
            
            // Increment probed count
            probed++;
        }
    }
    // Insert failed, so return -1
    return -1;    
 }

// Insert existing collocate node into new subtable
int LM::InsertCollocateNode(shared_ptr<collocate_table> table, shared_ptr<collocate_node> node){
    // Get bucket index using hash function
    unsigned int bucket = table->hash_function(node->collocate, table->capacity);

    // Initialize probed count
    unsigned int probed = 0;

    // Until all possible buckets have been probed
    while (probed < table->capacity){

        // If bucket is empty or contains a previously deleted node
        if (!table->table->at(bucket) || table->table->at(bucket)->deleted){

            // Insert new node at current bucket index
            table->table->at(bucket) = node;

            // If node being inserted is not deleted
            if (!node->deleted){
                // Increment occupied and ngrams count
                table->occupied++;
            }
            return bucket;
        }
        else{
            // Try the next bucket
            // Increment bucket index modulo capacity
            bucket = (bucket + 1) % table->capacity;
            
            // Increment probed count
            probed++;
        }
    }
    // Insert failed, so return -1
    return -1;    
 }


// Sorts all frequencies subtables after building or expanding LM
void LM::SortCounts(){
    // Confirm that headword table exists
    if(main_table){
        // For each headword in table
        for (unsigned int i = 0; i < main_table->table->size(); i++){
            // If node is not empty and not deleted
            if (main_table->table->at(i) && !(main_table->table->at(i)->deleted)){
                // Sort frequency subtable in descending order by count
                sort(main_table->table->at(i)->frequencies->rbegin(), main_table->table->at(i)->frequencies->rend(), CompareCNodes);
            }
        }
    }    
}

// Separates n-gram string into headword and collocate returned vector
vector<string> LM::SplitNGram(string ngram){
    // Initialize output vector
    vector<string> output;
    // If n-gram not empty
    if (ngram.size() > 0){
        // Initialize words and output vectors
        vector<string> words;
        // Initialize temporary variable to hold current word
        string word;
        // Initialize string stream with ngram
        stringstream stream(ngram);
        // Step through n-gram stream one word at a time, skipping white space
        while (stream >> word){
            // Append word to words vector
            words.push_back(word);
        }
        // Insert headword into output vector
        output.push_back(words.at(0));
        // If not unigram
        if (words.size() > 1){
            // Initialize collocate string with second word
            string collocate = words.at(1);
            // If words remain
            if (words.size() > 2){
                // Iterate through remaining word(s)
                for (unsigned int i = 2; i < words.size(); i++){
                    // Append word to collocate adding space between words
                    collocate += " " + words.at(i);
                }
            }
            // Append collocate to output vector
            output.push_back(collocate);
        }
    }
    return output;
}
