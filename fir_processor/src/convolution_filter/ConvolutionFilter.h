#ifndef EARLYACCESSPRODUCT_CONVOLUTIONFILTER_H
#define EARLYACCESSPRODUCT_CONVOLUTIONFILTER_H

class ConvolutionFilter {
public:
    virtual ~ConvolutionFilter() = default;
    ConvolutionFilter() = default;
    ConvolutionFilter(const ConvolutionFilter& other) = default;
    ConvolutionFilter(ConvolutionFilter&& other) noexcept = default;
    ConvolutionFilter& operator=(const ConvolutionFilter& other) = default;
    ConvolutionFilter& operator=(ConvolutionFilter&& other) noexcept = default;

    virtual std::vector<float>& operator[](int index) {
        return GetChannelAt(index);
    };

    virtual std::vector<float>& GetChannelAt(int channel_index) = 0;

    virtual float& GetValueAt(int channel_index, int index) = 0;

    virtual double GetSampleRate() = 0;

    virtual void ResampleTo(double sample_rate) = 0;

    virtual unsigned int GetFilterLength() = 0;

    virtual unsigned int GetChannelCount() = 0;
};

#endif // EARLYACCESSPRODUCT_CONVOLUTIONFILTER_H
