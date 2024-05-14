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

// Lee un archivo CSV y cuenta las palabras
// filename: nombre del archivo CSV
// count: map donde se almacenan las cuentas de palabras
// vocabulary: set donde se almacenan las palabras únicas
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

// Escribe el bag of words en un archivo CSV
// filename: nombre del archivo de salida
// counts: vector de maps que contienen las cuentas de palabras de cada libro
// vocabulary: set que contiene todas las palabras únicas
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

// Serializa un set a una cadena
// Convierte un set de palabras en una sola cadena, donde las palabras están separadas por espacios.
// Esto facilita el envío del set completo a través de MPI en una sola operación de comunicación.
std::string serialize_set(const std::set<std::string> &s) {
    std::ostringstream oss;
    for (const auto &word : s) {
        oss << word << " ";
    }
    return oss.str();
}

// Deserializa una cadena a un set
// Convierte una cadena de palabras separadas por espacios de vuelta a un set de palabras.
// Esto es necesario después de recibir la cadena serializada a través de MPI para reconstruir el set original.
void deserialize_set(const std::string &s, std::set<std::string> &set) {
    std::istringstream iss(s);
    std::string word;
    while (iss >> word) {
        set.insert(word);
    }
}

// Serializa un map a una cadena
// Convierte un map de palabras y sus cuentas en una sola cadena, donde cada par palabra-cuenta está separado por espacios.
// Esto facilita el envío del map completo a través de MPI en una sola operación de comunicación.
std::string serialize_map(const std::map<std::string, int> &m) {
    std::ostringstream oss;
    for (const auto &pair : m) {
        oss << pair.first << " " << pair.second << " ";
    }
    return oss.str();
}

// Deserializa una cadena a un map
// Convierte una cadena de pares palabra-cuenta separados por espacios de vuelta a un map de palabras y sus cuentas.
// Esto es necesario después de recibir la cadena serializada a través de MPI para reconstruir el map original.
void deserialize_map(const std::string &s, std::map<std::string, int> &m) {
    std::istringstream iss(s);
    std::string key;
    int value;
    while (iss >> key >> value) {
        m[key] = value;
    }
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) { // No se han proporcionado libros
        if (rank == 0) {
            std::cout << "Por favor, proporcione al menos un nombre de archivo de libro como argumento." << std::endl;
        }
        MPI_Finalize();
        return -1;
    }

    int nBooks = argc - 1;

    // Iniciar medición del tiempo
    auto start = std::chrono::high_resolution_clock::now();

    std::map<std::string, int> local_count;
    std::set<std::string> local_vocabulary;

    // Cada proceso maneja un libro si hay suficientes libros
    if (rank < nBooks) {
        std::string filename = std::string("./books/") + argv[rank + 1] + ".txt";
        read_csv(filename, local_count, local_vocabulary);
    }

    // Recopilar vocabularios locales en el proceso raíz
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

    // El proceso raíz agrega vocabularios
    std::set<std::string> global_vocabulary;
    if (rank == 0) {
        std::istringstream iss(std::string(allVocabStrings.data(), totalVocabSize));
        std::string word;
        while (iss >> word) {
            global_vocabulary.insert(word);
        }
    }

    // Difundir vocabulario global a todos los procesos
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

    // Recopilar conteos locales en el proceso raíz
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

    // El proceso raíz agrega los conteos de palabras
    std::vector<std::map<std::string, int>> all_counts(size);
    if (rank == 0) {
        std::istringstream iss(std::string(allCountStrings.data(), totalCountSize));
        for (int i = 0; i < size; ++i) {
            std::string serializedMap;
            std::getline(iss, serializedMap, '|');
            deserialize_map(serializedMap, all_counts[i]);
        }
    }

    // Medición final del tiempo
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
