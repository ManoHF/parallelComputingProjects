#include <mpi.h>
#include <string>
#include <iostream>
#include <map>
#include <set>
#include <fstream>
#include <vector>
#include <sstream>
#include <chrono>
#include <numeric>

void read_csv(const std::string &filename, std::map<std::string, int> &count, std::set<std::string> &vocabulary);
void write_bag_of_words(const std::string& filename, const std::vector<std::map<std::string, int>> &counts, const std::set<std::string>& vocabulary);
std::string serialize_set(const std::set<std::string> &s);
void deserialize_set(const std::string &s, std::set<std::string> &set);
std::string serialize_map(const std::map<std::string, int> &m);
void deserialize_map(const std::string &s, std::map<std::string, int> &m);

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0) {
            std::cout << "Por favor, proporcione al menos un nombre de archivo de libro como argumento." << std::endl;
        }
        MPI_Finalize();
        return -1;
    }

    int nBooks = argc - 1;

    // Start of time measurement
    auto start = std::chrono::high_resolution_clock::now();

    std::map<std::string, int> local_count;
    std::set<std::string> local_vocabulary;

    // Each process handles one book if there are enough
    if (rank < nBooks) {
        std::string filename = std::string("./books/") + argv[rank + 1] + ".txt";
        read_csv(filename, local_count, local_vocabulary);
    }

    // Gather local vocabularies on the root process
    std::string localVocabString = serialize_set(local_vocabulary);
    int localVocabSize = localVocabString.size();
    std::vector<int> allVocabSizes(size);
    MPI_Gather(&localVocabSize, 1, MPI_INT, allVocabSizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    std::vector<char> allVocabStrings;
    int totalVocabSize = 0;
    std::vector<int> displs(size, 0);

    if (rank == 0) {
        totalVocabSize = std::accumulate(allVocabSizes.begin(), allVocabSizes.end(), 0);
        allVocabStrings.resize(totalVocabSize);
        for (int i = 1; i < size; ++i) {
            displs[i] = displs[i - 1] + allVocabSizes[i - 1];
        }
    }

    MPI_Gatherv(localVocabString.data(), localVocabSize, MPI_CHAR, allVocabStrings.data(), allVocabSizes.data(), displs.data(), MPI_CHAR, 0, MPI_COMM_WORLD);

    // Root process adds all vocabularies
    std::set<std::string> global_vocabulary;
    if (rank == 0) {
        std::istringstream iss(std::string(allVocabStrings.data(), totalVocabSize));
        std::string word;
        while (iss >> word) {
            global_vocabulary.insert(word);
        }
    }

    // Global vocabulary is broadcasted to all processes
    std::string globalVocabString;
    if (rank == 0) {
        globalVocabString = serialize_set(global_vocabulary);
    }

    int globalVocabSize = globalVocabString.size();
    MPI_Bcast(&globalVocabSize, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0) {
        globalVocabString.resize(globalVocabSize);
    }

    MPI_Bcast(&globalVocabString[0], globalVocabSize, MPI_CHAR, 0, MPI_COMM_WORLD);

    if (rank != 0) {
        deserialize_set(globalVocabString, global_vocabulary);
    }

    // Gather all local counts on root process
    std::string localCountString = serialize_map(local_count);
    int localCountSize = localCountString.size();
    std::vector<int> allCountSizes(size);
    MPI_Gather(&localCountSize, 1, MPI_INT, allCountSizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    std::vector<char> allCountStrings;
    int totalCountSize = 0;
    std::vector<int> countDispls(size, 0);

    if (rank == 0) {
        totalCountSize = std::accumulate(allCountSizes.begin(), allCountSizes.end(), 0);
        allCountStrings.resize(totalCountSize);
        for (int i = 1; i < size; ++i) {
            countDispls[i] = countDispls[i - 1] + allCountSizes[i - 1];
        }
    }

    MPI_Gatherv(localCountString.data(), localCountSize, MPI_CHAR, allCountStrings.data(), allCountSizes.data(), countDispls.data(), MPI_CHAR, 0, MPI_COMM_WORLD);

    // Root process adds all word counts
    std::vector<std::map<std::string, int>> all_counts(size);
    if (rank == 0) {
        std::istringstream iss(std::string(allCountStrings.data(), totalCountSize));
        for (int i = 0; i < size; ++i) {
            std::string serializedMap;
            std::getline(iss, serializedMap, '|');
            deserialize_map(serializedMap, all_counts[i]);
        }
    }

    // End of time measument
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    if (rank == 0) {
        std::cout << "Tiempo total de procesamiento (sin escritura): " << duration.count() << " segundos" << std::endl;
        std::string out_file = "bag_of_words_parallel.csv";
        write_bag_of_words(out_file, all_counts, global_vocabulary);
    }

    MPI_Finalize();
    return 0;
}

/**
 * Function that reads a book in csv format and stores all its words in a set and their count on a map
 *
 * @param filename name of the file to be read
 * @param count map where the count for each word will be stored
 * @param vocabulary set with the words present in all books
 *
 **/
void read_csv(const std::string &filename, std::map<std::string, int> &count, std::set<std::string> &vocabulary) {
    std::ifstream file(filename);
    std::string word;

    while (std::getline(file, word, ',')) {
        if (count.find(word) == count.end()) {
            count[word] = 0;
        }
        count[word]++;
        vocabulary.insert(word);
    }
}

/**
 * Function that writes the counts of words present in each book given the complete vocabulary in a csv format
 *
 * @param filename name of the file where the result will be stored
 * @param counts vector of maps that contains the counts of words for each book given
 * @param vocabulary set with the words present in all books
 *
 **/
void write_bag_of_words(const std::string& filename, const std::vector<std::map<std::string, int>> &counts, const std::set<std::string>& vocabulary) {
    std::ofstream file(filename);

    for (const std::string& elem : vocabulary) {
        file << elem << ",";
    }
    file << "\n";

    for (const auto &count_map : counts) {
        for (const std::string &elem : vocabulary) {
            if (count_map.find(elem) != count_map.end()) {
                file << count_map.at(elem) << ",";
            } else {
                file << "0" << ",";
            }
        }
        file << "\n";
    }
}

/**
 * Function that turns a sets of strings into a single string where all the words are divided by white spaces.
 * Then, this string can be sent through MPI in a single communication operation.
 *
 * @param s set of strings
 *
 * @return string containing all the words in the set
 **/
std::string serialize_set(const std::set<std::string> &s) {
    std::ostringstream oss;
    for (const auto &word : s) {
        oss << word << " ";
    }
    return oss.str();
}

/**
 * Function that turns a string of words separated by white spaces into a set in order to have the
 * original set rebuilt for further processing.
 *
 * @param s serialized set as a string
 * @param set set where the words will be stored
 *
 **/
void deserialize_set(const std::string &s, std::set<std::string> &set) {
    std::istringstream iss(s);
    std::string word;
    while (iss >> word) {
        set.insert(word);
    }
}

/**
 * Function that turns a map of string keys and integer values where each key-value is separated by white spaces
 * Then, this string can be sent through MPI in a single communication operation.
 * 
 * @param s map with strings as keys and ints as values
 *
 * @return string containing words and their respective counts
 **/
std::string serialize_map(const std::map<std::string, int> &m) {
    std::ostringstream oss;
    for (const auto &pair : m) {
        oss << pair.first << " " << pair.second << " ";
    }
    return oss.str();
}

/**
 * Function that turns a string of words and counts separated by white spaces into a map in order to have the
 * original map rebuilt for further processing.
 *
 * @param s serialized map as a string
 * @param set map where the words will be stored
 *
 **/
void deserialize_map(const std::string &s, std::map<std::string, int> &m) {
    std::istringstream iss(s);
    std::string key;
    int value;
    while (iss >> key >> value) {
        m[key] = value;
    }
}