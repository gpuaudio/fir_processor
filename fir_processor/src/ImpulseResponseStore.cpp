#include "ImpulseResponseStore.h"

#include <cassert>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <future>
#include <numeric>
#include <thread>
#include <execution>
#include <utility>

#if defined(__GNUC__)
#define powf pow
#endif

ImpulseResponseStore::ImpulseResponseStore(uint32_t filter_length, uint32_t filter_index) :
    m_audio_file_path(R"(C:\)") {
    m_loaded_audio_files = std::vector<AudioFile<T>>();
    m_loaded_audio_filenames = std::vector<std::filesystem::path>();

    // TODO: Use getenv and COMMONW64 for win
    m_audio_file_path = m_audio_file_path / "Program Files" / "Common Files" / "VST3" / "GpuAudio" / "EAP" / "Impulse Responses";
    for (const auto& file_path : FindAllWavFiles()) {
        AudioFile<T> a;
        InitializeAudioFileSlot(file_path, a);
        if (m_loaded_audio_files.size() >= MAX_IR_COUNT) {
            break;
        }
    }

    if (GetLoadedAudioFileCount() == 0) {
        AudioFile<T> a = CreateTestImpulseResponse(filter_length, filter_index);
        InitializeAudioFileSlot(":memory:", a, true, true);
    }

    PreloadAudioFiles();
}

std::filesystem::path ImpulseResponseStore::GetIRRootPath() const {
    return m_audio_file_path;
}

std::vector<std::filesystem::path> ImpulseResponseStore::FindAllWavFiles() const {
    std::vector<std::filesystem::path> paths;

    if ((!std::filesystem::exists(m_audio_file_path)) || (!std::filesystem::is_directory(m_audio_file_path))) {
        return paths;
    }

    for (auto const& entry : std::filesystem::recursive_directory_iterator(m_audio_file_path)) {
        if (std::filesystem::is_regular_file(entry) && entry.path().extension() == ".wav") {
            paths.emplace_back(entry.path());
        }
    }

    return paths;
}

void ImpulseResponseStore::PreloadAudioFiles() {
    std::vector<int> a(GetLoadedAudioFileCount());
    std::iota(std::begin(a), std::end(a), 0);
#if !defined(GPU_AUDIO_MAC)
    std::for_each(std::execution::par, std::begin(a), std::end(a), [&](int i) {
#else
    std::for_each(std::begin(a), std::end(a), [&](int i) {
#endif
        this->GetGainCompensatedIr(i);
    });
    m_preloaded = true;
}

bool ImpulseResponseStore::IsFileLoaded(int audio_file_index) const {
    audio_file_index = std::max(0, std::min(audio_file_index, MAX_IR_COUNT));
    if (m_preloaded) {
        return true;
    }
    return m_per_file_data[audio_file_index].first;
}

bool ImpulseResponseStore::IsFileGainCompensated(int audio_file_index) const {
    audio_file_index = std::max(0, std::min(audio_file_index, MAX_IR_COUNT));
    if (m_preloaded) {
        return true;
    }
    return m_per_file_data[audio_file_index].second;
}

const AudioFile<T>& ImpulseResponseStore::GetAudioFile(int audio_file_index) {
    audio_file_index = std::max(0, std::min(audio_file_index, MAX_IR_COUNT));
    if (IsFileLoaded(audio_file_index)) {
        return m_loaded_audio_files[audio_file_index];
    }
    auto& a = m_loaded_audio_files[audio_file_index];
    bool loaded = a.load(m_loaded_audio_filenames[audio_file_index].string());
    assert(loaded);
    // m_loaded_audio_files[audio_file_index] = a;
    m_per_file_data[audio_file_index].first = loaded;
    return a;
}

AudioFile<T> ImpulseResponseStore::CompensateIrGain(const AudioFile<T>& impulse_response) {
    auto to_be_returned = impulse_response;
    for (auto& sample : to_be_returned.samples) {
        T sum = static_cast<T>(0);
        auto* ir = sample.data();
        const auto length = sample.size();

        // TODO: Is it ok to normalize channels separately?
        for (int s = 0; s < length; ++s) {
            sum += std::powf(ir[s], 2);
        }

        const T ir_gain_correction = std::min(1.f, 1.f / (2.0f * std::sqrt(sum)));

        for (int s = 0; s < length; ++s) {
            ir[s] *= ir_gain_correction;
        }
    }
    return to_be_returned;
}

const AudioFile<T>& ImpulseResponseStore::GetGainCompensatedIr(int audio_file_index) {
    audio_file_index = std::max(0, std::min(audio_file_index, MAX_IR_COUNT));
    if (IsFileGainCompensated(audio_file_index)) {
        return m_gain_compensated_irs[audio_file_index];
    }

    auto& a = GetAudioFile(audio_file_index);
    m_gain_compensated_irs[audio_file_index] = CompensateIrGain(a);
    m_per_file_data[audio_file_index].second = true;
    return m_gain_compensated_irs[audio_file_index];
}

std::string ImpulseResponseStore::GetAudioFileNameByIndex(int audio_file_index) const {
    audio_file_index = std::max(0, std::min(audio_file_index, MAX_IR_COUNT));
    return m_loaded_audio_filenames[audio_file_index].filename().string();
}

std::wstring ImpulseResponseStore::GetWideAudioFileNameByIndex(int audio_file_index) const {
    audio_file_index = std::max(0, std::min(audio_file_index, MAX_IR_COUNT));
    return m_loaded_audio_filenames[audio_file_index].filename().replace_extension().wstring();
}

void ImpulseResponseStore::InitializeAudioFileSlot(const std::filesystem::path& key, const AudioFile<T>& audio_file, bool loaded, bool gain_compensated) {
    m_loaded_audio_files.push_back(audio_file);
    m_gain_compensated_irs.push_back(audio_file);
    m_loaded_audio_filenames.push_back(key);
    m_per_file_data.emplace_back(loaded, gain_compensated);
}

std::vector<uint8_t> ImpulseResponseStore::OpenFileAsRawData(const std::filesystem::path& file_path) {
    std::ifstream file(file_path, std::ios::binary);

    assert(file.good());

    std::vector<uint8_t> file_data;

    file.unsetf(std::ios::skipws);

    file.seekg(0, std::ios::end);
    size_t length = file.tellg();
    file.seekg(0, std::ios::beg);

    // allocate
    file_data.resize(length);

    file.read(reinterpret_cast<char*>(file_data.data()), length);
    file.close();

    return file_data;
}

size_t ImpulseResponseStore::GetLoadedAudioFileCount() const {
    return m_loaded_audio_files.size();
}

AudioFile<T> ImpulseResponseStore::CreateTestImpulseResponse(uint32_t filter_length, uint32_t filter_index) const {
    std::vector<std::vector<T>> filter(1, std::vector<T>(filter_length, 0));
    filter[0][filter_index] = 1;

    AudioFile<T> ir;
    ir.setSampleRate(96000);
    ir.setAudioBufferSize(1, filter_length);
    ir.setAudioBuffer(filter);

    return ir;
}
