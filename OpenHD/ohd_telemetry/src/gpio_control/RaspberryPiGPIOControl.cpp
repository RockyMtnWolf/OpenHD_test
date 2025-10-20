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

#include "RaspberryPiGPIOControl.h"

#include "openhd_util.h"

namespace openhd::telemetry::rpi {

static void configure_gpio_as_output(int gpio_number) {
  OHDUtil::run_command("raspi-gpio",
                       {"set", std::to_string(gpio_number), "op"});
}

static void configure_gpio_low_high(int gpio_number, bool low) {
  const std::string tmp = low ? "dl" : "dh";  // drive low / drive high
  OHDUtil::run_command("raspi-gpio", {"set", std::to_string(gpio_number), tmp});
}

static void configure_gpio(int gpio_number, int gpio_value) {
  if (gpio_value == GPIO_LEAVE_UNTOUCHED) {
    return;
  }
  const bool low = gpio_value == GPIO_LOW;
  configure_gpio_as_output(gpio_number);
  configure_gpio_low_high(gpio_number, low);
}

static bool validate_gpio_setting_int(int value) {
  return value == 0 || value == 1 || value == 2;
}

GPIOControl::GPIOControl() {
  m_settings = std::make_unique<GPIOControlSettingsHolder>();
  const auto& tmp = m_settings->get_settings();
  configure_gpio(2, tmp.gpio_2);
  configure_gpio(2, tmp.gpio_26);
}

std::vector<openhd::Setting> GPIOControl::get_all_settings() {
  std::vector<openhd::Setting> ret;
  auto cb_gpio2 = [this](std::string, int value) {
    if (!validate_gpio_setting_int(value)) return false;
    m_settings->unsafe_get_settings().gpio_2 = value;
    m_settings->persist();
    configure_gpio(2, value);
    return true;
  };
  ret.push_back(openhd::Setting{
      "GPIO_2",
      openhd::IntSetting{static_cast<int>(m_settings->get_settings().gpio_2),
                         cb_gpio2}});
  auto cb_gpio26 = [this](std::string, int value) {
    if (!validate_gpio_setting_int(value)) return false;
    m_settings->unsafe_get_settings().gpio_2 = value;
    m_settings->persist();
    configure_gpio(26, value);
    return true;
  };
  ret.push_back(openhd::Setting{
      "GPIO_26",
      openhd::IntSetting{static_cast<int>(m_settings->get_settings().gpio_26),
                         cb_gpio26}});
  return ret;
}

}  // namespace openhd::telemetry::rpi
