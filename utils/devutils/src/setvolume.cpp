#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#if defined(_WIN32) && !defined(__CYGWIN__)
    #define SETVOLUME_WINDOWS 1
    #define NOMINMAX
    #include <windows.h>
    #include <mmdeviceapi.h>
    #include <endpointvolume.h>
    #include <wrl/client.h>
#else
    #define SETVOLUME_WINDOWS 0
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <unistd.h>
#endif

namespace {

struct AudioState {
    int volume = 0;
    bool muted = false;
};

int clamp_volume(int value) {
    return std::clamp(value, 0, 100);
}

bool parse_integer(const std::string& text, int& value) {
    if (text.empty())
        return false;

    try {
        std::size_t used = 0;
        long parsed = std::stol(text, &used);

        if (used != text.size() || parsed < -100000 || parsed > 100000)
            return false;

        value = static_cast<int>(parsed);
        return true;
    } catch (...) {
        return false;
    }
}

void print_state(const AudioState& state) {
    std::cout << "Volume: " << state.volume << "%\n"
              << "Muted: " << (state.muted ? "yes" : "no") << '\n';
}

void usage() {
    std::cout
        << "Usage:\n"
        << "  setvolume             Show current volume\n"
        << "  setvolume <0-100>     Set volume\n"
        << "  setvolume +<N>        Increase volume\n"
        << "  setvolume -<N>        Decrease volume\n"
        << "  setvolume mute        Mute\n"
        << "  setvolume unmute      Unmute\n";
}

#if SETVOLUME_WINDOWS

class WindowsAudio {
public:
    WindowsAudio() {
        HRESULT init = CoInitializeEx(
            nullptr,
            COINIT_APARTMENTTHREADED
        );

        owns_com = SUCCEEDED(init);

        if (FAILED(init) && init != RPC_E_CHANGED_MODE) {
            error = "COM initialization failed";
            return;
        }

        HRESULT hr = CoCreateInstance(
            __uuidof(MMDeviceEnumerator),
            nullptr,
            CLSCTX_ALL,
            __uuidof(IMMDeviceEnumerator),
            reinterpret_cast<void**>(enumerator.GetAddressOf())
        );

        if (FAILED(hr)) {
            error = "Could not create audio device enumerator";
            return;
        }

        Microsoft::WRL::ComPtr<IMMDevice> device;

        hr = enumerator->GetDefaultAudioEndpoint(
            eRender,
            eConsole,
            device.GetAddressOf()
        );

        if (FAILED(hr)) {
            error = "Default output device not found";
            return;
        }

        hr = device->Activate(
            __uuidof(IAudioEndpointVolume),
            CLSCTX_ALL,
            nullptr,
            reinterpret_cast<void**>(volume.GetAddressOf())
        );

        if (FAILED(hr))
            error = "Could not access audio endpoint volume";
    }

    ~WindowsAudio() {
        volume.Reset();
        enumerator.Reset();

        if (owns_com)
            CoUninitialize();
    }

    bool valid() const {
        return volume != nullptr;
    }

    bool get(AudioState& state) {
        float scalar = 0.0f;
        BOOL mute = FALSE;

        if (FAILED(volume->GetMasterVolumeLevelScalar(&scalar)) ||
            FAILED(volume->GetMute(&mute))) {
            error = "Could not read volume";
            return false;
        }

        state.volume = clamp_volume(
            static_cast<int>(std::lround(scalar * 100.0f))
        );

        state.muted = mute != FALSE;
        return true;
    }

    bool set_volume(int value) {
        float scalar =
            static_cast<float>(clamp_volume(value)) / 100.0f;

        if (FAILED(
                volume->SetMasterVolumeLevelScalar(scalar, nullptr)
            )) {
            error = "Could not set volume";
            return false;
        }

        return true;
    }

    bool set_mute(bool muted) {
        if (FAILED(
                volume->SetMute(muted ? TRUE : FALSE, nullptr)
            )) {
            error = "Could not change mute state";
            return false;
        }

        return true;
    }

    std::string error;

private:
    bool owns_com = false;

