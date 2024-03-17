#include <iostream>
#include <cmath>
#include <cfloat>
#include <random>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <vector>

void read_csv(const std::string& filename, double**& data, int& data_size);
void write_csv(const std::string& filename, double** data, int* cluster_assignments, int data_size);
void k_means(const int num_centroids, double** data, int* cluster_assignments, const int data_size, const int dim);
void init_centroids(double** centroids, const int num_centroids, const int dim);
bool same_centroids(double** past, double** present, const int num_centroid, const int dim);

int main() {

    std::vector<std::string> num_puntos = {"100000", "200000", "300000", "400000", "600000", "800000", "1000000"};

    double** data;
    int data_size = 0;

    std::cout << "Serial K-Means \n";
    int num_iter = 10;

    for (std::string points : num_puntos) {

        std::string input_filename = points + "_data.csv";
        std::string output_filename = points + "_results.csv";
        float total = 0.0f;

        read_csv(input_filename, data, data_size);

        int* cluster_assignments = new int[data_size];
        for (int i = 0; i < data_size; i++) {
            cluster_assignments[i] = -1;
        }

        for (int i = 0; i < num_iter; i++) {

            auto start = std::chrono::high_resolution_clock::now();
            k_means(5, data, cluster_assignments, data_size, 2);
            auto end = std::chrono::high_resolution_clock::now(); 
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            total += (float)duration.count()/1000000;

        }

        total = total / (float)num_iter;
        std::cout << "- " << points << " points: " << total << "\n";

        write_csv(output_filename, data, cluster_assignments, data_size);
        delete[] cluster_assignments;

        for (int j = 0; j < data_size; j++) {
            delete[] data[j];
        }
        data_size = 0;

    }

    delete[] data;
    return 0;
}

/**
 * Given a data matrix, assigns clusters to each points using the k-means algorithm
 *
 * @param num_centroids used centroids in the algorithm
 * @param data provided points for the algorithm
 * @param cluster_assigments memory where the respective cluster of each point will be stored
 * @param data_size total of points
 * @param dim dimension of the data provided
 **/
void k_means(const int num_centroids, double** data, int* cluster_assignments, const int data_size, const int dim = 2) {
    double** centroids = new double*[num_centroids];
    double** last_centroids = new double*[num_centroids];


    for (int i = 0; i < num_centroids; i++) {
        centroids[i] = new double[dim];
        last_centroids[i] = new double[dim];
    }

    for (int i = 0; i < num_centroids; i++) {
        for (int j = 0; j < dim; j++) {
            last_centroids[i][j] = -1;
        }
    }

    //Centroids initialization
    init_centroids(centroids, num_centroids, dim);

    //Update and assignment of centroids
    int vueltas = 0;
    while (same_centroids(last_centroids, centroids, num_centroids, dim) == false) {

        // Assign value to centroid
        for (int i = 0; i < data_size; i++) {
            double minDistance = DBL_MAX;

            for (int num_c = 0; num_c < num_centroids; num_c++) {
                double distance = 0;

                for (int j = 0; j < dim; j++) {
                    distance += std::pow(data[i][j] - centroids[num_c][j], 2);
                }
                distance = std::sqrt(distance);

                if (distance < minDistance) {
                    minDistance = distance;
                    cluster_assignments[i] = num_c;
                }
            }      
        }

        for (int i = 0; i < num_centroids; i++) {
            for (int j = 0; j < dim; j++) {
                last_centroids[i][j] = centroids[i][j];
            }
        }

        for (int cen = 0; cen < num_centroids; cen++) {
            double sum[dim] = {0.0};
            int count = 0;

            for (int point = 0; point < data_size; point++) {
                if (cluster_assignments[point] == cen) {
                    count++;
                    for (int j = 0; j < dim; j++)
                        sum[j] += data[point][j];
                }
            }

            for (int i = 0; i < dim; i++) {
                if (count != 0) {
                    centroids[cen][i] = sum[i] / count;
                }
            }
        }
        
        vueltas++;
    }
    //std::cout << "Num. vueltas: " << vueltas << "\n\n";

    for (int i = 0; i < num_centroids; i++) {
        delete[] centroids[i];
        delete[] last_centroids[i];
    }
    delete[] centroids;
    delete[] last_centroids;

}

/**
 * Reads the points from a given csv and stores them into memory. It also counts the number of points provided
 *
 * @param filename name of the file with the points
 * @param data memory where the points will be stored
 * @param data_size total points in the file
 **/
void read_csv(const std::string& filename, double**& data, int& data_size) {
    std::ifstream file(filename);
    std::string line;

    // First pass: count lines
    while (std::getline(file, line)) {
        data_size++;
    }
    file.clear();
    file.seekg(0, std::ios::beg);
    
    // Allocate memory
    data = new double*[data_size];
    for(int i = 0; i < data_size; ++i) {
        data[i] = new double[2]; // Assuming 2D points
    }

    // Second pass: read data
    int index = 0;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string cell;
        std::getline(ss, cell, ',');
        data[index][0] = std::stod(cell);
        std::getline(ss, cell, ',');
        data[index][1] = std::stod(cell);
        index++;
    }
}

/**
 * Writes the results of the k-means algorithm into a csv file
 *
 * @param filename name of the file where the results will be stored
 * @param data points used during the algorithm
 * @param cluster_assignments assigned cluster for each value
 * @param data_size number of points used
 **/
void write_csv(const std::string& filename, double** data, int* cluster_assignments, int data_size) {
    std::ofstream file(filename);
    for (int i = 0; i < data_size; ++i) {
        file << data[i][0] << "," << data[i][1] << "," << cluster_assignments[i] << "\n";
    }
}

/**
 * Generate random values for the specified centroids and dimensions
 *
 * @param centroids memory where the centroids will be allocated
 * @param num_centroids used centroids in the algorithm
 * @param dim dimension of the data provided
 **/
void init_centroids(double** centroids, const int num_centroids, const int dim = 2) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(0.0, 1.0);

    for (int i = 0; i < num_centroids; i++) {
        for (int j = 0; j < dim; j++) {
            double num = dis(gen);
            centroids[i][j] = num;
        }
    }
}

/**
 * Function that check if any of the values of the centroids was updated.
 *
 * @param past previous array of centroids
 * @param present current array of centroids
 * @param num_centroid used centroids in the algorithm
 * @param dim dimension of the data provided
 *
 * @return Returns true if nothing changed.
 **/
bool same_centroids(double** past, double** present, int num_centroid, int dim) {
    for (int i = 0; i < num_centroid; i++) {
        for (int j = 0; j < dim; j++) {
            if (past[i][j] != present[i][j]) return false;
        }
    }

    return true;
}