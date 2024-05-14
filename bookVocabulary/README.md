# Bolsa de palabras

El proyecto involucra el conteo de palabras de un conjunto de libros recibidos en formato csv. El código debe aceptar la dirección de cada uno de los libros desde la terminal y
debe de regresar una matriz con el conteo de palabras presentes en cada uno. En este caso, se buscar paralelizar mediante el uso de MPI.

## Results

Se recopilan los resultados con los tiempos promediades de 10 iteraciones para el código paralelo:

|                     |            SERIAL            |           PARALLEL           |
|:-------------------:|:----------------------------:|:----------------------------:|
| **number_of_books** | **execution_time (seconds)** | **execution_time (seconds)** |
|          1          |           0.0766651          |           0.0750764          |
|          2          |           0.0741967          |           0.0731991          |
|          3          |           0.0942225          |           0.0863241          |
|          4          |           0.171999           |           0.0697186          |
|          5          |           0.186656           |           0.0726768          |
|          6          |           0.193785           |           0.0696912          |

De manera gráfica:

<img src=https://github.com/ManoHF/parallelComputingProjects/assets/70402438/dc6b1761-fae5-4255-8ed0-61c67e5d6cf2 alt="Parallel results" width="600" height="400">

## Speed Up

Podemos observar que se logró el speed up deseado al llegar a un mínimo de cuatro libros.

<img src=https://github.com/ManoHF/parallelComputingProjects/assets/70402438/8cf61628-8e08-4007-8949-88bdd507cf95 alt="Parallel results" width="600" height="400">
