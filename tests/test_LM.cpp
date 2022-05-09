#include <gtest/gtest.h>  // https://google.github.io/googletest/
#include <iostream>

// Access private members for unit testing
#define private public
#include "../code/LM.h"

using namespace std;

class test_LM : public ::testing::Test {
protected:
	// This function runs only once before any TEST_F function
	static void SetUpTestCase(){
	}

	// This function runs after all TEST_F functions have been executed
	static void TearDownTestCase(){
	}
    
	// this function runs before every TEST_F function
	void SetUp() override {
    }

	// this function runs after every TEST_F function
	void TearDown() override {
	}
};

// Vectors of 12 headwords and 12 collocates for testing
vector<string> test_headwords = {"serendipitous", "happy", "unfortunate", "sheer", "lucky", "grave", "great", "troubling", "fortuitous", "tragic", "extraordinary", "unexpected"};
vector<string> test_collocates = {"discovery", "coincidence", "event", "chance", "break", "circumstances", "importance", "sign", "occasion", "end", "achievement", "twist"};

// Acceptable error in frequencies
const float EPSILON = 1e-8; // 0.00000001

// Unit test LM class constructor
TEST_F(test_LM, InitLM){

	// Initialize bigram LM object with
	// default constructor (i.e., no parameters)
	LM testlm;

	// Confirm n-gram size set to default of 2
	EXPECT_EQ(testlm.ngram_size, 2);

	// Confirm unique n-grams count is zero
	EXPECT_EQ(testlm.unique_ngrams_count, 0);

	// Confirm total n-grams is zero
	EXPECT_EQ(testlm.ngrams_total, 0);

	// Confirm total n-grams is zero
	EXPECT_EQ(testlm.tokens, 0);

	// Confirm main headword hash table pointer is null
	EXPECT_FALSE(testlm.main_table);

	// Initialize trigram language model object
	LM testlm3(3);

	// Confirm n-gram size set to default of 3
	EXPECT_EQ(testlm3.ngram_size, 3);

	// Confirm n-grams count is zero
	EXPECT_EQ(testlm3.unique_ngrams_count, 0);

	// Confirm main headword hash table pointer is null
	EXPECT_FALSE(testlm3.main_table);

	// Confirm total n-grams is zero
	EXPECT_EQ(testlm3.ngrams_total, 0);

	// Confirm total n-grams is zero
	EXPECT_EQ(testlm3.tokens, 0);
}


// Unit test collocate (subtable) node intialization
TEST_F(test_LM, InitCollocate){
    // Initialize bigram language model object
	LM testlm(2);

	// Initialize collocate (subtable) node
	shared_ptr<collocate_node> node = testlm.InitCollocate("serendipitous");

	// Confirm that node was created
	ASSERT_TRUE(node);

	// Confirm that value matches
    EXPECT_EQ(node->collocate, "serendipitous");
	
	// Confirm that count is one
	EXPECT_EQ(node->count, 1);

	// Confirm that deleted tag is set to false
	EXPECT_EQ(node->deleted, false);
}

TEST_F(test_LM, PrimeSize){
    // Initialize bigram language model object
	LM testlm(2);

	unsigned int size = testlm.PrimeSize(4);
	
	// Confirm that size is 7
	EXPECT_EQ(size, 7);

	// Get smallest prime greater than or equal to 2^ceil(lg(8))
	size = testlm.PrimeSize(8);

	// Confirm that size is 11
	EXPECT_EQ(size, 11);

	// Get smallest prime greater than or equal to 2^ceil(lg(2^12)) = 4096
	size = testlm.PrimeSize(pow(2,12));

	// Confirm that size is 4099
	EXPECT_EQ(size, 4099);

	// Get smallest prime greater than or equal to 2^ceil(lg(2^12-1)) = 4096
	size = testlm.PrimeSize(pow(2,12)-1);

	// Confirm that size is 4099
	EXPECT_EQ(size, 4099);

	// Get smallest prime greater than or equal to 2^ceil(lg(2^12+5)) = 8192
	size = testlm.PrimeSize(pow(2,12)+5);

	// Confirm that size is 8209
	EXPECT_EQ(size, 8209);
}


