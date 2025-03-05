#include "IRFilter.h"

// TODO: Might need to add locks on this for thread safety.
//  If the m_active_ir gets updated while it is getting accessed, shit might happen.

std::vector<float>& IRFilter::GetChannelAt(int channel_index) {
    return m_active_ir[channel_index];
}

float& IRFilter::GetValueAt(int channel_index, int index) {
    return m_active_ir[channel_index][index];
}

void IRFilter::ResampleTo(double sample_rate) {
    if (m_sample_rate == sample_rate) {
        return;
    }

    if (ResampleSourceAudioFile(sample_rate)) {
        RefreshDataFromSourceAudioFile();
    }
}

unsigned int IRFilter::GetChannelCount() {
    return m_active_ir.size();
}

double IRFilter::GetSampleRate() {
    return m_sample_rate;
}

unsigned int IRFilter::GetFilterLength() {
    return m_filter_length;
}

void IRFilter::LoadImpulseResponse(int index) {
    if (index >= static_cast<int>(m_ir_store.GetLoadedAudioFileCount())) {
        return;
    }

    if (index == m_active_ir_index) {
        return;
    }

    m_active_ir_index = index;
    m_source_audio_file = m_ir_store.GetGainCompensatedIr(index);
    RefreshDataFromSourceAudioFile();
}

void IRFilter::RefreshDataFromSourceAudioFile() {
    m_active_ir = m_source_audio_file.samples;
    m_sample_rate = m_source_audio_file.getSampleRate();

    // TODO: There's no guarantee that the size of each channel is the same, unfortunately. Fix this accordingly.
    m_filter_length = m_active_ir[0].size();
}

bool IRFilter::ResampleSourceAudioFile(double sample_rate) {
    // TODO: In order for it to get resampled it needs to get saved. To get saved in memory,
    //  I need to create a file mapping. When done, needs to get unmapped. If someone tries to resample
    //  an in memory file, something will probably break because of it. Ultimately seems clumsy,
    //  is not platform independent and frankly I'm not certain the resampling would even work.
    //  Might as well just use a library specifically for resampling a raw buffer.
    // m_source_audio_file.setSampleRate(static_cast<unsigned int>(sample_rate));
    // return m_source_audio_file.save(m_ir_store.GetAudioFileNameByIndex(m_active_ir_index));
    return false;
}
