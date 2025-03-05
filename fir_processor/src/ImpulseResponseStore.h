// Copyright (c) 2021 Braingines SA - All Rights Reserved
// Unauthorized copying of this file is strictly prohibited
// Proprietary and confidential

#ifndef EAP_IMPULSERESPONSESTORE_H
#define EAP_IMPULSERESPONSESTORE_H

#include <filesystem>

#include <AudioFile.h>

constexpr auto MAX_IR_COUNT = 10;
#define LAZY_LOAD_IR_STORE false

typedef float T;

class ImpulseResponseStore {
public:
    static ImpulseResponseStore& GetInstance(uint32_t filter_length, uint32_t filter_index) {
        static ImpulseResponseStore instance(filter_length, filter_index);
        return instance;
    }

    ~ImpulseResponseStore() = default;

    ImpulseResponseStore() = delete;
    ImpulseResponseStore(const ImpulseResponseStore& other) = delete;
    ImpulseResponseStore(ImpulseResponseStore&& other) noexcept = delete;
    ImpulseResponseStore& operator=(const ImpulseResponseStore& other) = delete;
    ImpulseResponseStore& operator=(ImpulseResponseStore&& other) noexcept = delete;

    static AudioFile<T> CompensateIrGain(const AudioFile<T>& impulse_response);

    bool IsFileLoaded(int audio_file_index) const;
    bool IsFileGainCompensated(int audio_file_index) const;
    const AudioFile<T>& GetAudioFile(int audio_file_index);
    const AudioFile<T>& GetGainCompensatedIr(int audio_file_index);
    std::string GetAudioFileNameByIndex(int audio_file_index) const;
    std::wstring GetWideAudioFileNameByIndex(int audio_file_index) const;

    void InitializeAudioFileSlot(const std::filesystem::path& key, const AudioFile<T>& audio_file, bool loaded = false, bool gain_compensated = false);
    static std::vector<uint8_t> OpenFileAsRawData(const std::filesystem::path&);

    size_t GetLoadedAudioFileCount() const;

    [[nodiscard]] std::filesystem::path GetIRRootPath() const;

private:
    ImpulseResponseStore(uint32_t filter_length, uint32_t filter_index);
    [[nodiscard]] std::vector<std::filesystem::path> FindAllWavFiles() const;
    void PreloadAudioFiles();
    AudioFile<T> CreateTestImpulseResponse(uint32_t filter_length, uint32_t filter_index) const;

    bool m_preloaded {false};

    std::vector<AudioFile<T>> m_loaded_audio_files;
    std::vector<AudioFile<T>> m_gain_compensated_irs;
    std::vector<std::filesystem::path> m_loaded_audio_filenames;
    // First is whether the file has been loaded. Second is whether it has been gain compensated.
    std::vector<std::pair<bool, bool>> m_per_file_data;
    std::filesystem::path m_audio_file_path;
};

#endif // EAP_IMPULSERESPONSESTORE_H