// Unit test collocate hash subtable
TEST_F(test_LM, InitCollocateTable){
    // Initialize bigram language model object
	LM testlm(2);

	// Initialize collocate hash subtable with minimum number of buckets
	// passing parameter less than minimum size
	shared_ptr<collocate_table> table = testlm.InitCollocateTable(testlm.MIN_SIZE - 4);

	// Confirm that table was created
	ASSERT_TRUE(table);

	// Confirm number of buckets matches minimum size
	EXPECT_EQ(table->capacity, testlm.MIN_SIZE);

	// Confirm none are occupied
	EXPECT_EQ(table->occupied, 0);
	
	// Confirm table vector created
	ASSERT_TRUE(table->table);

	// Confirm every vector element empty
	for (unsigned int i=0; i < table->capacity; i++) {
    	EXPECT_FALSE(table->table->at(i));
	}
}

TEST_F(test_LM, HashFunction){
    // Initialize bigram language model object
	LM testlm(2);

	// Initialize collocate hash subtable with minimum number buckets
	shared_ptr<collocate_table> table = testlm.InitCollocateTable(testlm.MIN_SIZE);

	// Confirm that table was created
	ASSERT_TRUE(table);
	
	// Confirm bucket number for "serendipitous" is 4167377003 (mod min. size)
	EXPECT_EQ(table->hash_function("serendipitous", testlm.MIN_SIZE), 4167377003 % testlm.MIN_SIZE);
}

// Unit test headword node intialization
TEST_F(test_LM, InitHeadword){
    // Initialize bigram language model object
	LM testlm(2);

	// Initialize collocate (subtable) node
	shared_ptr<headword_node> node = testlm.InitHeadword("serendipitous");

	// Confirm that node was created
	ASSERT_TRUE(node);

	// Confirm that value matches
    EXPECT_EQ(node->headword, "serendipitous");
	
	// Confirm that count is one
	EXPECT_EQ(node->count, 1);

	// Confirm that deleted tag is set to false
	EXPECT_EQ(node->deleted, false);

	// Confirm collocate subtable capacity is 7
	EXPECT_EQ(node->collocates->capacity, 7);

	// Confirm frequencies subtable is empty
	EXPECT_EQ(node->frequencies->size(), 0);

}

// Unit test headword multihash table
TEST_F(test_LM, InitHeadTable){
    // Initialize bigram language model object
	LM testlm(2);

	// Initialize head multihash table with 11 buckets
	// passing capacity parameter of 8
	shared_ptr<head_table> table = testlm.InitHeadTable(8);

	// Confirm that table was created
	ASSERT_TRUE(table);

	// Confirm number of buckets is 11
	// See PrimeSize function declaration for why this should be 11
	EXPECT_EQ(table->capacity, 11);

	// Confirm none are occupied
	EXPECT_EQ(table->occupied, 0);
	
	// Confirm table vector created
	ASSERT_TRUE(table->table);

	// Confirm every vector element empty
	for (unsigned int i=0; i < table->capacity; i++) {
    	EXPECT_FALSE(table->table->at(i));
	}
}

