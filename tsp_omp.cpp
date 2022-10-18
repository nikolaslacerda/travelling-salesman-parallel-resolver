#include<bits/stdc++.h>
#include <ctime>
#include "mpi.h"

using namespace std;

const int INF = 1e9;

int get_path_cost(vector<vector<int>> &matrix, vector<int> &path) {
    int cost = 0;
    for (int i = 0; i < path.size() - 1; i++) {
        cost += matrix[path[i]][path[i + 1]];
    }
    return cost;
}

vector<int> search_best_path_with_openMP(vector<vector<int>> &matrix) {

    // Numero de cidades
    int num_nodes = matrix.size() - 1;

    // Melhor custo
    int optimal_cost = INF;

    // Caminho com o melhor custo
    vector<int> solution_list;

    // Cidades
    vector<int> nodes;

    // Numero de cidades por thread
    int num_nodes_per_thread = (matrix.size() - 1) / omp_get_num_threads();

    // Se a divisão não for exata
    int rest = (matrix.size() - 1) % omp_get_num_threads();
    printf("Numero de cidades que sobram %d\n", rest);

    // Inicial Node
    int initial_node = (omp_get_thread_num() * num_nodes_per_thread);

    if (id < rest) {
        num_nodes_per_thread += 1;
        initial_node += 1;
    } else {
        initial_node += 1 + rest;
    }

    for (int i = initial_node; i < initial_node + num_nodes_per_thread; i++) {
        printf("=> Inicial node da thread %d: %d\n", id, i);
        nodes.erase(nodes.begin(), nodes.end());
        for (int l = 1; l <= num_nodes; l++)
            if (l != i)
                nodes.push_back(l);
            do {
                vector<int> possible_solution_list = nodes;
                possible_solution_list.push_back(0);
                possible_solution_list.insert(possible_solution_list.begin(), 0);
                possible_solution_list.insert(possible_solution_list.begin() + 1, i);
                int current_path_weight = get_path_cost(matrix, possible_solution_list);
                    if (current_path_weight < optimal_cost) {
                        optimal_cost = current_path_weight;
                        solution_list = possible_solution_list;
                    }
            } while (next_permutation(nodes.begin(), nodes.end()));
        }
        return solution_list;
    }

int main(int argc, char *argv[]) {
    int my_rank;       // Identificador deste processo
    int proc_n;        // Numero de processos disparados pelo usuario na linha de comando (np)  
    int message;       // Buffer para as mensagens 
    MPI_COMM new_comm;                   
    MPI_Status status; // estrutura que guarda o estado de retorno

    MPI_Init(&argc , &argv); // funcao que inicializa o MPI, todo o codigo paralelo estah abaixo

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // pega pega o numero do processo atual (rank)
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);  // pega informacao do numero de processos (quantidade total)

    // Cria a matrix de distancia a partir do arquivo passado pelo argumento
    freopen(argv[1], "r", stdin);
    size_t n;
    std::cin >> n;
    // Instancia a matriz
    vector<vector<int>> matrix(n, vector<int>(n));
    // Popula a matriz
    for (size_t i = 0; i < n; i++)
        for (size_t j = 0; j < n; j++)
            std::cin >> matrix[i][j];

    if ( my_rank == 0 ) // sou o primeiro?
        master( MPI_COMM_WORLD, new_comm, matrix);
    else
	    slave( MPI_COMM_WORLD, new_comm);

    MPI_Finalize();
    return 0;
}

/* This is the master */
int master( master_comm, comm, matrix )
MPI_Comm comm;
{
    int        i,j, size;
    char       buf[256];
    MPI_Status status;

    MPI_Comm_size( master_comm, &size );

    while(true) {
        MPI_Recv( buf, 256, MPI_CHAR, i, 0, master_comm, &status );
	    fputs( buf, stdout );

    }
}

/* This is the slave */
int slave( master_comm, comm )
MPI_Comm comm;
{
    char buf[256];
    int  rank;
    
    MPI_Comm_rank( comm, &rank );
    //SimpleTimer timer;
    //timer.start();
    //vector<int> ans = search_best_path_with_openMP(matrix);
    //timer.stop();
    //double time = timer.elapsedSeconds();
    //Imprime resultados
    //int num_nodes = matrix.size();
    //printf("Solution: ");
    //for (int i = 0; i < num_nodes; i++)
    //    printf("%d -> ", ans[i]);
    //printf("0\n");
    //printf("Cost: %d\n", get_path_cost(matrix, ans));
    //printf("Time: %f seconds\n", time);
    MPI_Send( buf, strlen(buf) + 1, MPI_CHAR, 0, 0, master_comm );

    return 0;
}