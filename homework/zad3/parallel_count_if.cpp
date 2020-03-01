#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>
#include <thread>
#include <vector>


template<typename Iterator, typename Predicate, typename T>
struct count_part
{
    void operator()(Iterator first, Iterator last, Predicate p, T& result)
    {
        result = std::count_if(first, last, p);
    }
};

template <typename T, typename Iterator, typename Predicate>
T p_count_if(Iterator first, Iterator last, Predicate p){
    const size_t size = std::distance(first, last);
    if(size == 0)
        return 0;
    const size_t hardware_threads = std::thread::hardware_concurrency() != 0 ? std::thread::hardware_concurrency() : 2;
    const size_t block_size_for_thread = size / hardware_threads;
    const size_t used_threads = std::min(block_size_for_thread, hardware_threads);
    const size_t data_chunk = size / used_threads;

    std::vector<T> results(used_threads);
    std::vector<std::thread> threads(used_threads - 1);
    Iterator begin_it = first;
    for(size_t i = 0; i < used_threads - 1; ++i)
    {
        Iterator end_it = std::next(begin_it, data_chunk);
        threads[i] = std::thread(count_part<Iterator, Predicate, T>(), begin_it, end_it, p, std::ref(results[i]));
        begin_it = end_it;
    }
    count_part<Iterator, Predicate, T>()(begin_it, last, p, results[used_threads - 1]);
    for (auto && thread : threads)
            thread.join();

    return std::accumulate(std::begin(results), std::end(results), 0);
}

int main() {
    std::vector<int> v{1,2,3,4,5,6,5,4,5,3,2,4,6,7,4,4,5,4,4};
    unsigned int n = std::thread::hardware_concurrency();
    std::cout << n << " concurrent threads are supported.\n";
    auto start = std::chrono::high_resolution_clock::now();
    auto result = std::count_if(v.begin(), v.end(), [](auto numb){ return numb == 4;});
    auto stop = std::chrono::high_resolution_clock::now();

    auto start2 = std::chrono::high_resolution_clock::now();
    auto resultParallel = p_count_if<int>(v.begin(), v.end(), [](auto numb){ return numb == 4;});
    auto stop2 = std::chrono::high_resolution_clock::now();

    std::cout << "\nParallel algorithm result: " << resultParallel << "\n : "
              << std::chrono::duration_cast<std::chrono::microseconds>(stop2- start2).count()
              << "us" << std::endl;
    std::cout << "\nSTL algorithm result: " << result << "\n : "
              << std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count()
              << "us" << std::endl;
}