TEST_F(test_LM, InsertNGram){
	// Note: InsertNGram dependent on InsertCollocate function

    // Initialize bigram language model object
	LM testlm(2);

	// Set capacity for head table
	const int CAPACITY = 11;

	// Initialize head multihash table with CAPACITY buckets
	testlm.main_table = testlm.InitHeadTable(CAPACITY);

	// Insert bigram "serendipitous discovery" and get bucket index
	int bucket = testlm.InsertNGram("serendipitous", "discovery");

	// Confirm bucket index is 0 or larger
	ASSERT_GE(bucket, 0);

	// Confirm total n-gram count is 1
	EXPECT_EQ(testlm.unique_ngrams_count, 1);

	// Confirm number of occupied buckets in main table is 1
	EXPECT_EQ(testlm.main_table->occupied, 1);

	// Confirm that headword in node at bucket index is "serendipitous"
	EXPECT_EQ(testlm.main_table->table->at(bucket)->headword, "serendipitous");

	// Confirm count in headword node is 1
	EXPECT_EQ(testlm.main_table->table->at(bucket)->count, 1);

	// Confirm frequencies subtable size is 1
	EXPECT_EQ(testlm.main_table->table->at(bucket)->frequencies->size(), 1);

	// Calculate collocate bucket index for "discovery"
	shared_ptr<collocate_table> serendipitous_collocates = testlm.main_table->table->at(bucket)->collocates;
	int discovery_bucket = serendipitous_collocates->hash_function("discovery", serendipitous_collocates->capacity);

	// Confirm collocate bucket index for "discovery" is 0 or larger
	ASSERT_GE(discovery_bucket, 0);

	// Confirm that collocate in node at collocate bucket index is "discovery"
	EXPECT_EQ(serendipitous_collocates->table->at(discovery_bucket)->collocate, "discovery");

	// Confirm count of collocate is 1
	EXPECT_EQ(serendipitous_collocates->table->at(discovery_bucket)->count, 1);

	// Confirm that first element of frequencies subtable points to same "discovery" node
	EXPECT_EQ(testlm.main_table->table->at(bucket)->frequencies->at(0), serendipitous_collocates->table->at(discovery_bucket));

	// Insert "serendipitous discovery" again
	testlm.InsertNGram("serendipitous", "discovery");

	// Confirm number of occupied buckets in main table is still 1
	EXPECT_EQ(testlm.main_table->occupied, 1);

	// Confirm count of headword is now 2
	EXPECT_EQ(testlm.main_table->table->at(bucket)->count, 2);

	// Confirm count of collocate is now 2
	EXPECT_EQ(serendipitous_collocates->table->at(discovery_bucket)->count, 2);

	// Insert "serendipitous moment"
	testlm.InsertNGram("serendipitous", "moment");

	// Confirm total n-gram count is now 2
	EXPECT_EQ(testlm.unique_ngrams_count, 2);

	// Confirm count of headword is now 3
	EXPECT_EQ(testlm.main_table->table->at(bucket)->count, 3);

	// Confirm count of collocate "discovery" is still 2
	EXPECT_EQ(serendipitous_collocates->table->at(discovery_bucket)->count, 2);

	// Add 4 other n-grams with different headwords
	for (unsigned int i=1; i<5; i++){
		testlm.InsertNGram(test_headwords.at(i), test_collocates.at(i));
	}

	// Confirm total n-gram count is now 6
	EXPECT_EQ(testlm.unique_ngrams_count, 6);

	// Confirm number of occupied buckets in main table is now 5
	EXPECT_EQ(testlm.main_table->occupied, 5);

}

TEST_F(test_LM, EstimateTokenCount){
    // Initialize bigram language model object
	LM testlm(2);

	// Estimate number of unigrams for GettysburgAddress.txt file
	int estimate = testlm.EstimateTokenCount("GettysburgAddress.txt");

	// Actual tokens count (including duplicates)
	const int TOKENS = 268;

	// Acceptable percent error threshold
	const float ALPHA = 0.05;

	// Confirm estimated count is within acceptable error threshold
	// rounded to nearest integer
	EXPECT_GE(estimate, floor(TOKENS - TOKENS * ALPHA/2 + 0.5));
	EXPECT_LE(estimate, floor(TOKENS + TOKENS * ALPHA/2 + 0.5));
}

