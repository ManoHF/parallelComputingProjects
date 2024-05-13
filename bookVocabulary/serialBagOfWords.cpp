#include <string>
#include <iostream>
#include <map>
#include <set>
#include <fstream>
#include <vector>
#include <chrono>

void read_csv(const std::string &filename, std::map<std::string, int> &count, std::set<std::string> &vocabulary);
void write_bag_of_words(const std::string& filename, std::vector<std::map<std::string, int>> &counts, std::set<std::string> vocabulary);

int main(int argc, char* argv[]) {

    if (argc == 1) {
        std::cout << "Please include the names of the files to read";
        return -1;
    }

    int nBooks = argc - 1;
    std::vector<std::map<std::string, int>> counts(nBooks);
    std::set<std::string> vocabulary;
    std::string path;

    float total = 0.0f;
    for (int i = 1; i <= nBooks; i++) {
        std::string name = argv[i];
        path = "./books/" + name + ".txt";

        auto start = std::chrono::high_resolution_clock::now();

        read_csv(path, counts[i - 1], vocabulary);

        auto end = std::chrono::high_resolution_clock::now(); 
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        total += (float)duration.count()/1000000;
    }

    total = total / (float)nBooks;
    std::cout << "Total time (" << nBooks << " books): " << total << "\n";

    std::string out_file = "bag_of_words.txt";
    write_bag_of_words(out_file, counts, vocabulary);
    return 0;
}

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

void write_bag_of_words(const std::string& filename, std::vector<std::map<std::string, int>> &counts, std::set<std::string> vocabulary) {
    std::ofstream file(filename);

    for (std::string elem : vocabulary) {
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