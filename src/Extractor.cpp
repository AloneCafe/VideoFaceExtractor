

#include <iostream>
#include <cstring>
#include <sstream>
#include <cstdlib>

#if !defined(_WIN32) && !defined(_WIN64)
#include <unistd.h>
#include <sys/wait.h>
#endif

#include "Extractor.h"

int Extractor::execute(const std::string &ffmpegPath, const std::string &inFile, const std::string &filt_path,
                       const std::string &outFile) {
#if !defined(_WIN32) && !defined(_WIN64)
    pid_t pid = fork();
    if (pid) { // parent
        wait4(pid, nullptr, 0, nullptr);
        std::cout << "任务子进程执行完毕, PID: " << pid << std::endl;
        return 0;

    } else { // child

        const char *av[] = {
                (ffmpegPath.empty() ? "/bin/ffmpeg" : ffmpegPath.data()),
                "-y",
                "-v",
                "warning",
                "-hide_banner",
                "-stats",
                "-i",
                inFile.data(),
                "-filter_complex_script",
                filt_path.data(),
                "-map",
                "[v]",
                outFile.data(),
                nullptr
        };

        fprintf(stdout, "output-> %s\n", outFile.data());

        fprintf(stdout, "exec: %s\n", [&av]() -> std::string {
            std::stringstream ss;
            for (size_t i = 0; av[i] != nullptr; ++i) {
                ss << av[i] << " ";
            }
            return ss.str();
        }().c_str());

        if (-1 == execv(ffmpegPath.empty() ? "/bin/ffmpeg" : ffmpegPath.c_str(),
                        const_cast<char *const *>(av))) {
            fprintf(stderr, "errno %d: %s\n", errno, strerror(errno));
            return 1;
        } else {
            return 0;
        }

    }

#else // WINDOWS VERSION

    const char *av[] = {
            (ffmpegPath.empty() ? "ffmpeg" : ffmpegPath.data()),
            "-y",
            "-v",
            "warning",
            "-hide_banner",
            "-stats",
            "-i",
            inFile.data(),
            "-filter_complex_script",
            filt_path.data(),
            "-map",
            "[v]",
            outFile.data(),
            nullptr
    };

    fprintf(stdout, "output-> %s\n", outFile.data());

    std::string cmd = [&av]() -> std::string {
        std::stringstream ss;
        for (size_t i = 0; av[i] != nullptr; ++i) {
            ss << av[i] << " ";
        }
        return ss.str();
    }();
    fprintf(stdout, "exec: %s\n", cmd.c_str());

    if (system(cmd.c_str()) == -1) {
        fprintf(stderr, "errno %d: %s\n", errno, strerror(errno));
        return 1;
    } else {
        return 0;
    }

#endif
}
