#pragma once
#include "choc/audio/choc_SampleBufferUtilities.h"
#include "choc/audio/choc_SampleBuffers.h"
#include "miniaudio.h"
#include <cstddef>
#include <expected>
#include <optional>
#include <tuple>
#include <utility>

template <typename T> class DataSource<T> {
protected:
  ma_data_source_base base{};
  static inline constexpr ma_data_source_vtable dsvtable = {
      &DataSource::on_read,
      &DataSource::on_seek,
      &DataSource::on_get_data_format,
      &DataSource::on_get_cursor,
      &DataSource::on_get_length,
      &DataSource::on_set_looping,
  };

public:
  DataSource() noexcept {
    ma_data_source_config config = ma_data_source_config_init();
    config.vtable = &dsvconfig;
    ma_data_source_init(&config, &base);
  }

  ~DataSource() { ma_data_source_uninit(*this); }

  // Callbacks
  // Normal read. Output buffer is specified and isn't nullptr, so fill it with
  // something.
  std::expected<void, ma_result>
  on_read(choc::buffer::InterleavedView<float> out,
          std::uint64_t &frames_read) noexcept {
    return MA_NOT_IMPLEMENTED;
  }

  // This callback is called when the output buffer is nullptr. This usually
  // indicates that the reader just wishes to seek forward by some certain
  // number of frames. Miniaudio will do this for us, but this is here because
  // we still need to do *something*.
  std::expected<void, ma_result> on_read() noexcept {
    return MA_NOT_IMPLEMENTED;
  }

  std::expected<void, ma_result>
  on_seek(const std::uint64_t frame_index) noexcept {
    return MA_NOT_IMPLEMENTED;
  }

  std::expected<
      std::tuple<ma_format, ma_uint32, ma_uint32, gtl::vector<ma_channel>>,
      ma_result>
  on_get_data_format(const std::size_t channel_map_cap) noexcept {
    return MA_NOT_IMPLEMENTED;
  }

  std::expected<std::uint64_t, ma_result> on_get_cursor() noexcept {
    return MA_NOT_IMPLEMENTED;
  }

  std::expected<std::uint64_t, ma_result> on_get_length() noexcept {
    return MA_NOT_IMPLEMENTED;
  }

  std::expected<void, ma_result> on_set_looping(const bool looping) noexcept {
    return MA_NOT_IMPLEMENTED;
  }

  operator ma_data_source *() noexcept {
    return reinterpret_cast<ma_data_source *>(&base);
  }

  operator const ma_data_source *() const noexcept {
    return reinterpret_cast<const ma_data_source *>(&base);
  }

  std::expected<std::uint64_t, ma_result>
  seek_frames(const std::uint64_t count) final noexcept {
    std::uint64_t frames_seeked;
    if (const auto res =
            ma_data_source_seek_pcm_frames(*this, frame_count, &frames_seeked);
        res == MA_SUCCESS) {
      return frames_seeked;
    } else {
      return res;
    }
  }

  std::expected<float, ma_result>
  seek_seconds(const float seconds) final noexcept {
    float seconds_seeked;
    if (const auto res =
            ma_data_source_seek_seconds(*this, seconds, &seconds_seeked);
        res == MA_SUCCESS) {
      return seconds_seeked;
    } else {
      return res;
    }
  }

  std::expected<void, ma_result>
  seek_to_frame(const std::uint64_t frame) final noexcept {
    if (const auto res = ma_data_source_seek_to_pcm_frame(*this, frame);
        res == MA_SUCCESS) {
      return {};
    } else {
      return res;
    }
  }

  std::expected<void, ma_result>
  seek_to_seconds(const float seconds) final noexcept {
    if (const auto res = ma_data_source_seek_to_second(*this, second);
        res == MA_SUCCESS) {
      return {};
    } else {
      return res;
    }
  }

  std::expected<
      std::tuple<ma_format, ma_uint32, ma_uint32, gtl::vector<ma_channel>>,
      ma_result>
  get_data_format() final noexcept {
    ma_format format;
    ma_uint32 channels, sample_rate;
    gtl::vector<ma_channel> channelmap;
    channelmap.resize(MA_MAX_CHANNELS);
    if (const auto res = ma_data_source_get_data_format(
            *this, &format, &channels, &sample_rate, channelmap.data(),
            channelmap.size());
        res == MA_SUCCESS) {
      return std::make_tuple(format, channels, sample_rate, channelmap);
    } else {
      return res;
    }
  }

  std::expected<std::uint64_t, ma_result> get_cursor_frames() final noexcept {
    std::uint64_t frames;
    if (const auto res =
            ma_data_source_get_cursor_in_pcm_frames(*this, &frames);
        res == MA_SUCCESS) {
      return frames;
    } else {
      return res;
    }
  }

  std::expected<std::uint64_t, ma_result> get_length_frames() final noexcept {
    std::uint64_t frames;
    if (const auto res =
            ma_data_source_get_length_in_pcm_frames(*this, &frames);
        res == MA_SUCCESS) {
      return frames;
    } else {
      return res;
    }
  }

  std::expected<float, ma_result> get_cursor_seconds() final noexcept {
    float seconds;
    if (const auto res = ma_data_source_get_cursor_in_seconds(*this, &seconds);
        res == MA_SUCCESS) {
      return seconds;
    } else {
      return res;
    }
  }

  std::expected<float, ma_result> get_length_seconds() final noexcept {
    float seconds;
    if (const auto res = ma_data_source_get_length_in_seconds(*this, &seconds);
        res == MA_SUCCESS) {
      return seconds;
    } else {
      return res;
    }
  }

  std::expected<void, ma_result>
  set_looping(const bool looping) final noexcept {
    if (const auto res =
            ma_data_source_set_looping(*this, static_cast<ma_bool32>(looping));
        res == MA_SUCCESS) {
      return {};
    } else {
      return res;
    }
  }

  bool is_looping() final noexcept { return ma_data_source_is_looping(*this); }

  std::expected<void, ma_result>
  set_pcm_range(const std::uint64_t start,
                const std::uint64_t end) final noexcept {
    if (const auto res =
            ma_data_source_set_range_in_pcm_frames(*this, start, end);
        res == MA_SUCCESS) {
      return {};
    } else {
      return res;
    }
  }

  std::expected<void, ma_result> set_pcm_range(
      const std::tuple<std::uint64_t, std::uint64_t> range) final noexcept {
    const auto [start, end] = range;
    if (const auto res =
            ma_data_source_set_range_in_pcm_frames(*this, start, end);
        res == MA_SUCCESS) {
      return {};
    } else {
      return res;
    }
  }

  std::tuple<std::uint64_t, std::uint64_t> get_pcm_range() final noexcept {
    std::uint64_t start, end;
    ma_data_source_get_range_in_pcm_frames(*this, &start, &end);
    return {start, end};
  }

  std::expected<void, ma_result>
  set_loop_point(const std::uint64_t start_frames,
                 const std::uint64_t end_frames) final noexcept {
    if (const auto res =
            ma_data_source_set_loop_point_in_pcm_frames(*this, start, end);
        res == MA_SUCCESS) {
      return {};
    } else {
      return res;
    }
  }

  std::expected<void, ma_result> set_loop_point(
      const std::tuple<std::uint64_t, std::uint64_t> range) final noexcept {
    const auto [start, end] = range;
    if (const auto res =
            ma_data_source_set_loop_point_in_pcm_frames(*this, start, end);
        res == MA_SUCCESS) {
      return {};
    } else {
      return res;
    }
  }

  std::tuple<std::uint64_t, std::uint64_t> get_pcm_range() final noexcept {
    std::uint64_t start, end;
    ma_data_source_get_loop_point_in_pcm_frames(*this, &start, &end);
    return {start, end};
  }

  static ma_result on_read(ma_data_source *pDataSource, void *pFramesOut,
                           ma_uint64 frameCount,
                           ma_uint64 *pFramesRead) noexcept {
    if (!pDataSource || !pFramesRead)
      return MA_INVALID_ARGS;
    T *self = static_cast<T *>(pDataSource);
    if (pFramesOut) {
      if (const auto format_res = self->get_data_format(); format_res) {
        const auto [format, channels, sample_rate, channel_map] = *format_res;
        if (const auto res =
                self->on_read(choc::buffer::createInterleavedView(
                                  reinterpret_cast<float *>(pFramesOut),
                                  channels, frameCount),
                              *pFramesRead);
            res) {
          return MA_SUCCESS;
        } else {
          return res.error();
        }
      } else {
        return format_res.error();
      }
    } else {
      if (const auto res = self->on_read(); res) {
        return MA_SUCCESS;
      } else {
        return res.error();
      }
    }
  }

  static ma_result on_seek(ma_data_source *pDataSource,
                           ma_uint64_t frameIndex) noexcept {
    if (!pDataSource)
      return MA_INVALID_ARGS;
    T *self = static_cast<T *>(pDataSource);
    if (const auto res = self->on_seek(frameIndex); res) {
      return MA_SUCCESS;
    } else {
      return res.error();
    }
  }

  static ma_result on_get_data_format(ma_data_source *pDataSource,
                                      ma_format *pFormat, ma_uint32 *pChannels,
                                      ma_uint32 *pSampleRate,
                                      ma_channel *pChannelMap,
                                      size_t channelMapCap) noexcept {
    if (!pDataSource || !pFormat || !pChannels || !pSampleRate ||
        !pChannelMap || channelMapCap > MA_MAX_CHANNELS ||
        channelMapCap < MA_MIN_CHANNELS)
      return MA_INVALID_ARGS;
    T *self = static_cast<T *>(pDataSource);
    if (const auto res = self->on_get_data_format(channelMapCap); res) {
      const auto format_data = *res;
      if (pFormat)
        *pFormat = std::get<0>(format_data);
      if (pChannels)
        *pChannels = std::get<1>(format_data);
      if (pSampleRate)
        *pSampleRate = std::get<2>(format_data);
      if (pChannelMap) {
        const auto channelmap = std::get<3>(format_data);
        for (std::size_t i = 0; i < channelMapCap; i++) {
          if (i >= channelmap.size())
            break;
          pChannelMap[i] = channelmap[i];
        }
      }
      return MA_SUCCESS;
    } else {
      return res.error();
    }
  }

  static ma_result on_get_cursor(ma_data_source *pDataSource,
                                 ma_uint64 *pCursor) noexcept {
    if (!pDataSource || !pCursor)
      return MA_INVALID_ARGS;
    T *self = static_cast<T *>(pDataSource);
    if (const auto res = self->on_get_cursor(); res) {
      *pCursor = *res;
      return MA_SUCCESS;
    } else {
      return res.error();
    }
  }

  static ma_result on_get_length(ma_data_source *pDataSource,
                                 ma_uint64 *pLength) noexcept {
    if (!pDataSource || !pLength)
      return MA_INVALID_ARGS;
    T *self = static_cast<T *>(pDataSource);
    if (const auto res = self->on_get_length(); res) {
      *pLength = *res;
      return MA_SUCCESS;
    } else {
      return res.error();
    }
  }

  static ma_result on_set_looping(ma_data_source *pDataSource,
                                  ma_bool32 isLooping) noexcept {
    if (!pDataSource)
      return MA_INVALID_ARGS;
    T *self = static_cast<T *>(pDataSource);
    auto res = self->on_set_looping(static_cast<bool>(isLooping));
    return res ? MA_SUCCESS : res.error();
  }
};

