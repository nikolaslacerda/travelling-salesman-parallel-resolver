#include <cstdio>
#include <cstring>
#include <vector>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <bits/stdc++.h>
#include "mpi.h"

#define TAG_ASK_FOR_JOB 1
#define TAG_JOB_DATA 2
#define TAG_STOP 3
#define TAG_RESULT 4

using namespace std;

const int INF = 1e9;

int getPathCost(vector<vector<int>> &matrix, vector<int> &path) {
    int cost = 0;
    for (int i = 0; i < path.size() - 1; i++) {
        cost += matrix[path[i]][path[i + 1]];
    }
    return cost;
}

// Busca o menor caminho a partir de cidades iniciais
vector<int> search_best_path_with_openMP(vector<vector<int>> &matrix, vector<int> &initial_cities) {

    // Numero de cidades
    int num_nodes = matrix.size() - 1;

    // Melhor custo
    int optimal_cost = INF;

    // Caminho com o melhor custo
    vector<int> solution_list;

    // Cidades para permutar
    vector<int> nodes;

    //for (int i = 2; i < matrix.size(); i++) {
	
        //nodes.erase(nodes.begin(), nodes.end());

        for (int city = 1; city <= num_nodes; city++) {
            if (city != initial_cities[0] && city != initial_cities[1]) {
                nodes.push_back(city);
            }
        }

	vector<int> possible_solution_list;
        possible_solution_list.insert(possible_solution_list.begin(), 0);
        possible_solution_list.insert(possible_solution_list.begin() + 1, initial_cities[0]);
        possible_solution_list.insert(possible_solution_list.begin() + 2, initial_cities[1]);

        do {
            possible_solution_list.insert(possible_solution_list.end(), nodes.begin(), nodes.end());
            possible_solution_list.push_back(0);
            //possible_solution_list.insert(possible_solution_list.begin(), 0);
            //possible_solution_list.insert(possible_solution_list.begin() + 1, initial_cities[0]);
            //possible_solution_list.insert(possible_solution_list.begin() + 2, initial_cities[1]);
            int current_path_weight = getPathCost(matrix, possible_solution_list);
            if (current_path_weight < optimal_cost) {
                optimal_cost = current_path_weight;
                solution_list = possible_solution_list;
            }
	    possible_solution_list.resize(3);
        } while (next_permutation(nodes.begin(), nodes.end()));
    //}
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


void master(vector<vector<int>> &cities, int slave_num) {
    //printf("[Mestre] Started\n");

    // Menor caminho
    int best_path_cost = 999999;

    // Menor caminho
    vector<int> best_path;

    // Saco de tarefas
    vector<vector<int>> bag_of_tasks = get_tasks(cities.size());

//    printf("[Mestre] Saco de tarefas com %zu tarefas\n", bag_of_tasks.size());

    // Variavel que recebe resultados
    vector<int> result;

    int work;

    // Status
    MPI_Status status, status2;

    // Controla slaves
    //int slaves_working = 0;
    //bool has_slave_alive = false;

    //printf("sn %d", slave_num);

    while (!bag_of_tasks.empty() || slave_num > 0) {

        // Aguardando pedido de tarefas
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int slave_rank = status.MPI_SOURCE;

        // Caso em que o escravo solicitou trabalho
        if (status.MPI_TAG == TAG_ASK_FOR_JOB) {

            // Recebendo a mensagem
            MPI_Recv(&work, 1, MPI_INT, slave_rank, TAG_ASK_FOR_JOB, MPI_COMM_WORLD, &status2);
            //printf("[Mestre] Recebi solicitacao de task do escravo %d\n", slave_rank);

	    //slave_num--;

            // Se existem tarefas
            if (!bag_of_tasks.empty()) {

                // Busca tarefa
                vector<int> task_to_send = bag_of_tasks.back();
                bag_of_tasks.pop_back();
                int size = task_to_send.size();

                // Envia tarefa
                //printf("[Mestre] Mandando task para o escravo %d\n", slave_rank);
                MPI_Send(&task_to_send[0], size, MPI_INT, slave_rank, TAG_JOB_DATA, MPI_COMM_WORLD);

                // Controla Slaves
                //slaves_working++;
                }
	    else {
                MPI_Send(&work, 1, MPI_INT, slave_rank, TAG_STOP, MPI_COMM_WORLD);
                //if (slaves_working == 0) {
                //    has_slave_alive = false;
                //}
		slave_num--;
            }
        }

        // Caso em que o escravo retornou resultado
        if (status.MPI_TAG == TAG_RESULT) {
            result.resize(cities.size() + 2);
            // Recebendo o resultado
            
	    MPI_Recv(&result[0], result.size(), MPI_INT, slave_rank, TAG_RESULT, MPI_COMM_WORLD, &status2);
            //printf("[Mestre] Recebi resultado da task do escravo %d\n", slave_rank);
	    //for (auto i: result)
            //    std::cout << i << ' ';
            //printf("\n");
	    if(result.back() < best_path_cost) {
		//printf("Novo melhor caminho encontrado, melhor custo mudou de %d para %d \n", best_path_cost, result.back());
		best_path_cost = result.back();
		best_path = result;
		//for (auto i: best_path)
		//	std::cout << i << ' ';
		//printf("\n");
	    }
            //slaves_working--;
        }
    }
    printf("[Result] Melhor caminho: ");
    for (int i = 0; i < best_path.size() - 1; i++){
	printf(" %d ", best_path[i]); 
	if (i != best_path.size() - 2) {
	    printf(" -> ");
	}
    }
    printf("\n");
    printf("[Result] Custo do menor caminho: %d\n", best_path_cost);
}

void slave(vector<vector<int>> &cities) {
    //printf("[Escravo] Iniciado\n");

    // Recebe tarefas
    vector<int> task_received;

    MPI_Status status, status2;

    // Armazena resultado
    vector<int> result;

    int work;
    int stop = 0;

    do {
        //printf("[Escravo] Pedindo tarefa para o mestre\n");
        MPI_Send(&work, 1, MPI_INT, 0, TAG_ASK_FOR_JOB, MPI_COMM_WORLD);

        // Aguardando a resposta do mestre
        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        // Caso em que mestre retornou tarefa
        if (status.MPI_TAG == TAG_JOB_DATA) {
            // Espera 2 cidades
            task_received.resize(2);

            // Recebendo a mensagem
            MPI_Recv(&task_received[0], 2, MPI_INT, 0, TAG_JOB_DATA, MPI_COMM_WORLD, &status2);
            //printf("[Escravo] Tarefa recebida, cidades iniciais: [%d, %d]\n", task_received[0], task_received[1]);

            // Calcula o menor caminho
            result = search_best_path_with_openMP(cities, task_received);
	    result.push_back(getPathCost(cities, result));

	    //for (auto i: result)
	    //	std::cout << i << ' ';
 	    //printf("\n");

            // Envia o resultado para o mestre
            MPI_Send(&result[0], result.size(), MPI_INT, 0, TAG_RESULT, MPI_COMM_WORLD);
        }

        // Caso em que mestre retornou parada
        if (status.MPI_TAG == TAG_STOP) {

            // Recebendo a mensagem
            MPI_Recv(&work, 1, MPI_INT, 0, TAG_STOP, MPI_COMM_WORLD, &status2);
            //printf("[Escravo] Parando de trabalhar\n");
            stop = 1;
        }
	//stop = 1;
    } while (stop == 0);
}

int main(int argc, char **argv) {
    int my_rank;
    int proc_n;
    double tempo_inicial;
    double tempo_final;

    // Cria a matrix de distancia
    freopen("data/n=14.txt", "r", stdin);
    size_t n;
    std::cin >> n;

    // Instancia a matriz
    vector<vector<int>> cities(n, vector<int>(n));

    // Popula a matriz
    for (size_t i = 0; i < n; i++)
        for (size_t j = 0; j < n; j++)
            std::cin >> cities[i][j];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);
    if (my_rank == 0) {
	tempo_inicial = MPI_Wtime();
	master(cities, proc_n - 1);
	tempo_final = MPI_Wtime();
	printf ("[Result] Tempo de execução: %3.1f segundos \n", tempo_final - tempo_inicial);
    } else {
        //printf("Escravo %d iniciado!\n", my_rank);
        slave(cities);
    }
    MPI_Finalize();
}