TEST_F(test_LM, Train){
    // Initialize bigram language model object
	LM testlm(2);

	// Confirm that train model returns false when non-existent text file
	// passed as parameter
	ASSERT_FALSE(testlm.Train("DoesNotExist.txt"));

	// Confirm that number of tokens in model is zero
	EXPECT_EQ(testlm.TotalTokens(), 0);

	// Confirm that train model returns true when trained 
	// with valid text file containing exactly one bigram
	ASSERT_TRUE(testlm.Train("OneBigram.txt"));

	// Confirm that unique bigram count is one
	EXPECT_EQ(testlm.UniqueNGramsCount(), 1);

	// Confirm that total bigram count is one
	EXPECT_EQ(testlm.NGramsTotal(), 1);

	// Confirm that total number of tokens in model is 2
	EXPECT_EQ(testlm.TotalTokens(), 2);

	// Confirm that total number of unique unigrams in model is 2
	EXPECT_EQ(testlm.UniqueUnigramCount(), 2);

	// Replace model with short valid text file of natural text
	// Confirm that train model returns true
	ASSERT_TRUE(testlm.Train("GettysburgAddress.txt"));

	// Actual unique bigram count based on http://guidetodatamining.com/ngramAnalyzer/
	// Note: ngramAnalyzer counts first word ("Four") as a "bigram" which results
	// in a count of 247 unique "bigrams", so this number has been corrected as 
	// 247 - 1 to obtain true bigram count
	const int COUNT = 246;
	
	// Confirm that unique bigram count matches actual
	EXPECT_EQ(testlm.UniqueNGramsCount(), COUNT);

	// Actual total bigrams (including duplicates) based on http://guidetodatamining.com/ngramAnalyzer/
	// Note: ngramAnalyzer counts first word ("Four") as a "bigram" which results
	// in a total of 268 "bigrams" including duplicates, so this number has been corrected as 
	// 268 - 1 to obtain true bigram total including duplicates
	const int TOTAL = 267;

	// Confirm that n-grams total matches actual
	EXPECT_EQ(testlm.NGramsTotal(), TOTAL);

	// Actual number of tokens based on http://guidetodatamining.com/ngramAnalyzer/
	const int TOKENS = 268;

	// Confirm that total number of tokens in model matches actual
	EXPECT_EQ(testlm.TotalTokens(), TOKENS);

	// Replace model with over-50-times longer valid text file of natural text
	// Confirm that train model returns true
	ASSERT_TRUE(testlm.Train("MyAntoniaChaps1-9.txt"));

	// Actual number of tokens based on http://guidetodatamining.com/ngramAnalyzer/
	const int LONG_TOKENS = 15298;

	// Confirm that total number of tokens in model matches actual
	EXPECT_EQ(testlm.TotalTokens(), LONG_TOKENS);

	// Actual total bigrams (including duplicates) based on http://guidetodatamining.com/ngramAnalyzer/
	// Actual unique bigram count based on http://guidetodatamining.com/ngramAnalyzer/
	// Note: ngramAnalyzer counts first word as a "bigram" which results in a total
	// one larger than actual total of 15297
	const int LONG_TOTAL = 15297;


	// Confirm that n-grams total matches actual
	EXPECT_EQ(testlm.NGramsTotal(), LONG_TOTAL);

	// Actual unique bigram count based on http://guidetodatamining.com/ngramAnalyzer/
	// Note: ngramAnalyzer counts first word as a "bigram" which results in a count
	// one larger than actual total of 11020
	const int LONG_COUNT = 11020;
	
	// Confirm that unique bigram count matches actual
	EXPECT_EQ(testlm.UniqueNGramsCount(), LONG_COUNT);
}

TEST_F(test_LM, UniqueUnigramCount){
    // Initialize bigram language model object
	LM testlm(2);

	// Confirm that train model returns true with valid text file
	ASSERT_TRUE(testlm.Train("GettysburgAddress.txt"));

	// Actual unigram count based on http://guidetodatamining.com/ngramAnalyzer/
	const int COUNT = 144;

	// Confirm that unigram count is within acceptable range of error when compared to
	EXPECT_EQ(testlm.UniqueUnigramCount(), COUNT);
}

TEST_F(test_LM, UniqueNGramsCount){
    // Initialize trigram language model object
	LM testlm(3);

	// Confirm that train model returns true with valid text file
	ASSERT_TRUE(testlm.Train("GettysburgAddress.txt"));

	// Actual unique trigram count based on http://guidetodatamining.com/ngramAnalyzer/
	// Note: ngramAnalyzer counts first word ("Four") and first two words ("Four score") 
	// as "trigrams" which results in a count of 266 unique "trigrams", 
	// so this number has been corrected as 266 - 2 to obtain true unique trigram count
	const int COUNT = 264;

	// Confirm that trigram count matches actual
	EXPECT_EQ(testlm.UniqueNGramsCount(), COUNT);
}