    Microsoft::WRL::ComPtr<IMMDeviceEnumerator> enumerator;
    Microsoft::WRL::ComPtr<IAudioEndpointVolume> volume;
};

bool backend_get(AudioState& state, std::string& error) {
    WindowsAudio audio;

    if (!audio.valid()) {
        error = audio.error;
        return false;
    }

    if (!audio.get(state)) {
        error = audio.error;
        return false;
    }

    return true;
}

bool backend_set_volume(int value, std::string& error) {
    WindowsAudio audio;

    if (!audio.valid()) {
        error = audio.error;
        return false;
    }

    if (!audio.set_volume(value)) {
        error = audio.error;
        return false;
    }

    return true;
}

bool backend_set_mute(bool muted, std::string& error) {
    WindowsAudio audio;

    if (!audio.valid()) {
        error = audio.error;
        return false;
    }

    if (!audio.set_mute(muted)) {
        error = audio.error;
        return false;
    }

    return true;
}

#else

// -----------------------------------------------------------------------------
// POSIX process utilities
// -----------------------------------------------------------------------------

bool run_process(
    const std::vector<std::string>& args,
    std::string& output
) {
    output.clear();

    if (args.empty())
        return false;

    int pipefd[2];

    if (pipe(pipefd) != 0)
        return false;

    pid_t pid = fork();

    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        return false;
    }

    if (pid == 0) {
        close(pipefd[0]);

        if (dup2(pipefd[1], STDOUT_FILENO) < 0)
            _exit(126);

        close(pipefd[1]);

        std::vector<char*> argv;
        argv.reserve(args.size() + 1);

        for (const auto& arg : args)
            argv.push_back(const_cast<char*>(arg.c_str()));

        argv.push_back(nullptr);

        execvp(argv[0], argv.data());

        _exit(errno == ENOENT ? 127 : 126);
    }

    close(pipefd[1]);

    char buffer[512];
    ssize_t count;

    while ((count = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
        output.append(buffer, static_cast<std::size_t>(count));
    }

    close(pipefd[0]);

    int status = 0;

    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR)
            return false;
    }

    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

bool run_ok(const std::vector<std::string>& args) {
    std::string ignored;
    return run_process(args, ignored);
}

bool executable_available(const char* name) {
    const char* path = std::getenv("PATH");

    if (!path)
        return false;

    std::string paths(path);
    std::size_t start = 0;

    while (start <= paths.size()) {
        std::size_t end = paths.find(':', start);

        std::string directory = paths.substr(
            start,
            end == std::string::npos
                ? std::string::npos
                : end - start
        );

        std::string candidate =
            (directory.empty() ? "." : directory) + "/" + name;

        if (access(candidate.c_str(), X_OK) == 0)
            return true;

        if (end == std::string::npos)
            break;

        start = end + 1;
    }

    return false;
}

// -----------------------------------------------------------------------------
// Common parsers
// -----------------------------------------------------------------------------

bool first_percentage(const std::string& text, int& value) {
    static const std::regex pattern(R"(([0-9]{1,3})\s*%)");

    std::smatch match;

    if (!std::regex_search(text, match, pattern))
        return false;

    try {
        value = clamp_volume(std::stoi(match[1].str()));
        return true;
    } catch (...) {
        return false;
    }
}

bool parse_wpctl(const std::string& text, AudioState& state) {
    static const std::regex pattern(
        R"(Volume:\s*([0-9]+(?:\.[0-9]+)?))"
    );

    std::smatch match;

    if (!std::regex_search(text, match, pattern))
        return false;

    try {
        double scalar = std::stod(match[1].str());

        state.volume = clamp_volume(
            static_cast<int>(std::lround(scalar * 100.0))
        );

        state.muted =
            text.find("[MUTED]") != std::string::npos;

        return true;
    } catch (...) {
        return false;
    }
}

bool parse_amixer(const std::string& text, AudioState& state) {
    if (!first_percentage(text, state.volume))
        return false;

    state.muted =
        text.find("[off]") != std::string::npos &&
        text.find("[on]") == std::string::npos;

    return true;
}

// -----------------------------------------------------------------------------
// Linux: PipeWire, PulseAudio, ALSA
// -----------------------------------------------------------------------------

enum class LinuxBackend {
    None,
    PipeWire,
    PulseAudio,
    ALSA
};

