# Parallel K-Means

El proyecto involucra la implementación del algoritmo K-Means de manera serial y paralela. El código debe ser diseñado de tal forma que acepte un archivo CSV que contendrá los puntos sujetos a ser clasificados en distintos clusters. Posteriormente creará un nuevo archivo CSV con los puntos y su respectivo centroide. Es necesario medir el tiempo de ejecución del algoritmo serial y paralelo para obtener el *speed up* obtenido mediante OpenMP. Con fines a obtener resultados más precisos, se realizarán 10 iteraciones para cada cantidad de puntos y hilos según sea el caso.

## Serial K-Means Result

Resultados de ejecución de la versión serial del algoritmo:

|   **number_of_points**   | **execution_time** |
|:------------------------:|:------------------:|
|          100,000         |      0.283438      |
|          200,000         |      0.725907      |
|          300,000         |      0.985555      |
|          400,000         |       1.17541      |
|          600,000         |       2.0213       |
|          800,000         |       3.01933      |
|         1,000,000        |       3.61154      |

## Parallelization Stategy

Durante el algoritmo es necesario el uso de distintos bloques 'for', los cuales pueden ser o no paralelizados. En primer lugar tenemos los bloques que no tienen ninguna dependencia interna tal como:
* asignación de memoria para los arreglos de centroides
* valores iniciales para arreglo de centroides auxiliar
* generación de valores aleatorios para los centroides
* actualización de valores para los centroides
* borrado de memoria de ambos arreglos

Se usar ya sea `parallel for` o `parallel for collapse(2)` según se tenga un ciclo sobre un arreglo o una matriz.

En el caso de la función `same_centroids()` (comparar los arreglos pasados y presentes de centroides) se evita la paralelización, ya que al dividir el código en bloques se puede llegar al caso de que no se encuentren diferencia. Por esa razón, se prefiere usar el return para cortar la función en lugar de actualizar una variable compartida por los hilos.

Hay que tener cuidado al intentar paralelizar la parte del algoritmo en donde se le asigna el centroide más cercano a cada punto. En esta sección se calcula la distancia entre el punto y cada centroide, por lo tanto es necesario tener el valor mínimo de distancia para hacer las comparaciones correctas. Dadas estas restricciones se decide usar un for paralelo en el ciclo más exterior para que los hilos se dividan el trabajo de cada punto, lo cual incluye la comparacion con cada centroide de respectiva dimensión.

## Parallel K-Means Result

|   **number_of_points**   |    **exec_time (1 thread)**   |    **exec_time (8 thread)**   |    **exec_time (16 thread)**   |    **exec_time (32 thread)**   |
|:------------------------:|:-----------------------------:|:-----------------------------:|:------------------------------:|:------------------------------:|
|          100,000         |             0.3284            |            0.22724            |            0.133924            |            0.108792            |
|          200,000         |             0.6187            |            0.33197            |            0.243697            |             0.17367            |
|          300,000         |             1.1952            |            0.46382            |            0.382082            |            0.515608            |
|          400,000         |             1.6696            |            0.80666            |            0.551755            |            0.543077            |
|          600,000         |             2.6405            |            1.05115            |            0.797415            |            0.842342            |
|          800,000         |             2.7614            |            1.74864            |             1.21176            |             1.14588            |
|         1,000,000        |             3.9947            |            1.58837            |             1.62874            |             1.21397            |

<img src=https://github.com/ManoHF/parallelComputingProjects/assets/70402438/6aa83638-a58f-4040-8b95-6ff12cdc12ea alt="Parallel results" width="600" height="400">

## Speed Up

Tenemos que la versión paralela proporciono un buen resultado de speed up para los casos con hilos mayor a uno. El caso de uno se debe al overhead que causa el setup de la paralelización para, al final, solo usar un único hilo.

<img src=https://github.com/ManoHF/parallelComputingProjects/assets/70402438/d1fc5275-e528-4027-a99a-834f22a43586 alt="Parallel results" width="600" height="400">



