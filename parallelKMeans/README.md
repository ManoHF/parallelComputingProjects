# Parallel K-Means

This project involves implementing the K-Means algorithm using both serial and parallel techniques. The code is designed to accept a CSV file containing points that will be partitioned into subsets. It will then create a new CSV file containing the points along with their assigned clusters. The execution time will be measured using both techniques to compare the speedup achieved using OpenMP. To ensure accuracy, ten iterations will be performed and averaged for each required number of points.

## Serial K-Means Result

|   **number_of_points**   | **execution_time** |
|:------------------------:|:------------------:|
|          100,000         |      0.283438      |
|          200,000         |      0.725907      |
|          300,000         |      0.985555      |
|          400,000         |       1.17541      |
|          600,000         |       2.0213       |
|          800,000         |       3.01933      |
|         1,000,000        |       3.61154      |

## Parallel K-Means Result
