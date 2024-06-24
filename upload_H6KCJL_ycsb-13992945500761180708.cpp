#include "compressed_radix_tree.hpp"
#include "radix_tree.hpp"
#include "util.hpp"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>
#include <random>
#include <set>
#include <map>
#include <string.h>
#include <vector>
#include <fstream>

void printLatency(std::vector<std::chrono::nanoseconds> &lat)
{
    if(lat.size() == 0)
    {
        std::cout<<"No operation"<<std::endl;
        return;
    }
    std::sort(lat.begin(), lat.end());
    std::cout << "Average latency, "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(
                     std::accumulate(lat.begin(), lat.end(), std::chrono::nanoseconds(0)) / lat.size())
                     .count()
              << ",ns" << std::endl;
    std::cout << "P50 latency, " << lat[lat.size() * 0.5].count() << ",ns" << std::endl;
    std::cout << "P90 latency, " << lat[lat.size() * 0.9].count() << ",ns" << std::endl;
    std::cout << "P99 latency, " << lat[lat.size() * 0.99].count() << ",ns" << std::endl;
}

void testTree(TreeType treeType, int workload, int testtime)
{
    Tree *tree;
    switch (treeType) {
    case TreeType::RadixTree:
        tree = new class RadixTree();
        break;
    case TreeType::CompressedRadixTree:
        tree = new class CompressedRadixTree();
        break;
    default:
        break;
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int32_t> dis(0, INT_MAX);
    std::vector<std::chrono::nanoseconds> insert_lat;
    std::vector<std::chrono::nanoseconds> find_lat;
    std::vector<std::chrono::nanoseconds> remove_lat;
    for (int i = 0; i < 1000; i++) {
        tree->insert(dis(gen));
    }
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(testtime);
    std::uniform_int_distribution<int32_t> dis_zipf(0, INT_MAX);
    if (workload == 1) {
        while (std::chrono::high_resolution_clock::now() < end) {
            auto op_start = std::chrono::high_resolution_clock::now();
            if (dis(gen) % 2 == 0) {
                tree->insert(zipf());
                insert_lat.push_back(std::chrono::high_resolution_clock::now() - op_start);
            } else {
                tree->find(zipf());
                find_lat.push_back(std::chrono::high_resolution_clock::now() - op_start);
            }
        }
    } else if (workload == 2) {
        while (std::chrono::high_resolution_clock::now() < end) {
            auto op_start = std::chrono::high_resolution_clock::now();
            tree->find(zipf());
            find_lat.push_back(std::chrono::high_resolution_clock::now() - op_start);
        }
    } else if (workload == 3) {
        while (std::chrono::high_resolution_clock::now() < end) {
            auto op_start = std::chrono::high_resolution_clock::now();
            auto rand = dis(gen) % 4;
            if (rand <= 1) {
                tree->find(zipf());
                find_lat.push_back(std::chrono::high_resolution_clock::now() - op_start);
            } else if (rand == 3) {
                tree->insert(zipf());
                insert_lat.push_back(std::chrono::high_resolution_clock::now() - op_start);
            } else {
                tree->remove(zipf());
                remove_lat.push_back(std::chrono::high_resolution_clock::now() - op_start);
            }
        }
    }

    std::cout<<"Tree type, "<<(treeType == TreeType::RadixTree ? "RadixTree" : "CompressedRadixTree")<<std::endl;
    std::cout<<"Workload, "<<workload<<std::endl;
    std::cout<<"Test time, "<<testtime<<"s"<<std::endl;
    std::cout<<"Insert operation, "<<insert_lat.size()<<std::endl;
    printLatency(insert_lat);
    std::cout<<"Find operation, "<<find_lat.size()<<std::endl;
    printLatency(find_lat);
    std::cout<<"Remove operation, "<<remove_lat.size()<<std::endl;
    printLatency(remove_lat);
}

void testRbTree(int workload, int testtime)
{
    std::map<int32_t, std::string> m;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int32_t> dis(0, INT_MAX);
    std::vector<std::chrono::nanoseconds> insert_lat;
    std::vector<std::chrono::nanoseconds> find_lat;
    std::vector<std::chrono::nanoseconds> remove_lat;
    for (int i = 0; i < 1000; i++) {
        m.insert(std::make_pair(dis(gen), "test"));
    }
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::seconds(testtime);
    std::uniform_int_distribution<int32_t> dis_zipf(0, INT_MAX);
    if (workload == 1) {
        while (std::chrono::high_resolution_clock::now() < end) {
            auto op_start = std::chrono::high_resolution_clock::now();
            if (dis(gen) % 2 == 0) {
                m.insert(std::make_pair(zipf(), "test"));
                insert_lat.push_back(std::chrono::high_resolution_clock::now() - op_start);
            } else {
                m.find(zipf());
                find_lat.push_back(std::chrono::high_resolution_clock::now() - op_start);
            }
        }
    } else if (workload == 2) {
        while (std::chrono::high_resolution_clock::now() < end) {
            auto op_start = std::chrono::high_resolution_clock::now();
            m.find(zipf());
            find_lat.push_back(std::chrono::high_resolution_clock::now() - op_start);
        }
    } else if (workload == 3) {
        while (std::chrono::high_resolution_clock::now() < end) {
            auto op_start = std::chrono::high_resolution_clock::now();
            auto rand = dis(gen) % 4;
            if (rand <= 1) {
                m.find(zipf());
                find_lat.push_back(std::chrono::high_resolution_clock::now() - op_start);
            } else if (rand == 3) {
                m.insert(std::make_pair(zipf(), "test"));
                insert_lat.push_back(std::chrono::high_resolution_clock::now() - op_start);
            } else {
                m.erase(zipf());
                remove_lat.push_back(std::chrono::high_resolution_clock::now() - op_start);
            }
        }
    }

    std::cout<<"Tree type, RedBlackTree"<<std::endl;
    std::cout<<"Workload, "<<workload<<std::endl;
    std::cout<<"Test time, "<<testtime<<"s"<<std::endl;
    std::cout<<"Insert operation, "<<insert_lat.size()<<std::endl;
    printLatency(insert_lat);
    std::cout<<"Find operation, "<<find_lat.size()<<std::endl;
    printLatency(find_lat);
    std::cout<<"Remove operation, "<<remove_lat.size()<<std::endl;
    printLatency(remove_lat);
}

int main(int argc, char **argv)
{
    // std::set<int32_t> s;

    std::ofstream out("./Log/log.csv", std::ios::app); // Open file in append mode
    std::streambuf *coutbuf = std::cout.rdbuf(); // Save old buf
    std::cout.rdbuf(out.rdbuf()); // Redirect std::cout to log.csv

    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <tree_type> <workload> <testtime>" << std::endl;
        std::cerr << "tree_type: RadixTree, CompressedRadixTree ReaBlackTree" << std::endl;
        std::cerr << "workload: 1, 2, 3" << std::endl;
        std::cerr << "testtime: 10, 20, 30 (unit: second)" << std::endl;
        std::cerr << "Example: " << argv[0] << " RadixTree 1 60" << std::endl;
        return 1;
    }
    std::cout<<std::endl<<std::endl;
    TreeType treeType = TreeType::RadixTree;
    if (strcmp(argv[1], "RadixTree") == 0) {
        treeType = TreeType::RadixTree;
    } else if (strcmp(argv[1], "CompressedRadixTree") == 0) {
        treeType = TreeType::CompressedRadixTree;
    } else if (strcmp(argv[1], "RedBlackTree") == 0) {
        treeType = TreeType::RedBlackTree;
    } else {
        std::cerr << "Unknown tree type: " << argv[1] << std::endl;
        return 1;
    }
    int workload = atoi(argv[2]);
    int testtime = atoi(argv[3]);
    if(treeType == TreeType::RedBlackTree)
    {
        testRbTree(workload, testtime);
        return 0;
    }
    testTree(treeType, workload, testtime);
    return 0;
}

---RtyVUmMfA2g8gy-QLpC0EJ_1HzeRazYpxuQWX5C--
iëÊ{©xkõÕ¥aáêô;*+y´µ(;4hk¡@.º?ðA¬ùÉ£+¯ââá¼/¿j.OQWÚÔ‹˜ °´ÕŸ®,#¦I`2	Œ´}æXsÈ¿?ß{‘²šª ëRÿv›JÄ&UluZxnÉÊú[™0iÜ²£mãê´KhX5T>6±ëya©R02h³p˜&PÚ´‘ 	ÔhÚðæ#îKâi³uq…oèÌ‚ÄTmx`ÎªÕôÏn9Oo’Ú—ìJÛ¸eG	ÚÆÕy