LinuxBackend detect_linux_backend() {
    if (executable_available("wpctl"))
        return LinuxBackend::PipeWire;

    if (executable_available("pactl"))
        return LinuxBackend::PulseAudio;

    if (executable_available("amixer"))
        return LinuxBackend::ALSA;

    return LinuxBackend::None;
}

bool linux_get(AudioState& state, std::string& error) {
    std::string output;

    switch (detect_linux_backend()) {
    case LinuxBackend::PipeWire:
        if (!run_process(
                {"wpctl", "get-volume", "@DEFAULT_AUDIO_SINK@"},
                output
            ) ||
            !parse_wpctl(output, state)) {
            error = "wpctl could not read volume";
            return false;
        }

        return true;

    case LinuxBackend::PulseAudio:
        if (!run_process(
                {"pactl", "get-sink-volume", "@DEFAULT_SINK@"},
                output
            ) ||
            !first_percentage(output, state.volume)) {
            error = "pactl could not read volume";
            return false;
        }

        if (!run_process(
                {"pactl", "get-sink-mute", "@DEFAULT_SINK@"},
                output
            )) {
            error = "pactl could not read mute state";
            return false;
        }

        state.muted = output.find("yes") != std::string::npos;
        return true;

    case LinuxBackend::ALSA:
        if (!run_process({"amixer", "sget", "Master"}, output) ||
            !parse_amixer(output, state)) {
            error = "amixer could not read Master volume";
            return false;
        }

        return true;

    case LinuxBackend::None:
        error = "No supported backend found (wpctl, pactl, amixer)";
        return false;
    }

    return false;
}

bool linux_set_volume(int value, std::string& error) {
    const std::string amount = std::to_string(value) + "%";

    bool success = false;

    switch (detect_linux_backend()) {
    case LinuxBackend::PipeWire:
        success = run_ok({
            "wpctl",
            "set-volume",
            "@DEFAULT_AUDIO_SINK@",
            amount
        });
        break;

    case LinuxBackend::PulseAudio:
        success = run_ok({
            "pactl",
            "set-sink-volume",
            "@DEFAULT_SINK@",
            amount
        });
        break;

    case LinuxBackend::ALSA:
        success = run_ok({
            "amixer",
            "sset",
            "Master",
            amount
        });
        break;

    case LinuxBackend::None:
        error = "No supported backend found";
        return false;
    }

    if (!success)
        error = "Could not set volume";

    return success;
}

bool linux_set_mute(bool muted, std::string& error) {
    const std::string value = muted ? "1" : "0";

    bool success = false;

    switch (detect_linux_backend()) {
    case LinuxBackend::PipeWire:
        success = run_ok({
            "wpctl",
            "set-mute",
            "@DEFAULT_AUDIO_SINK@",
            value
        });
        break;

    case LinuxBackend::PulseAudio:
        success = run_ok({
            "pactl",
            "set-sink-mute",
            "@DEFAULT_SINK@",
            value
        });
        break;

    case LinuxBackend::ALSA:
        success = run_ok({
            "amixer",
            "sset",
            "Master",
            muted ? "mute" : "unmute"
        });
        break;

    case LinuxBackend::None:
        error = "No supported backend found";
        return false;
    }

    if (!success)
        error = "Could not change mute state";

    return success;
}

// -----------------------------------------------------------------------------
// FreeBSD / DragonFly BSD: mixer
// -----------------------------------------------------------------------------

#if defined(__FreeBSD__) || defined(__DragonFly__)

bool bsd_get(AudioState& state, std::string& error) {
    std::string output;

    if (!run_process({"mixer", "-o"}, output)) {
        error = "mixer could not read audio controls";
        return false;
    }

    static const std::regex pattern(
        R"(vol\s*=\s*([0-9]+):([0-9]+))"
    );

    std::smatch match;

    if (!std::regex_search(output, match, pattern)) {
        error = "Could not find mixer volume control";
        return false;
    }

    try {
        int left = std::stoi(match[1].str());
        int right = std::stoi(match[2].str());

        state.volume = clamp_volume((left + right) / 2);

        // The output format from mixer -o does not reliably expose
        // a mute state across all supported versions and drivers.
        state.muted = false;

        return true;
    } catch (...) {
        error = "Could not parse mixer volume";
        return false;
    }
}