template <typename T, ma_uint8 input_buses, ma_uint8 output_buses,
          ma_uint32 flags>
class AudioNode<T, input_buses, output_buses, flags> {
protected:
  ma_node_base base{};
  static inline constexpr ma_node_vtable nvtable = {
      &AudioNode::on_process, &AudioNode::on_get_required_input_frames,
      input_buses, output_buses, flags};

public:
  AudioNode(ma_node_graph *graph,
            std::optional<ma_uint32> actual_input_bus_count = std::nullopt,
            std::optional<ma_uint32> actual_output_bus_count = std::nullopt) {
    auto config = ma_node_config_init();
    config.vtable = &nvtable;
    if constexpr (input_buses == MA_NODE_BUS_COUNT_UNKNOWN) {
      assert(actual_input_bus_count);
      config.inputBusCount = *actual_input_bus_count;
    }
    if constexpr (output_buses == MA_NODE_BUS_COUNT_UNKNOWN) {
      assert(actual_output_bus_count);
      config.outputBusCount = *actual_output_bus_count;
    }
    ma_node_init(graph, &config, nullptr, &base);
  }

  ~AudioNode() { ma_node_uninit(&base); }

  operator ma_node *() noexcept { return reinterpret_cast<ma_node *>(&base); }

  operator const ma_node *() const noexcept {
    return reinterpret_cast<const ma_node *>(&base);
  }

  // To do: finish audio node class with callbacks and final implementations of
  // remaining functions
};
