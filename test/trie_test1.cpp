#include "../trie/src2.hpp"
#include "../utility/print_utility.hpp"
#include <iostream>
#include <string>
#include <unordered_map>
using std::cout, std::endl;
int main() {
    sjtu::Trie trie;
    trie = trie.Put<std::string>("key1", "value1");
    trie = trie.Put<std::string>("dick", "long");
    trie = trie.Put<std::string>("keep", "up");
    trie = trie.Put<std::string>("key4", "value12");
    print_trie(trie.getroot());
    if (*trie.Get<std::string>("key1") != "value1") {
        std::cout << "Test failed: key1 does not return value1" << std::endl;
        return 1;
    }

    if (*trie.Get<std::string>("dick") != "long") {
        std::cout << "Test failed: key2 does not return value2" << std::endl;
        return 1;
    }

    if (*trie.Get<std::string>("keep") != "up") {
        std::cout << "Test failed: key3 does not return value3" << std::endl;
        return 1;
    }

    if (*trie.Get<std::string>("key4") != "value12") {
        std::cout << "Test failed: key4 does not return value4" << std::endl;
        return 1;
    }

    trie = trie.Remove("key4");

    if (trie.Get<std::string>("key4") != nullptr) {
        std::cout << "Test failed: key4 was not removed" << std::endl;
        return 1;
    }

    std::cout << "All tests passed!" << std::endl;
    return 0;
}