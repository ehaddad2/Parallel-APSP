#include <iostream>
#include <stdlib.h>
#include <chrono>
#include <algorithm>
#include <thread>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <fstream>
#include <barrier>

using namespace std;
using namespace chrono;

const int INF = numeric_limits<int>::max();
int n = 0; // num vertices
int p = 0; // total available threads (num blocks)
int **dist;
atomic<int> tasks_completed_ = 0;
condition_variable thread_barrier_;
mutex th_barrier_mutex_;

// ThreadPool as implemented by geeks for geeks
class ThreadPool
{
public:
	// Constructor to create a thread pool with given number of threads
	ThreadPool(size_t num_threads = thread::hardware_concurrency())
	{
		// Creating worker threads
		for (size_t i = 0; i < num_threads; ++i)
		{
			threads_.emplace_back([this]
								  { 
                while (true) { 
                    function<void()> task; 
                    { 
                        unique_lock<mutex> lock(queue_mutex_); 
						// Wait until there is a task to execute or the pool is stopped
                        cv_.wait(lock, [this] { 
                            return !tasks_.empty() || stop_; 
                        }); 
  
                        // Exit the thread in case the pool is stopped and there are no tasks 
                        if (stop_ && tasks_.empty()) { 
                            return; 
                        } 
  
                        // Get the next task from the queue 
                        task = std::move(tasks_.front()); 
                        tasks_.pop(); 
                    } 
					// Execute task
                    task(); 
                } });
		}
	}

	// Destructor to stop the thread pool
	~ThreadPool()
	{
		{
			// Lock the queue to update the stop flag safely
			unique_lock<mutex> lock(queue_mutex_);
			stop_ = true;
		}

		// Notify all threads
		cv_.notify_all();

		// Joining all worker threads to ensure they have completed their tasks
		for (auto &thread : threads_)
		{
			thread.join();
		}
	}

	// Enqueue task for execution by the thread pool
	void enqueue(function<void()> task)
	{
		{
			unique_lock<std::mutex> lock(queue_mutex_);
			tasks_.emplace(std::move(task));
		}
		cv_.notify_one();
	}

private:
	vector<thread> threads_;
	queue<function<void()>> tasks_;
	mutex queue_mutex_;
	condition_variable cv_;
	bool stop_ = false;
};

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

	// All tasks for k-th loop completed
	tasks_completed_.fetch_add(1);
	if (tasks_completed_ == p)
	{
		thread_barrier_.notify_one();
	}
}

void floydWarshallParallel_Pool()
{
	int b = n / sqrt(p); // block dimensions are b x b
	ThreadPool pool(p);
	std::barrier thread_barrier(p);

	for (int k = 0; k < n; k++)
	{
		tasks_completed_ = 0; // reset counter
		for (int i = 0; i < sqrt(p); i++)
		{
			for (int j = 0; j < sqrt(p); j++)
			{
				pool.enqueue([k, i, j, b]
							 { innerLoop(k, i * b, j * b, (i * b) + b, (j * b) + b); });
			}
		}

		// Ensure all tasks of this k iteration are done
		std::unique_lock<std::mutex> lock(th_barrier_mutex_);
		while (tasks_completed_.load() != p)
		{
			thread_barrier_.wait(lock);
		}
	}
}

int main(int argc, char *argv[])
{
	cin >> n;
	dist = new int *[n];

	// get input matrix
	for (int i = 0; i < n; i++)
	{
		dist[i] = new int[n];
		for (int j = 0; j < n; j++)
		{
			cin >> dist[i][j];
			if (dist[i][j] == -1)
				dist[i][j] = INF;
		}
	}

	p = atoi(argv[1]);
	string output_filename = argv[2];
	int b = n / sqrt(p); // block dimensions are b x b

	cout << "Num blocks/threads: " << p << endl;
	cout << "Block size: " << b << "x" << b << endl;

	auto parallel_start = high_resolution_clock::now();
	floydWarshallParallel_Pool();
	auto parallel_end = high_resolution_clock::now();

	duration<double, milli> parallel_time = parallel_end - parallel_start;
	printMatrixToFile(dist, output_filename + "_pool");

	cout << "Parallel time is: " << parallel_time.count() << " milliseconds." << endl;
	bool correct = checkCorrectness(output_filename + "_ser", output_filename + "_pool");
	cout << "Correct Results: " << correct << endl;
}