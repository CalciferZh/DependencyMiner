#include "DFD.hpp"
#include "TANE.hpp"
#include <ctime>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

void run_TANE() {
    std::string path = "./data.txt";
    TANE t;
    t.read_data(path);
    t.run();

    std::ofstream ofs("./TANE_out.txt");

    for (auto item: t.FD) {
        auto lhs = decode_to_vector(item.first);
        auto rhs = item.second;
        for (auto col: lhs) {
            if (col != rhs) {
                ofs << col + 1 << " ";
            }
        }
        ofs << "-> " << rhs + 1 << std::endl;
    }
}

void run_DFD(bool test = false) {
    srand(time(0));
    if (test) {
        DFD dfd("./test_data.txt");
        dfd.extraction();
        assert(dfd.getFD().size() == 109);
        assert(dfd.getFD().at(48).at(0) == 5);
        assert(dfd.getFD().at(48).at(1) == 11);
        assert(dfd.getFD().at(48).at(2) == 12);
    } else {
        DFD dfd("./data.txt");
        dfd.extraction();
        std::ofstream os("./result.txt");
        dfd.output(os);
    }
}

void test_speed() {
    std::vector<int> T(1000000000);
    clock_t start, end;

    start = clock();
    for (int i = 0; i < T.size(); ++i) {
        T[i] = 1;
    }
    end = clock();
    std::cout << "Time elapsed: " << (double)(end - start) / CLOCKS_PER_SEC << std::endl;

    start = clock();
    for (auto i = T.begin(); i != T.end(); ++i) {
        *i = -1;
    }
    end = clock();
    std::cout << "Time elapsed: " << (double)(end - start) / CLOCKS_PER_SEC << std::endl;

}

void test_TANE() {
    double total = 0;
    int loop = 10;

    for (int i = 0; i < loop; ++i) {
        clock_t start, end;
        start = clock();
        run_TANE();
        end = clock();
        double diff = (double)(end - start) / CLOCKS_PER_SEC;
        std::cout << "Time elapsed: " << diff << std::endl;
        total += diff;
    }
    total /= loop;
    std::cout << "Average: " << total << std::endl;
    system("pause");
}

void test_DFD() {
    double total = 0;
    int loop = 10;

    for (int i = 0; i < loop; ++i) {
        clock_t start, end;
        start = clock();
        run_DFD(false);
        end = clock();
        double diff = (double)(end - start) / CLOCKS_PER_SEC;
        std::cout << "Time elapsed: " << diff << std::endl;
        total += diff;
    }
    total /= loop;
    std::cout << "Average: " << total << std::endl;
    system("pause");
}

int main() {
    test_DFD();
    test_TANE();
    return 0;
}
