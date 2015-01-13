#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string>
#include <algorithm>

#ifdef _WIN32
#include <fcntl.h> // O_BINARY
#include <io.h> // setmode
#endif


int main(int argc, const char **argv) {
    assert(argc == 2 && "Requires target name as an argument (e.g. compute_35)");
#ifdef _WIN32
    setmode(fileno(stdin), O_BINARY); // On windows bad things will happen unless we read stdin in binary mode
#endif
    std::string target(argv[1]);
    std::replace(target.begin(), target.end(), '.', '_'); // replace illegal characters
    printf("extern \"C\" {\n");
    printf("unsigned char simit_gpu_%s[] = {\n", target.c_str());
    int count = 0;
    while (1) {
        int c = getchar();
        if (c == EOF) break;
        printf("%d, ", c);
        count++;
    }
    printf("0};\n");
    printf("int simit_gpu_%s_length = %d;\n", target.c_str(), count);
    printf("}\n"); // extern "C"
    return 0;
}
