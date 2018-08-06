#include <iostream>
#include "filesystem/filesystem.hpp"

using namespace shine;

int main(){

    std::cout << filesystem::getcwd() << endl;
    std::cout << filesystem::mkdir("a/b/c/d/e/f") << endl;
    std::cout << filesystem::rmdir("a/b/c/d/e/f") << endl;
    std::cout << filesystem::tempdir() << endl;
    return 0;
}