TEST_F(test_LM, NGramsTotal){
    // Initialize trigram language model object
	LM testlm(3);

	// Confirm that train model returns true with valid text file
	ASSERT_TRUE(testlm.Train("GettysburgAddress.txt"));

	// Actual total trigrams count based on http://guidetodatamining.com/ngramAnalyzer/
	// Note: ngramAnalyzer counts first word ("Four") and first two words ("Four score") 
	// as "trigrams" which results in a count of 268, 
	// so this number has been corrected as 268 - 2 to obtain true trigram count
	const int TOTAL = 266;

	// Confirm that trigram count matches actual
	EXPECT_EQ(testlm.NGramsTotal(), TOTAL);
}

TEST_F(test_LM, TotalTokens){
    // Initialize 4-gram language model object
	LM testlm(4);

	// Confirm that train model returns true with valid text file
	ASSERT_TRUE(testlm.Train("GettysburgAddress.txt"));

	// Actual tokens count (including duplicates)
	const int TOKENS = 268;

	// Confirm that trigram count matches actual
	EXPECT_EQ(testlm.TotalTokens(), TOKENS);
}

TEST_F(test_LM, FindHeadword){
    // Initialize bigram language model object
	LM testlm(2);

	// Set capacity for head table
	const int CAPACITY = 11;

	// Initialize head multihash table with CAPACITY buckets
	testlm.main_table = testlm.InitHeadTable(CAPACITY);

	// Insert bigram "serendipitous discovery" and get bucket index
	int bucket = testlm.InsertNGram("serendipitous", "discovery");

	// Confirm bucket index is 0 or larger
	ASSERT_GE(bucket, 0);

	// Confirm that headword node at bucket index matches node returned by FindHeadword
	EXPECT_EQ(testlm.FindHeadword("serendipitous"), testlm.main_table->table->at(bucket));
}

TEST_F(test_LM, FindCollocate){
    // Initialize bigram language model object
	LM testlm(2);

	// Set capacity for head table
	const int CAPACITY = 11;

	// Initialize head multihash table with CAPACITY buckets
	testlm.main_table = testlm.InitHeadTable(CAPACITY);

	// Insert bigram "serendipitous discovery" and get bucket index
	int bucket = testlm.InsertNGram("serendipitous", "discovery");

	// Confirm bucket index is 0 or larger
	ASSERT_GE(bucket, 0);

	// Confirm that headward at bucket index is "serendipitous"
	ASSERT_EQ(testlm.main_table->table->at(bucket)->headword,"serendipitous");

	// Calculate collocate bucket index for "discovery"
	shared_ptr<collocate_table> serendipitous_collocates = testlm.main_table->table->at(bucket)->collocates;
	int discovery_bucket = serendipitous_collocates->hash_function("discovery", serendipitous_collocates->capacity);

	// Confirm collocate bucket index for "discovery" is 0 or larger
	ASSERT_GE(discovery_bucket, 0);

	// Confirm that collocate node at collocate bucket index matches node returned by FindCollocate
	EXPECT_EQ(testlm.FindCollocate(testlm.main_table->table->at(bucket), "discovery"), serendipitous_collocates->table->at(discovery_bucket));
}

TEST_F(test_LM, NGrams){
    // Initialize bigram language model object
	LM testlm(2);

	// Confirm that train model returns true with valid text file
	ASSERT_TRUE(testlm.Train("GettysburgAddress.txt"));

	// Get all ngrams from model
	map<string, int> output = testlm.NGrams();

	// Confirm that size of output map matches total count of ngrams
	ASSERT_EQ(output.size(), testlm.unique_ngrams_count);

	// Expected bigrams and counts for headword "to" from http://guidetodatamining.com/ngramAnalyzer/
	map<string, int> expected = {{"to the", 3},{"to be", 2},{"to that", 1},{"to add", 1},{"to dedicate", 1}};

	// Confirm expected counts for ngrams match output
    for (const auto& [ngram, count] : expected) {
		EXPECT_EQ(output[ngram], expected[ngram]);
	}
}

