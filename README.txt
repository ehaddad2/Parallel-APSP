Authors: Stela Ciko, Elias Haddad
Parallel Floyd-Warshall: All Pairs Shortest Paths


References:
https://cse.buffalo.edu/faculty/miller/Courses/CSE633/Asmita-Gautam-Spring-2019.pdf
https://en.wikipedia.org/wiki/Parallel_all-pairs_shortest_path_algorithm#:~:text=Another%20variation%20of%20the%20problem,single-source%20shortest%20path%20algorithm


Contents:
    project.cpp 
        Contains serial version and simple parallelization.
    project_pool.cpp 
        Contains parallelization with thread pooling.
    create_matrix.cpp  
        File used to create test matrices from other larger files.


Compile:
    g++ project.cpp -o project -std=c++17
    g++ project_pool.cpp -o pool -std=c++17


Run:
    ** note: our approach only accepts number of threads that are divisors of NxN where N = num vertices
     * see raw data file provided for a list of divisors for each graph and results
     
    ** 1 or 0 will be printed at each run to signify corectness of matrix results
     * run (1) Simple parallelization before (2) Parallelization with Thread Pool so that parallel results can be
       compared with serial ones
     * output file saved in outputs folder

    => (1) Simple parellelization:

        ./project < test/[input filename] [p: num threads] [output filename]

        ./project < test/100a.txt 4 100a_4
        ./project < test/318.txt 4 318_4
        ./project < test/512.txt 16 512_16
        ./project < test/729.txt 9 729_9
        ./project < test/1000.txt 25 1000_25
        ./project < test/1414.txt 49 1414_49


    => Parallelization with Thread Pool:

        ./pool < test/[input filename]  [p: num threads] [output filename]

        ./pool < test/100a.txt 4 100a_4
        ./pool < test/318.txt 4 318_4
        ./pool < test/512.txt 16 512_16
        ./pool < test/729.txt 9 729_9
        ./pool < test/1000.txt 25 1000_25
        ./pool < test/1414.txt 49 1414_49


Notes on thread pool implementation in project_pool.cpp:

=> ThreadPool Constructor:
    When ThreadPool pool(p) is called in floydWarshallParallel_Pool(), a ThreadPool object is created with p worker threads.

=> Enqueuing Tasks:
    Inside the Floyd Warshall for-loop, tasks are enqueued for execution by the thread pool.
    Each task is represented by a lambda function that returns a call to the innerLoop() function with proper parameter values 
    for the current block.

=> Task Execution:
    Each worker thread in the thread pool executes an infinite loop where it continuously waits for tasks to be enqueued.
    When a task is enqueued, the thread dequeues the task from the task queue and executes it.
    After executing the task, the thread goes back to waiting for more tasks to be enqueued.

=> Task Completion:
    Each task is executed asynchronously by one of the worker threads in the thread pool.
    As tasks complete execution, the worker threads wait for more tasks to be enqueued.

=> ThreadPool Destructor:
    When the main function exits, the destructor of the ThreadPool object pool is called.
    It notifies all threads to wake up from their wait states and it joins all worker threads 
    to ensure they have completed their tasks before the ThreadPool object is destroyed.

