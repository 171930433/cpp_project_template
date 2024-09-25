#include <iostream>
#include <assert.h>
#include "mylib.h"

struct SA
{
    int x;
    std::string str;
};

int main()
{
#ifdef HAVE_THIRD_PARTY
    SA sa{1, "abc"};
    assert(to_string(sa) == R"({"x":1,"str":"abc"})");
#endif

    return 0;
}