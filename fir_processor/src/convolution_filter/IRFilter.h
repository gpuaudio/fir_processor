#ifndef EARLYACCESSPRODUCT_IRSTOREFILTER_H
#define EARLYACCESSPRODUCT_IRSTOREFILTER_H

#include <vector>
#include <cstdint>

#include "../ImpulseResponseStore.h"
#include "ConvolutionFilter.h"

class IRFilter : public virtual ConvolutionFilter {
public:
    IRFilter(uint32_t filter_length, uint32_t filter_index) :
        m_ir_store(ImpulseResponseStore::GetInstance(filter_length, filter_index)) {}

    std::vector<float>& GetChannelAt(int channel_index) override;
    float& GetValueAt(int channel_index, int index) override;

    void ResampleTo(double sample_rate) override;
    double GetSampleRate() override;

    unsigned int GetFilterLength() override;

    unsigned int GetChannelCount() override;

    void LoadImpulseResponse(int index);

private:
    void RefreshDataFromSourceAudioFile();
    bool ResampleSourceAudioFile(double sample_rate);

    ImpulseResponseStore& m_ir_store;
    std::vector<std::vector<float>> m_active_ir;

    AudioFile<float> m_source_audio_file;

    int m_active_ir_index = -420;
    double m_sample_rate = 0;
    uint32_t m_filter_length = 0;
};

#endif // EARLYACCESSPRODUCT_IRSTOREFILTER_H
