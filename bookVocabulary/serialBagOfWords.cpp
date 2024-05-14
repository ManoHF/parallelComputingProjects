#include <string>
#include <iostream>
#include <map>
#include <set>
#include <fstream>
#include <vector>
#include <chrono>

void read_csv(const std::string &filename, std::map<std::string, int> &count, std::set<std::string> &vocabulary);
void write_bag_of_words(const std::string& filename, const std::vector<std::map<std::string, int>> &counts, const std::set<std::string>& vocabulary);

int main(int argc, char* argv[]) {

    if (argc == 1) {
        std::cout << "Please include the names of the files to read";
        return -1;
    }

    // Start of time measurement
    auto start = std::chrono::high_resolution_clock::now();

    int nBooks = argc - 1;
    std::vector<std::map<std::string, int>> counts(nBooks);
    std::set<std::string> vocabulary;
    std::string path;

    float total = 0.0f;
    for (int i = 1; i <= nBooks; i++) {
        std::string name = argv[i];
        path = "./books/" + name + ".txt";

        read_csv(path, counts[i - 1], vocabulary);
    }

    // End of time measurement
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    std::cout << "Tiempo total de procesamiento (sin escritura): " << duration.count() << " segundos" << std::endl;

    // Write the results into a csv file
    std::string out_file = "bag_of_words_serial.csv";
    write_bag_of_words(out_file, counts, vocabulary);

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
            }
            else {
                file << "0" << ",";
            }
        }
        file << "\n";
    }
}