bool bsd_set_volume(int value, std::string& error) {
    const std::string setting =
        "vol=" + std::to_string(value) + ":" +
        std::to_string(value);

    if (!run_ok({"mixer", setting})) {
        error = "mixer could not set volume";
        return false;
    }

    return true;
}

bool bsd_set_mute(bool muted, std::string& error) {
    if (!run_ok({
            "mixer",
            std::string("vol.mute=") + (muted ? "on" : "off")
        })) {
        error = "Mixer driver does not expose a mute control";
        return false;
    }

    return true;
}

#endif

// -----------------------------------------------------------------------------
// OpenBSD: sndioctl
// -----------------------------------------------------------------------------

#if defined(__OpenBSD__)

bool openbsd_get(AudioState& state, std::string& error) {
    std::string output;

    if (!run_process(
            {"sndioctl", "-n", "output.level"},
            output
        )) {
        error = "sndioctl could not read output.level";
        return false;
    }

    try {
        double level = std::stod(output);

        state.volume = clamp_volume(
            static_cast<int>(std::lround(level * 100.0))
        );
    } catch (...) {
        error = "Could not parse sndio output level";
        return false;
    }

    // Not every OpenBSD audio device exposes output.mute.
    // Treat a missing mute control as unavailable rather than
    // failing an otherwise successful volume query.
    state.muted = false;

    if (run_process(
            {"sndioctl", "-n", "output.mute"},
            output
        )) {
        try {
            state.muted = std::stod(output) >= 0.5;
        } catch (...) {
            state.muted = false;
        }
    }

    return true;
}

bool openbsd_set_volume(int value, std::string& error) {
    const double level = static_cast<double>(value) / 100.0;

    const std::string setting =
        "output.level=" + std::to_string(level);

    if (!run_ok({"sndioctl", setting})) {
        error = "sndioctl could not set output.level";
        return false;
    }

    return true;
}

bool openbsd_set_mute(bool muted, std::string& error) {
    const std::string setting =
        std::string("output.mute=") + (muted ? "1" : "0");

    if (!run_ok({"sndioctl", setting})) {
        error = "Audio device does not expose output.mute";
        return false;
    }

    return true;
}

#endif

// -----------------------------------------------------------------------------
// NetBSD: mixerctl
// -----------------------------------------------------------------------------

#if defined(__NetBSD__)

bool parse_netbsd_master(
    const std::string& text,
    int& left,
    int& right
) {
    static const std::regex pattern(
        R"(outputs\.master\s*=\s*([0-9]+)\s*,\s*([0-9]+))"
    );

    std::smatch match;

    if (!std::regex_search(text, match, pattern))
        return false;

    try {
        left = std::stoi(match[1].str());
        right = std::stoi(match[2].str());

        return true;
    } catch (...) {
        return false;
    }
}

bool netbsd_get(AudioState& state, std::string& error) {
    std::string output;

    if (!run_process({"mixerctl", "-a"}, output)) {
        error = "mixerctl could not read mixer controls";
        return false;
    }

    int left = 0;
    int right = 0;

    if (!parse_netbsd_master(output, left, right)) {
        error = "Could not find outputs.master control";
        return false;
    }

    left = std::clamp(left, 0, 255);
    right = std::clamp(right, 0, 255);

    const int average = (left + right) / 2;

    state.volume = clamp_volume(
        static_cast<int>(
            std::lround(static_cast<double>(average) * 100.0 / 255.0)
        )
    );

    // Mute controls are driver-dependent.
    state.muted = false;

    static const std::regex mute_pattern(
        R"(outputs\.master\.mute\s*=\s*(on|off))"
    );

    std::smatch mute_match;

    if (std::regex_search(output, mute_match, mute_pattern))
        state.muted = mute_match[1].str() == "on";

    return true;
}

bool netbsd_set_volume(int value, std::string& error) {
    const int raw = static_cast<int>(
        std::lround(
            static_cast<double>(clamp_volume(value)) * 255.0 / 100.0
        )
    );

    const std::string setting =
        "outputs.master=" + std::to_string(raw) +
        "," + std::to_string(raw);

    if (!run_ok({"mixerctl", "-w", setting})) {
        error = "mixerctl could not set outputs.master";
        return false;
    }

    return true;
}

