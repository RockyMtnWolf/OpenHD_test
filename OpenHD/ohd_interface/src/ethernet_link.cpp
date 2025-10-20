/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

#include "ethernet_link.h"

#include <arpa/inet.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "config_paths.h"
#include "openhd_config.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"

static std::string ETHERNET_FILE_PATH =
    std::string(getConfigBasePath()) + "ethernet.txt";

EthernetLink::EthernetLink(const openhd::Config& config, OHDProfile profile)
    : m_config(config), m_profile(profile) {
  std::cout << "ethernet starting " << std::endl;

  if (OHDFilesystemUtil::exists(ETHERNET_FILE_PATH)) {
    const auto config = openhd::load_config();
    std::cout << "ethernet config load " << std::endl;

    try {
      static const auto GROUND_UNIT_IP = config.GROUND_UNIT_IP;
      static const auto AIR_UNIT_IP = config.AIR_UNIT_IP;
      static const auto VIDEO_PORT = config.VIDEO_PORT;
      static const auto TELEMETRY_PORT = config.TELEMETRY_PORT;

      // Debugging the values after assignment
      std::cout << "Assigned ethernet parameters:" << std::endl;
      std::cout << "  GROUND_UNIT_IP: " << config.GROUND_UNIT_IP << std::endl;
      std::cout << "  AIR_UNIT_IP: " << AIR_UNIT_IP << std::endl;
      std::cout << "  VIDEO_PORT: " << VIDEO_PORT << std::endl;
      std::cout << "  TELEMETRY_PORT: " << TELEMETRY_PORT << std::endl;
    } catch (const std::exception& ex) {
      std::cerr << "Failed to read ethernet parameters: " << ex.what()
                << std::endl;
      throw;
    }
  } else {
    std::cerr << "Ethernet parameters not found. Using default configuration."
              << std::endl;
  }

  // Initialize either air or ground unit based on the profile
  if (m_profile.is_air) {
    initialize_air_unit();
  } else {
    initialize_ground_unit();
  }
}

EthernetLink::EthernetLink(OHDProfile profile)
    : EthernetLink(openhd::load_config(), profile) {}

EthernetLink::~EthernetLink() {
  // Stop background receivers
  if (m_video_rx) m_video_rx->stopBackground();
  if (m_telemetry_rx) m_telemetry_rx->stopBackground();
}

void EthernetLink::initialize_air_unit() {
  // Initialize video transmitter for sending video to the ground unit
  m_video_tx =
      std::make_unique<openhd::UDPForwarder>(GROUND_UNIT_IP, VIDEO_PORT);

  // Initialize telemetry transmitter and receiver for bidirectional telemetry
  m_telemetry_tx =
      std::make_unique<openhd::UDPForwarder>(GROUND_UNIT_IP, TELEMETRY_PORT);
  m_telemetry_rx = std::make_unique<openhd::UDPReceiver>(
      "0.0.0.0", TELEMETRY_PORT, [this](const uint8_t* data, std::size_t len) {
        handle_telemetry_data(data, len);  // Process incoming telemetry
      });

  // Start telemetry receiver in the background
  if (m_telemetry_rx) m_telemetry_rx->runInBackground();
}

void EthernetLink::initialize_ground_unit() {
  // Initialize video receiver for receiving video from the air unit
  m_video_rx = std::make_unique<openhd::UDPReceiver>(
      "0.0.0.0", VIDEO_PORT, [this](const uint8_t* data, std::size_t len) {
        handle_video_data(0, data, len);  // Process incoming video
      });

  // Initialize telemetry transmitter and receiver for bidirectional telemetry
  m_telemetry_tx =
      std::make_unique<openhd::UDPForwarder>(AIR_UNIT_IP, TELEMETRY_PORT);
  m_telemetry_rx = std::make_unique<openhd::UDPReceiver>(
      "0.0.0.0", TELEMETRY_PORT, [this](const uint8_t* data, std::size_t len) {
        handle_telemetry_data(data, len);  // Process incoming telemetry
      });

  // Start video and telemetry receivers in the background
  if (m_video_rx) m_video_rx->runInBackground();
  if (m_telemetry_rx) m_telemetry_rx->runInBackground();
}

void EthernetLink::transmit_telemetry_data(TelemetryTxPacket packet) {
  // Send telemetry data to the destination
  if (m_telemetry_tx) {
    m_telemetry_tx->forwardPacketViaUDP(packet.data->data(),
                                        packet.data->size());
  }
}

void EthernetLink::transmit_video_data(
    int stream_index,
    const openhd::FragmentedVideoFrame& fragmented_video_frame) {
  // Send video data fragments to the destination
  if (m_video_tx) {
    for (const auto& fragment : fragmented_video_frame.rtp_fragments) {
      m_video_tx->forwardPacketViaUDP(fragment->data(), fragment->size());
    }
  }
}

void EthernetLink::transmit_audio_data(
    const openhd::AudioPacket& audio_packet) {
  // Currently not implemented for EthernetLink
}

void EthernetLink::handle_video_data(int stream_index, const uint8_t* data,
                                     int data_len) {
  // Forward incoming video data to the upper layer
  on_receive_video_data(stream_index, data, data_len);
}

void EthernetLink::handle_telemetry_data(const uint8_t* data, int data_len) {
  // Forward incoming telemetry data to the upper layer
  auto shared = std::make_shared<std::vector<uint8_t>>(data, data + data_len);
  on_receive_telemetry_data(shared);
}