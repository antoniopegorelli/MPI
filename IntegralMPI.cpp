#include <math.h>
#include <iostream>
#include <mpi.h>
#define MASTER 0
using namespace std;

// Fun��o para gerar n�mero aleat�rio
float random(float init, float max) 
{
	return init + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - init)));
}

// Fun��o de refer�ncia do tor�ide
float function(float x, float y, float z) 
{
	return pow(z, 2) + pow(sqrt(pow(x, 2) + pow(y, 2)) - 3, 2);
}

// Fun��o que verifica que est� dentro da fun��o
bool isToroide(float result) 
{
	return result <= 1;
}

float result = 0;					// Vari�vel para o resultado final

// Vari�veis de volume de teste
float xUm = 1;
float xD = 4;

float yUm = -3;
float yD = 4;

float zUm = -1;
float zD = 1;

float valor = 0;
float tempError = 0;

// Função para calcular as iterações
void monteCarlo(int iterations) 
{
	// Vari�veis para receber valores aleat�rios
	float x = 0;
	float y = 0;
	float z = 0;

	// Loop para gera��o dos pontos aleat�rios
	for (int i = 0; i <= iterations; i++) {
		x = random(xUm, xD);
		y = random(yUm, yD);
		z = random(zUm, zD);

		result = function(x, y, z);

		// Verifica se o ponto de teste est� dentro do tor�ide e atualiza valores de resultado e erro
		if (isToroide(result))
		{
			valor += 1;
			tempError += pow(1, 2);
		}
	}
} 

int main(int argc, char *argv[]) 
{
	int tasks, taskid;

	// Inicialização do MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &tasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
	
	// Inicialização do timer
	double startTime = MPI_Wtime();

	int iterations = 100000;			// Quantidade de itera��es
	
	// Verifica se foi adicionado um parâmetro de iterações e o utiliza
	if(argc > 1)
	{
		iterations = atoi(argv[1]);
	}
	float total = 0;					// Total de pontos na fun��o
	float f = 0;						// Vari�vel para o c�lculo de f
	float f2 = 0;						// Vari�vel para o c�lculo de f ao quadrado
	float V = 0;						// Vari�vel para o c�lculo do volume total
	float error = 0;					// Vari�vel para o c�lculo do erro

	// Chama a função para testar as iterações
	monteCarlo(iterations / tasks);

	// Consolida os resultados e os cálculos de f ao quadrado para calcular o erro
	MPI_Reduce(&valor, &total, 1, MPI_FLOAT, MPI_SUM, MASTER, MPI_COMM_WORLD);
	MPI_Reduce(&tempError, &f2, 1, MPI_FLOAT, MPI_SUM, MASTER, MPI_COMM_WORLD);

	// Para realizar apenas no Mestre e retornar os resultados
	if(taskid == MASTER) 
	{
		// C�lculo do volume de teste
		V = (xD - xUm) * (yD - yUm) * (zD - zUm);

		// C�lculo do resultado
		result = V * (total / iterations);

		// C�lculo do erro
		f = pow((total / iterations), 2);
		f2 = f2 / iterations;
		error = V * sqrt((f2 - f) / iterations);

		// Cálculo do tempo de processamento
		double stopTimer = MPI_Wtime();
		double totalTimer = stopTimer - startTime;

		// Exibi��o dos resultados
		cout << "Result: " << result << endl;
		cout << "Error: " << error << endl;
		cout << "Iterations: " << iterations << endl;
		cout << "Tasks: " << tasks << endl;
		cout << "Total time: " << totalTimer << endl;
	}
	MPI_Finalize();

	return 0;
}