TEST_F(test_LM, Collocates){
    // Initialize bigram language model object
	LM testlm(2);

	// Confirm that train model returns true with valid text file
	ASSERT_TRUE(testlm.Train("GettysburgAddress.txt"));

	// Expected output for headword "to" based on http://guidetodatamining.com/ngramAnalyzer/
	// Note: Frequency of "that", "add", and "dedicate" are equal, 
	// so the last three elements could be in any order
	vector<string> expected = {"the", "be", "that", "add", "dedicate"}; 

	// Get collocates for headword "to"
	vector<string> output = testlm.Collocates("to", 3);

	// Confirm that output vector is size 3
	ASSERT_EQ(output.size(), 3);

	// Confirm that elements are expected values
	EXPECT_EQ(output.at(0), expected.at(0));
	EXPECT_EQ(output.at(1), expected.at(1));
	// Third, fourth, and fifth expected elements could be in any order, so:
	EXPECT_TRUE(output.at(2) == expected.at(2) || output.at(2) == expected.at(3) || output.at(2) == expected.at(4));

	// Attempt to get more collocates for headword "to" than actually exist
	vector<string> output2 = testlm.Collocates("to", 7);

	// Confirm that output vector size equals expected vector size
	EXPECT_EQ(output2.size(), expected.size());
}

TEST_F(test_LM, Frequency){
    // Initialize bigram language model object
	LM testlm(2);

	// Confirm that train model returns true with valid text file
	ASSERT_TRUE(testlm.Train("GettysburgAddress.txt"));

	// Actual total n-grams (including duplicates)
	const int TOTAL = 267;

	// Expected frequencies for several bigrams using counts from http://guidetodatamining.com/ngramAnalyzer/
	map<string, float> expected = {{"we cannot", 3.0/TOTAL},{"It is", 3.0/TOTAL},{"for us", 2.0/TOTAL},{"a new", 2.0/TOTAL},{"be here", 1.0/TOTAL},{"us to", 1.0/TOTAL}};

	// Confirm expected frequencies for each n-gram matches output
    for (const auto& [ngram, freq] : expected) {
		EXPECT_FLOAT_EQ(testlm.Frequency(ngram), expected[ngram]);
	}

	// Actual tokens count (including duplicates)
	const int TOKENS = 268;

	// Actual number of occurences of "that"
	const int THAT = 12;

	// Confirm frequency of "that" is 12/268
	EXPECT_FLOAT_EQ(testlm.Frequency("that"), (float)THAT/TOKENS);

}

TEST_F(test_LM, CollocateCounts){
    // Initialize bigram language model object
	LM testlm(2);

	// Confirm that train model returns true with valid text file
	ASSERT_TRUE(testlm.Train("GettysburgAddress.txt"));

	// Get all collocate counts for headword "that" from model
	map<string, int> output = testlm.CollocateCounts("that");

	// Actual number of bigram collocates for "that"
	const int COUNT = 10;

	// Confirm that size of output map matches total count of "that" collocates
	ASSERT_EQ(output.size(), COUNT);

	// Expected collocates and counts for headword "that" from http://guidetodatamining.com/ngramAnalyzer/
	map<string, int> expected = {{"this", 2},{"we", 2},{"all", 1},{"cause", 1},{"field", 1},{"from", 1},{"government", 1},{"nation", 1},{"these", 1},{"war", 1}};

	// Confirm expected collocates and counts match output
    for (const auto& [collocate, count] : expected) {
		EXPECT_EQ(output[collocate], expected[collocate]);
	}
}

TEST_F(test_LM, CSV){
    // Initialize bigram language model object
	LM testlm(2);

	// Confirm that train model returns true with valid text file
	ASSERT_TRUE(testlm.Train("GettysburgAddress.txt"));

	// Confirm that CSV returns true
	ASSERT_TRUE(testlm.CSV("GettysburgAddress"));

	// Confirm number of words in output CSV
	// Note: Here, "ngram,count" is considered one word and "we cannot,3" is considered two words

	// Expected number of words based on CSV created from output from http://guidetodatamining.com/ngramAnalyzer/
	const unsigned int WORDS = 493;

    // Initialize input stream
    ifstream stream;

    // Open output CSV file
    stream.open("../results/GettysburgAddress.csv");

	// Count of "words" processed
    unsigned int count = 0;  

    // Check if open failed
    if(stream.fail()){
		// Do nothing
	}
    else{
        // Step through text one word at a time, skipping white space (space, tab, newline)
        string word;             // holds current word
        while(stream >> word){
            // Increment count
            count++;
		}
		// Close stream
		stream.close();
	}

	// Confirm number of "words" in CSV matches
	EXPECT_EQ(count, WORDS);

	// Replace model with over-50-times longer valid text file of natural text
	// Confirm that train model returns true
	ASSERT_TRUE(testlm.Train("MyAntoniaChaps1-9.txt"));

	// Confirm that CSV returns true
	ASSERT_TRUE(testlm.CSV("MyAntoniaChaps1-9"));
}

