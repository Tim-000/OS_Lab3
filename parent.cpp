#include <iostream>
#include <vector>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sys/wait.h>
#include <cerrno>

struct SharedData
{
    int count;
    float nums[64];
    int error;
};

int main()
{
    std::vector<float> input;
    float x;

    while (std::cin >> x)
        input.push_back(x);

    if (input.empty())
    {
        std::cerr << "Нет введённых чисел.\n";
        return 0;
    }

    int fd = open("shared.bin", O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd < 0)
    {
        std::cerr << "Ошибка открытия файла: " << strerror(errno) << "\n";
        return 1;
    }

    if (ftruncate(fd, sizeof(SharedData)) == -1)
    {
        std::cerr << "Ошибка установки размера файла: " << strerror(errno) << "\n";
        close(fd);
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

    shm->count = input.size();
    shm->error = 0;

    for (size_t i = 0; i < input.size(); i++)
        shm->nums[i] = input[i];

    pid_t pid = fork();
    if (pid < 0)
    {
        std::cerr << "Ошибка fork: " << strerror(errno) << "\n";
        munmap(shm, sizeof(SharedData));
        return 1;
    }

    if (pid == 0)
    {
        if (execl("./child", "child", nullptr) == -1)
        {
            std::cerr << "Ошибка execl: " << strerror(errno) << "\n";
            return 1;
        }
    }

    if (wait(nullptr) == -1)
    {
        std::cerr << "Ошибка wait: " << strerror(errno) << "\n";
        munmap(shm, sizeof(SharedData));
        return 1;
    }

    if (shm->error == 1)
    {
        std::cout << "Ошибка: деление на 0. Процессы завершены.\n";
    }

    munmap(shm, sizeof(SharedData));
    return 0;
}
