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

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_UTIL_FILESYSTEM_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_UTIL_FILESYSTEM_H_

#include <optional>
#include <string>
#include <vector>

// boost::filesystem or std::filesystem, what a pain
// If possible, one should not use boost::filesystem or anything from boost::
// inside the project, but quickly write a wrapper here.
// We want to get rid of boost at some time.
// which should reduce build time and dependency issues on all the platforms.
// However, for std::filesystem we need c++17 AND support from the compiler for
// std::filesystem which is lagging behind immensely.

namespace OHDFilesystemUtil {

/**
 * Quite common during the discovery step, find all directory entries in a
 * directory. If the given directory does not exist, return no entries.
 * @param directory
 * @return the full paths of each entry in the directory.
 */
std::vector<std::string> getAllEntriesFullPathInDirectory(
    const std::string& directory);

// Same as above, but returns the filenames only.
std::vector<std::string> getAllEntriesFilenameOnlyInDirectory(
    const std::string& directory);

std::optional<std::string> getFirstMatchingDirectoryByPrefix(
    const std::string& directory, const std::string& prefix);

std::vector<std::string> getAllMatchingDirectoriesByPrefix(
    const std::string& directory, const std::string& prefix);
// same as boost::filesystem::exists
bool exists(const std::string& file);

// These don't create top level directories recursively
void create_directory(const std::string& directory);

// creates top level directories recursively
void create_directories(const std::string& directory);

// delete directory if it exists
void safe_delete_directory(const std::string& directory);

// Write a text file. If the file already exists, its content is overwritten.
// This does not recursively create directories(s) - use create_directories
// before if needed. This method intentionally doesn't return an error code, but
// logs verbose warning(s) when things go wrong.
void write_file(const std::string& path, const std::string& content);

// Read a file as text and return its content as a string.
// If the file doesn't exist, return std::nullopt
std::optional<std::string> opt_read_file(const std::string& filename,
                                         bool log_debug = true);

// Similar to above, but returns empty string if file doesn't exist
std::string read_file(const std::string& filename);

void remove_if_existing(const std::string& filename);

// OpenHD is run as root and as such, any file(s) it creates are only read/write
// as root Changes the permission of a file to read write anybody (non-root)
void make_file_read_write_everyone(const std::string& filename);

// Return: remaining space in root directory
int get_remaining_space_in_mb();

// return size of file in bytes,
// -1 if file does not exist
long get_file_size_bytes(const std::string& filepath);

std::optional<int> read_int_from_file(const std::string& filename);

}  // namespace OHDFilesystemUtil

#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_UTIL_FILESYSTEM_H_