TEST_F(test_LM, Remove){
    // Initialize bigram language model object
	LM testlm(2);

	// Confirm that train model returns true with valid text file
	ASSERT_TRUE(testlm.Train("GettysburgAddress.txt"));

	// Actual tokens count (including duplicates)
	const int TOKENS = 268;

	// Actual unique bigram count
	const int COUNT = 246;
	
	// Actual total bigrams (including duplicates)
	const int TOTAL = 267;

	// Confirm that "Four score" bigram exists in LM (frequency should be > 0)
	ASSERT_GT(testlm.Frequency("Four score"), 0);

	// Remove "Four score" from LM
	ASSERT_TRUE(testlm.Remove("Four score"));

	// Confirm that "Four score" no longer exists in LM (frequency should be 0)
	EXPECT_FLOAT_EQ(testlm.Frequency("Four score"), 0.0);

	// Confirm tokens count
	EXPECT_EQ(testlm.TotalTokens(), TOKENS - 2);

	// Confirm unique bigrams count
	EXPECT_EQ(testlm.UniqueNGramsCount(), COUNT - 1);

	// Confirm total bigrams count
	EXPECT_EQ(testlm.NGramsTotal(), TOTAL - 1);

	// Attempt to remove "Four score" again
	EXPECT_FALSE(testlm.Remove("Four score"));

	// Confirm tokens count
	EXPECT_EQ(testlm.TotalTokens(), TOKENS - 2);

	// Attempt to remove trigram "and seven years"
	EXPECT_FALSE(testlm.Remove("and seven years"));

	// Confirm tokens count
	EXPECT_EQ(testlm.TotalTokens(), TOKENS - 2);

	// Attempt to remove unigram "seven"
	EXPECT_FALSE(testlm.Remove("seven"));

	// Confirm tokens count
	EXPECT_EQ(testlm.TotalTokens(), TOKENS - 2);

	// Remove "to dedicate"
	EXPECT_TRUE(testlm.Remove("to dedicate"));

	// Confirm tokens count
	EXPECT_EQ(testlm.TotalTokens(), TOKENS - 4);

	// Confirm unique bigrams count
	EXPECT_EQ(testlm.UniqueNGramsCount(), COUNT - 2);

	// Confirm total bigrams count
	EXPECT_EQ(testlm.NGramsTotal(), TOTAL - 2);

	// Actual number of unique bigrams with headword "that"
	const int THAT = 10;
	
	// Get vector of all collocates with that
	vector<string> that_collocates = testlm.Collocates("that", THAT + 5);

	// Confirm number of "that" bigrams
	ASSERT_EQ(that_collocates.size(), THAT);

	// Get headword node pointer for "that"
	shared_ptr<headword_node> that_node = testlm.FindHeadword("that");

	// Get capacity of "that" headword's collocate subtable
	int capacity = that_node->collocates->capacity;

	// Remove all but two of collocates for "that"
	for (unsigned int i = 0; i < that_collocates.size() - 2; i++){
		EXPECT_TRUE(testlm.Remove("that " + that_collocates.at(i)));
	}

	// Get new vector of all collocates of "that"
	vector<string> that_collocates2 = testlm.Collocates("that", THAT + 5);

	// Confirm only two collocates remain in frequencies subtable
	EXPECT_EQ(that_collocates2.size(), 2);

	// Get new capacity of "that" headword's collocate subtable
	int capacity2 = that_node->collocates->capacity;

	// Confirm new capacity is smaller than original capacity
	// (table size should have been roughly halved and set to nearest prime)
	EXPECT_LT(capacity2, capacity);

	// Replace model with model containing exactly one bigram
	ASSERT_TRUE(testlm.Train("OneBigram.txt"));

	// Confirm that total bigram count is one
	ASSERT_EQ(testlm.NGramsTotal(), 1);

	// Confirm that total number of tokens in model is 2
	ASSERT_EQ(testlm.TotalTokens(), 2);

	// Remove only bigram ("one bigram") from LM
	ASSERT_TRUE(testlm.Remove("one bigram"));

	// Confirm that total bigram count is zero
	EXPECT_EQ(testlm.NGramsTotal(), 0);

	// Confirm that total number of tokens in model is 0
	EXPECT_EQ(testlm.TotalTokens(), 0);

	// Confirm that head table has been reset to null
	EXPECT_FALSE(testlm.main_table);

}

