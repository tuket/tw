#include <stdio.h>
#include <stdlib.h>

template <typename T, int N>
constexpr int size(const T(&)[N]) { return N; }

void launch_test_0();
void launch_test_1();
void launch_test_2();

static void (*exampleFns[])() = {
    launch_test_0,
    launch_test_1,
    launch_test_2,
};

constexpr int numTests = size(exampleFns);

static const char* testNames[] = {
    "triangle",
    "spinning_cube",
    "mesh_viewer",
};
static_assert(size(testNames) == numTests);

int requestTestInput()
{
    printf("Choose a test number\n");
    for(int i = 0; i < numTests; i++)
        printf("%d) %s\n", i, testNames[i]);
    int index = -1;
    scanf("%d", &index);
    return index;
}

void runTestIfValid(int testIndex)
{
    if(testIndex >= 0 && testIndex < numTests)
        exampleFns[testIndex]();
}

int main(int argc, char* argv[])
{
    
    if(argc > 1)
    {
        const int testIndex = atoi(argv[1]);
        runTestIfValid(testIndex);
    }
    else
    {
        const int testIndex = requestTestInput();
        runTestIfValid(testIndex);
    }
    
}
