#include <iostream>
#include <stdlib.h>
#include <chrono>
#include <algorithm>
#include <thread>
#include <fstream>

using namespace std;
using namespace chrono;
const int INF = numeric_limits<int>::max();

int p = 0; // total available threads (num blocks)
int n = 0; // num vertices
int b = 0; // block dimension
int **dist;
int **distCpy;

void printMatrix(int **mat)
{
	for (int r = 0; r < n; r++)
	{
		for (int c = 0; c < n; c++)
		{
			cout << mat[r][c] << " ";
		}
		cout << "\n";
	}
}

void printMatrixToFile(int **mat, const std::string &filename)
{
	// Open the output file for writing
	std::ofstream outputFile("outputs/" + filename);

	// Check if the file is opened successfully
	if (!outputFile.is_open())
	{
		std::cerr << "Error: Unable to open file " << filename << " for writing." << std::endl;
		return;
	}

	// Redirect the output to the output file stream
	std::ostream &output = outputFile;

	// Print each number to file
	for (int r = 0; r < n; r++)
	{
		for (int c = 0; c < n; c++)
		{
			output << mat[r][c] << "\n";
		}
	}

	outputFile.close();
}

bool checkCorrectness(const std::string &filename1, const std::string &filename2)
{
	std::ifstream file1("outputs/" + filename1);
	std::ifstream file2("outputs/" + filename2);

	if (!file1.is_open() || !file2.is_open())
	{
		cout << "Failed to open one of the files" << endl;
		return false;
	}

	std::string line1, line2;

	// Compare each line in the files
	int r = 0;
	while (std::getline(file1, line1) && std::getline(file2, line2))
	{
		if (line1 != line2)
		{
			cout << "First incorrect matching line: " << r << endl;
			return false;
		}
		r++;
	}

	// Check if one file has more lines than the other
	if (std::getline(file1, line1) || std::getline(file2, line2))
	{
		return false;
	}

	return true;
}

void innerLoop(int k, int startRow = 0, int startCol = 0, int endRow = n, int endCol = n)
{
	for (int i = startRow; i < endRow; i++)
	{
		for (int j = startCol; j < endCol; j++)
		{
			if (dist[i][j] > (dist[i][k] + dist[k][j]) && (dist[k][j] != INF) && (dist[i][k] != INF))
				dist[i][j] = dist[i][k] + dist[k][j];
		}
	}
}

// Input: n - number of vertices
void floydWarshallSerial()
{
	for (int k = 0; k < n; k++)
	{
		innerLoop(k);
	}
}

void floydWarshallParallel()
{
	thread **threads = new thread *[p];
	for (int i = 0; i < sqrt(p); i++) // Making 2d thread array
		threads[i] = new thread[sqrt(p)];

	for (int k = 0; k < n; k++)
	{
		for (int i = 0; i < sqrt(p); i++)
		{
			for (int j = 0; j < sqrt(p); j++)
			{
				threads[i][j] = thread(innerLoop, k, i * b, j * b, (i * b) + b, (j * b) + b);
			}
		}

		for (int i = 0; i < sqrt(p); i++)
		{
			for (int j = 0; j < sqrt(p); j++)
			{
				threads[i][j].join();
			}
		}
	}
}

int main(int argc, char *argv[])
{
	cin >> n;
	dist = new int *[n];
	distCpy = new int *[n];

	for (int i = 0; i < n; i++)
	{
		dist[i] = new int[n];
		distCpy[i] = new int[n];
		for (int j = 0; j < n; j++)
		{
			cin >> dist[i][j];
			if (dist[i][j] == -1)
				dist[i][j] = INF;
		}
		copy(dist[i], dist[i] + n, distCpy[i]);
	}

	p = atoi(argv[1]); // num blocks = num threads
	string output_filename = argv[2];
	b = n / sqrt(p); // block dimensions are b x b

	cout << "Num blocks/threads: " << p << endl;
	cout << "Block size: " << b << "x" << b << endl;

	// Serial
	auto serial_start = high_resolution_clock::now();
	floydWarshallSerial();
	auto serial_end = high_resolution_clock::now();

	duration<double, milli> serial_time = serial_end - serial_start;
	printMatrixToFile(dist, output_filename + "_ser");
	cout << "Serial time is: " << serial_time.count() << " milliseconds." << endl;

	// Parallel
	dist = distCpy;

	auto parallel_start = high_resolution_clock::now();
	floydWarshallParallel();
	auto parallel_end = high_resolution_clock::now();

	duration<double, milli> parallel_time = parallel_end - parallel_start;
	printMatrixToFile(dist, output_filename + "_par");
	cout << "Parallel time is: " << parallel_time.count() << " milliseconds." << endl;

	bool correct = checkCorrectness(output_filename + "_ser", output_filename + "_par");
	cout << "Correct Results: " << correct << endl;
}