TEST_F(test_LM, Grow){
    // Initialize bigram language model object
	LM testlm(2);

	// Confirm that train model returns true with valid text file
	ASSERT_TRUE(testlm.Train("GettysburgAddress.txt"));

	// Actual unique bigram count based on http://guidetodatamining.com/ngramAnalyzer/
	// Note: ngramAnalyzer counts first word ("Four") as a "bigram" which results
	// in a count of 247 unique "bigrams", so this number has been corrected as 
	// 247 - 1 to obtain true bigram count
	const int COUNT = 246;
	
	// Confirm that unique bigram count matches actual
	ASSERT_EQ(testlm.UniqueNGramsCount(), COUNT);

	// Actual total bigrams (including duplicates) based on http://guidetodatamining.com/ngramAnalyzer/
	const int TOTAL = 267;

	// Actual number of tokens based on http://guidetodatamining.com/ngramAnalyzer/
	const int TOKENS = 268;

	// Confirm that grow model returns true after expanding with same text file
	ASSERT_TRUE(testlm.Grow("GettysburgAddress.txt"));

	// Confirm that bigram count matches actual
	EXPECT_EQ(testlm.UniqueNGramsCount(), COUNT);

	// Confirm that n-grams total has doubled
	EXPECT_EQ(testlm.NGramsTotal(), TOTAL*2);

	// Confirm that total number of tokens has doubled
	EXPECT_EQ(testlm.TotalTokens(), TOKENS*2);

	// Expected frequencies for several bigrams using counts from http://guidetodatamining.com/ngramAnalyzer/
	map<string, float> expected = {{"we cannot", 3.0/TOTAL},{"It is", 3.0/TOTAL},{"for us", 2.0/TOTAL},{"a new", 2.0/TOTAL},{"be here", 1.0/TOTAL},{"us to", 1.0/TOTAL}};

	// Confirm expected frequencies for each n-gram remains same
	// (because both n-gram count and total have doubled so 2/2 = 1)
    for (const auto& [ngram, freq] : expected) {
		EXPECT_FLOAT_EQ(testlm.Frequency(ngram), expected[ngram]);
	}

	// Replace model with text from original file
	ASSERT_TRUE(testlm.Train("GettysburgAddress.txt"));

	// Confirm that unique bigram count matches actual
	ASSERT_EQ(testlm.UniqueNGramsCount(), COUNT);

	// Confirm that grow model returns true after expanding with different, longer text file
	ASSERT_TRUE(testlm.Grow("MyAntoniaChaps1-9.txt"));

	// Actual number of tokens based on http://guidetodatamining.com/ngramAnalyzer/
	const int LONG_TOKENS = 15298;

	// Confirm that total number of tokens in model is sum of both texts
	EXPECT_EQ(testlm.TotalTokens(), TOKENS + LONG_TOKENS);

	// Confirm frequency of n-gram from first model has changed
	EXPECT_NE(testlm.Frequency("a new"), expected["a new"]);
	
	// Confirm that n-grams unique to each text 
	// (e.g., "Four score" and "Antonia laughed") 
	// are present (frequency should be > 0)
	ASSERT_GT(testlm.Frequency("Four score"), 0);
	ASSERT_GT(testlm.Frequency("Antonia laughed"), 0);
}