#include <iostream>
#include <iomanip>
#include "../code/LM.h"

using namespace std;

int main(){
    // Demonstration of Multi-hash N-Gram Language Model
    cout << endl << "Demonstration of Multihash Table N-Gram Language Model" << endl;

    // Initialize bigram language model object
    cout << endl << "Construct a 2-gram (bigram) Language Model (LM) object" << endl;
	LM testlm(2);

    // Train model with Gettysburg Address
    cout << endl << "Train model with plain-text file of 'Gettysburg Address' placed in '../corpus/ directory'" << endl;
	testlm.Train("GettysburgAddress.txt");

    // Total number of tokens in text including duplicates
    cout << endl << "Total number of words (i.e., tokens), including duplicates (expected: 268): " << endl;
    cout << testlm.TotalTokens() << endl;

    // Total unique tokens (i.e., unigrams) in text
    cout << endl << "Total number of unique words (i.e., unigrams), (expected: 144): " << endl;
    cout << testlm.UniqueUnigramCount() << endl;

    // Total bigrams including duplicates
    cout << endl << "Total number of bigrams, including duplicates (expected: 267): " << endl;
    cout << testlm.NGramsTotal() << endl;

    // Total unique bigrams
    cout << endl << "Total number of unique bigrams (expected: 246): " << endl;
    cout << testlm.UniqueNGramsCount() << endl;

	// Get all bigrams and their respective number of occurences in text
    cout << endl << "All bigrams and their respective number of occurrences in the text: " << endl;
	map<string, int> bigramcounts = testlm.NGrams();
    // Output each bigram and count
    for (const auto& [ngram, count] : bigramcounts) {
    	cout << setw(18) << ngram << " " << bigramcounts[ngram] << endl;
	}

	// Output CSV of all bigrams and counts
    cout << endl << "Output CSV of all bigrams and counts to '../results/GettysburgAddress.csv'" << endl;
	testlm.CSV("GettysburgAddress");

	// Get top 5 collocates which follow the headword "to"
    cout << endl << "Top 5 collocates which follow the headword 'to': " << endl;
	vector<string> collocates = testlm.Collocates("to", 5);
    // Output each collocate separated by commas
    for (unsigned int i = 0; i < collocates.size()-1; i++){
    	cout << collocates[i] << ", ";
	}
    cout << collocates[collocates.size()-1] << endl;

	// Get frequencies for several bigrams
    cout << endl << "Frequencies for several bigrams in the text: " << endl;
	vector<string> afewbigrams = {"we cannot","to the","for us","a new","be here","to dedicate"};
    for (unsigned int i = 0; i < afewbigrams.size(); i++){
       	cout << setw(11) << afewbigrams[i] << " " << fixed << setprecision(5) << testlm.Frequency(afewbigrams[i]) << endl;
    }

	// Get all collocate counts for headword "that" from model
    cout << endl << "Number of occurences for each collocate that follows the headword 'that': " << endl;
	map<string, int> collocatecounts = testlm.CollocateCounts("that");
    // Output each collocate and count
    for (const auto& [collocate, count] : collocatecounts) {
       	cout << setw(10) << collocate << " " << collocatecounts[collocate] << endl;
    }

	// Expand model with first nine chapters of Willa Cather's 'My Antonia'
    cout << endl << "Expand bigram model with first nine chapters of Willa Cather's 'My Antonia'" << endl;
    testlm.Grow("MyAntoniaChaps1-9.txt");

    // Total number of tokens in text including duplicates
    cout << endl << "Total number of words (i.e., tokens), including duplicates (expected: 268 + 15298 = 15566): " << endl;
    cout << testlm.TotalTokens() << endl;

    // Total bigrams including duplicates
    cout << endl << "Total number of bigrams, including duplicates (expected: 267 + 15297 = 15564): " << endl;
    cout << testlm.NGramsTotal() << endl;

    // Total unique bigrams
    cout << endl << "Total number of unique bigrams (expected: < 246 + 11020 = 11266): " << endl;
    cout << testlm.UniqueNGramsCount() << endl;

	// Get all collocate counts for headword "that" from model
    cout << endl << "Number of occurences for each collocate that follows the headword 'that': " << endl;
	map<string, int> newcollocatecounts = testlm.CollocateCounts("that");
    // Output each collocate and count
    for (const auto& [collocate, count] : newcollocatecounts) {
       	cout << setw(12) << collocate << " " << newcollocatecounts[collocate] << endl;
    }

    cout << endl << "Frequency with which 'that Antonia' occurs in model: " << endl;
    cout << "that Antonia " << defaultfloat << testlm.Frequency("that Antonia") << endl;

    // Remove "that Antonia" from model
    cout << endl << "Remove 'that Antonia' bigram from model" << endl;
    testlm.Remove("that Antonia");

    cout << endl << "Frequency with which 'that Antonia' now occurs in model: " << endl;
    cout << "that Antonia " << defaultfloat << testlm.Frequency("that Antonia") << endl;

    // New total number of tokens in text including duplicates
    cout << endl << "Total number of words (i.e., tokens), including duplicates (expected: 15566 - 2 = 15564): " << endl;
    cout << testlm.TotalTokens() << endl;

	// Get all collocate counts for headword "that" from model
    cout << endl << "Number of occurences for each collocate that follows the headword 'that': " << endl;
	map<string, int> newestcollocatecounts = testlm.CollocateCounts("that");
    // Output each collocate and count
    for (const auto& [collocate, count] : newestcollocatecounts) {
       	cout << setw(12) << collocate << " " << newestcollocatecounts[collocate] << endl;
    }

    // Initialize trigram language model object
    cout << endl << "Construct a 3-gram (trigram) Language Model (LM) object" << endl;
	LM testlm3(3);

    // Train trigram model with Macbeth
    cout << endl << "Train trigram model with Shakespeare's 'Macbeth'" << endl;
	testlm3.Train("Macbeth.txt");

	// Get top 5 collocates which follow the headword "Out"
    cout << endl << "Collocates which follow the headword 'Out': " << endl;
	vector<string> macbcollocates = testlm3.Collocates("Out", 5);
    // Output each collocate separated by commas
    if (macbcollocates.size() > 1){
        for (unsigned int i = 0; i < macbcollocates.size()-1; i++){
            cout << macbcollocates[i] << ", ";
        }
        cout << macbcollocates[macbcollocates.size()-1] << endl;
    }
    else if (macbcollocates.size() == 1){
        cout << macbcollocates[0] << endl;
    }

    // Replace trigram model with first chapter of A Tale of Two Cities
    cout << endl << "Replace trigram model with first chapter of Charles Dickens' 'A Tale of Two Cities'" << endl;
	testlm3.Train("TaleOfTwoCitiesChapter1.txt");

	// Get top 5 collocates which follow the headword "best"
    cout << endl << "Collocates which follow the headword 'best': " << endl;
	vector<string> talecollocates = testlm3.Collocates("best", 5);
    // Output each collocate separated by commas
    if (talecollocates.size() > 1){
        for (unsigned int i = 0; i < talecollocates.size()-1; i++){
            cout << talecollocates[i] << ", ";
        }
        cout << talecollocates[talecollocates.size()-1] << endl;
    }
    else if (talecollocates.size() == 1){
        cout << talecollocates[0] << endl;
    }

    cout << endl;

    return 0;
}