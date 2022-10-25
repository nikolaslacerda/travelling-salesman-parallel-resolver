#include <cstdio>
#include <cstring>
#include <vector>
#include <iostream>
#include <bits/stdc++.h>
#include "mpi.h"

#define TAG_ASK_FOR_JOB 1
#define TAG_JOB_DATA 2
#define TAG_STOP 3
#define TAG_RESULT 4

using namespace std;

const int INF = 1e9;

int get_path_cost(vector<vector<int>> &matrix, vector<int> &path) {
    int cost = 0;
    for (int i = 0; i < path.size() - 1; i++) {
        cost += matrix[path[i]][path[i + 1]];
    }
    return cost;
}

vector<int> search_best_path_with_openMP(vector<vector<int>> &matrix, vector<int> &initial_cities) {

    // Numero de cidades
    int num_nodes = matrix.size() - 1;
    printf("Nodes number: %d\n", num_nodes);

    // Melhor custo
    int optimal_cost = INF;

    // Caminho com o melhor custo
    vector<int> solution_list;

    // Cidades Permutadas
    vector<int> nodes;


    for (int i = 2; i < matrix[0].size(); i++) {
            
            nodes.erase(nodes.begin(), nodes.end());

            for (int l = 1; l <= num_nodes; l++) {
                if (l != initial_cities[0] && l != initial_cities[1]) {
                    nodes.push_back(l);
		}
	    }

            do {
                vector<int> possible_solution_list = nodes;
                possible_solution_list.push_back(0);
                possible_solution_list.insert(possible_solution_list.begin(), initial_cities[0]);
                possible_solution_list.insert(possible_solution_list.begin() + 1, initial_cities[1]);
                int current_path_weight = get_path_cost(matrix, possible_solution_list);

                    if (current_path_weight < optimal_cost) {
                        optimal_cost = current_path_weight;
                        solution_list = possible_solution_list;
                    }

            } while (next_permutation(nodes.begin(), nodes.end()));
  	}

    return solution_list;
	
}

// Retorna vetores de 2 cidades a partir do numero de cidades passado
vector<vector<int>> get_tasks(int num_cities) {
    vector<vector<int>> tasks;
    for (int i = 1; i < num_cities; i++) {
        for (int j = 1; j < num_cities; j++) {
            if (i != j) {
                vector<int> tuple;
                tuple.push_back(i);
                tuple.push_back(j);
                tasks.push_back(tuple);
            }
        }
    }
    return tasks;
}

void slave() {
    printf("[Escravo] Iniciado\n");

    // Recebe tarefas
    vector<int> task_received;

    MPI_Status status, status2;
    int work;
    int stop = 0;

    do {
        printf("[Escravo] Pedindo tarefa para o mestre\n");
        MPI_Send(&work, 1, MPI_INT, 0, TAG_ASK_FOR_JOB, MPI_COMM_WORLD);

        // Aguardando a resposta do mestre
        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        // Caso em que mestre retornou tarefa
        if (status.MPI_TAG == TAG_JOB_DATA) {
            // Espera 2 cidades
            task_received.resize(2);

            // Recebendo a mensagem
            MPI_Recv(&task_received[0], 2, MPI_INT, 0, TAG_JOB_DATA, MPI_COMM_WORLD, &status2);
            printf("[Escravo] Tarefa recebida: Cidades iniciais [%d, %d] \n", task_received[0], task_received[1]);

            // TODO Processar a tarefa
            work = task_received[0] + task_received[1];

            // Envia o resultado para o mestre
            MPI_Send(&work, 1, MPI_INT, 0, TAG_RESULT, MPI_COMM_WORLD);
        }

        // Caso em que mestre retornou parada
        if (status.MPI_TAG == TAG_STOP) {

            // Recebendo a mensagem
            MPI_Recv(&work, 1, MPI_INT, 0, TAG_STOP, MPI_COMM_WORLD, &status2);
            printf("[Escravo] Parando de trabalhar\n");
            stop = 1;
        }
    } while (stop == 0);
}

void master() {
    printf("[Mestre] Started\n");

    // Saco de tarefas
    vector<vector<int>> bag_of_tasks = get_tasks(4);

    // Variavel que recebe resultados
    int work;

    // Status
    MPI_Status status, status2;

    // Controla slaves
    int slaves_working = 0;
    bool has_slave_alive = false;

    while (!bag_of_tasks.empty() || has_slave_alive) {

        // Aguardando pedido de tarefas
        MPI_Probe(1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            // Envia o resultado para o mestre
            MPI_Send(&work, 1, MPI_INT, 0, TAG_RESULT, MPI_COMM_WORLD);
        }

        // Caso em que mestre retornou parada
        if (status.MPI_TAG == TAG_STOP) {

            // Recebendo a mensagem
            MPI_Recv(&work, 1, MPI_INT, 0, TAG_STOP, MPI_COMM_WORLD, &status2);
            printf("[Escravo] Parando de trabalhar\n");
            stop = 1;
        }
    }
}

int main(int argc, char **argv) {
    int my_rank;
    int proc_n;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

    // Cria a matrix de distancia a partir do arquivo passado pelo argumento
    freopen("data/n=4.txt", "r", stdin);
    size_t n;
    std::cin >> n;
    
    printf("Open file");

    // Instancia a matriz
    vector<vector<int>> matrix = { {0,9,3,2}, {9, 0, 8, 7}, {3,8,0,4}, {2,7,4,0} };

    printf("criou vetor");

    // Popula a matriz
    printf("populou matriz");
    // Inicia o algoritmo
    std::cout << matrix[0].size() << "\n";
    printf("ok");
    if (my_rank == 0) {
        master();
    } else {
        slave();
    }
    MPI_Finalize();
    return 0;
}
