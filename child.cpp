#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cerrno>

struct SharedData
{
    int count;
    float nums[64];
    int error;
};

int main()
{
    int fd = open("shared.bin", O_RDWR, 0666);
    if (fd < 0)
    {
        std::cerr << "Ошибка открытия файла: " << strerror(errno) << "\n";
        return 1;
    }

    SharedData *shm = (SharedData *)mmap(
        nullptr,
        sizeof(SharedData),
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0);

    if (shm == MAP_FAILED)
    {
        std::cerr << "Ошибка mmap: " << strerror(errno) << "\n";
        close(fd);
        return 1;
    }

    close(fd);

    FILE *out = fopen("output.txt", "w");
    if (!out)
    {
        std::cerr << "Ошибка открытия файла для записи: " << strerror(errno) << "\n";
        munmap(shm, sizeof(SharedData));
        return 1;
    }

    float res = shm->nums[0];

    // for (int i = 1; i < shm->count; i++)
    // {
    //     if (shm->nums[i] == 0.0f)
    //     {
    //         shm->error = 1;
    //         fclose(out);
    //         munmap(shm, sizeof(SharedData));
    //         return 0;
    //     }
    //     res /= shm->nums[i];
    // }
    // fprintf(out, "%f\n", res);

    for (int i = 1; i < shm->count; i++)
    {
        if (shm->nums[i] == 0.0f)
        {
            shm->error = 1;
            fclose(out);
            munmap(shm, sizeof(SharedData));
            return 0;
        }
        float r = shm->nums[0] / shm->nums[i];
        fprintf(out, "%f\n", r);
    }

    fclose(out);
    munmap(shm, sizeof(SharedData));
    return 0;
}