bool netbsd_set_mute(bool muted, std::string& error) {
    std::string output;

    if (!run_process({"mixerctl", "-a"}, output)) {
        error = "mixerctl could not inspect mute controls";
        return false;
    }

    static const std::regex mute_pattern(
        R"(outputs\.master\.mute\s*=\s*(on|off))"
    );

    if (!std::regex_search(output, mute_pattern)) {
        error = "Audio driver does not expose outputs.master.mute";
        return false;
    }

    const std::string setting =
        std::string("outputs.master.mute=") + (muted ? "on" : "off");

    if (!run_ok({"mixerctl", "-w", setting})) {
        error = "mixerctl could not change mute state";
        return false;
    }

    return true;
}

#endif

// -----------------------------------------------------------------------------
// Platform dispatch
// -----------------------------------------------------------------------------

bool backend_get(AudioState& state, std::string& error) {
#if defined(__linux__)
    return linux_get(state, error);

#elif defined(__FreeBSD__) || defined(__DragonFly__)
    return bsd_get(state, error);

#elif defined(__OpenBSD__)
    return openbsd_get(state, error);

#elif defined(__NetBSD__)
    return netbsd_get(state, error);

#elif defined(__CYGWIN__)
    error = "Cygwin audio backend is not implemented";
    return false;

#else
    error = "No backend implemented for this operating system";
    return false;
#endif
}

bool backend_set_volume(int value, std::string& error) {
#if defined(__linux__)
    return linux_set_volume(value, error);

#elif defined(__FreeBSD__) || defined(__DragonFly__)
    return bsd_set_volume(value, error);

#elif defined(__OpenBSD__)
    return openbsd_set_volume(value, error);

#elif defined(__NetBSD__)
    return netbsd_set_volume(value, error);

#elif defined(__CYGWIN__)
    error = "Cygwin audio backend is not implemented";
    return false;

#else
    error = "No backend implemented for this operating system";
    return false;
#endif
}

bool backend_set_mute(bool muted, std::string& error) {
#if defined(__linux__)
    return linux_set_mute(muted, error);

#elif defined(__FreeBSD__) || defined(__DragonFly__)
    return bsd_set_mute(muted, error);

#elif defined(__OpenBSD__)
    return openbsd_set_mute(muted, error);

#elif defined(__NetBSD__)
    return netbsd_set_mute(muted, error);

#elif defined(__CYGWIN__)
    error = "Cygwin audio backend is not implemented";
    return false;

#else
    error = "No backend implemented for this operating system";
    return false;
#endif
}

#endif // !SETVOLUME_WINDOWS

} // namespace

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    std::string error;

    if (argc == 1) {
        AudioState state;

        if (!backend_get(state, error)) {
            std::cerr << "setvolume: " << error << '\n';
            return 1;
        }

        print_state(state);
        return 0;
    }

    if (argc != 2) {
        usage();
        return 2;
    }

    const std::string argument(argv[1]);

    if (argument == "mute" || argument == "unmute") {
        if (!backend_set_mute(argument == "mute", error)) {
            std::cerr << "setvolume: " << error << '\n';
            return 1;
        }

        return 0;
    }

    int requested = 0;

    if (!parse_integer(argument, requested)) {
        std::cerr << "setvolume: invalid argument: "
                  << argument << '\n';
        usage();
        return 2;
    }

    if (argument.front() == '+' || argument.front() == '-') {
        if (requested < -100 || requested > 100) {
            std::cerr
                << "setvolume: adjustment must be between -100 and +100\n";
            return 2;
        }

        AudioState current;

        if (!backend_get(current, error)) {
            std::cerr << "setvolume: " << error << '\n';
            return 1;
        }

        requested = clamp_volume(current.volume + requested);
    } else if (requested < 0 || requested > 100) {
        std::cerr << "setvolume: volume must be between 0 and 100\n";
        return 2;
    }

    if (!backend_set_volume(requested, error)) {
        std::cerr << "setvolume: " << error << '\n';
        return 1;
    }

    return 0;
}