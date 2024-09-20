#include <iostream>
#include <assert.h>

#include <mylib.h>

int main()
{
    int a = add(1, 1);

    assert(a == 2);
    return 0;
}
