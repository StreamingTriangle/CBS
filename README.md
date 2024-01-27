This is the source code for paper "Approximate Triangle Counting in Sliding Window-based Streaming Graphs with High Accuracy". The algorithm proposed in this project is called CBS, which is a sampling-based approximated triangle counting algorithm for streaming graphs in sliding window model. It has bounded memory usage and higher accuracy than prior art SWTC in this problem setting.

### Complie and Run

we provide two demo file demo.cpp and demo-direct.cpp, one for undirected triangle counting and the other for directed triangle counting. User can compile and run then with command. 

g++ -O3 -std=c++11 demo.cpp

User can change the path of input data, window size, algorithm type, sample size, running rounds and other parameters in demo.cpp. The meaning of different algorithm type is as followings.

0: golden counter, which counts triangles precisely.

1: SWTC.

2: CBS.



The program will process the input data with the indicated algorithm and count triangle at each checkpoint, which is set every 1/50 of the sliding window, and output the triangle cout in ./result folder.

After running the program, user can use result-process.cpp and direct-result-process.cpp in ./result folder to compute the max error and average error of the approximate counting algorithm SWTC and CBS. 
User can choose the target algorithm by changing the algorithm type parameter in the these 2 cpp file. 1 means SWTC and 2 means CBS. Note that golden counter needs to be ran before computing error (namely run demo.cpp or demo-direct.cpp with algorithm type 0).
