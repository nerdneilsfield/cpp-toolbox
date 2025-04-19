#include <cpp-toolbox/file/memory_mapped_file.hpp>
#include <cpp-toolbox/logger/thread_logger.hpp>  // For LOG_* macros

// Include necessary headers that were originally in the hpp
#include <cstring>  // For strerror on POSIX
#include <exception>  // For std::exception
#include <limits>  // For std::numeric_limits
#include <system_error>  // For std::error_code

// Platform-specific includes might be needed again if not pulled by hpp
#ifdef CPP_TOOLBOX_PLATFORM_WINDOWS
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  include <fcntl.h>  // For open()
#  include <sys/mman.h>  // For mmap(), munmap()
#  include <unistd.h>  // For close()
#endif

namespace toolbox::file
{

memory_mapped_file_t::~memory_mapped_file_t()
{
  close();
}

bool memory_mapped_file_t::open(const std::filesystem::path& path)
{
  close();  // Ensure any previous mapping is closed

  try {
    std::error_code ec;
    // Use uintmax_t for the raw result as required by the standard
    uintmax_t file_size_raw = std::filesystem::file_size(path, ec);

    // 1. Check for filesystem error first
    if (ec) {
      LOG_ERROR_S << "Failed to get file size for: " << path.string()
                  << ". Error: " << ec.message() << " (Code: " << ec.value()
                  << ")";
      // m_mapped_size is already 0 due to close() at the start
      return false;
    }

    // 2. Check for empty file
    if (file_size_raw == 0) {
      LOG_ERROR_S << "File is empty, cannot map: " << path.string();
      // m_mapped_size is already 0 due to close() at the start
      return false;
    }

    // 3. Check if file size exceeds size_t capacity
    if (file_size_raw > std::numeric_limits<size_t>::max()) {
      LOG_ERROR_S << "File size exceeds size_t capacity: " << path.string()
                  << " Size: " << file_size_raw;
      // m_mapped_size is already 0 due to close() at the start
      return false;
    }

    // 4. Assign size only if valid and non-zero
    m_mapped_size = static_cast<size_t>(file_size_raw);

#ifdef CPP_TOOLBOX_PLATFORM_WINDOWS
    m_file_handle = CreateFileW(path.c_str(),
                                GENERIC_READ,
                                FILE_SHARE_READ,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
                                NULL);
    if (m_file_handle == INVALID_HANDLE_VALUE) {
      LOG_ERROR_S << "Failed to open file (CreateFileW): " << path.string()
                  << ". Error code: " << GetLastError();
      m_mapped_size = 0;  // Reset size on failure
      return false;
    }

    m_mapping_handle =
        CreateFileMappingW(m_file_handle, NULL, PAGE_READONLY, 0, 0, NULL);
    if (m_mapping_handle == NULL) {
      LOG_ERROR_S << "Failed to create file mapping (CreateFileMappingW): "
                  << path.string() << ". Error code: " << GetLastError();
      CloseHandle(m_file_handle);
      m_file_handle = INVALID_HANDLE_VALUE;
      m_mapped_size = 0;  // Reset size on failure
      return false;
    }

    m_mapped_ptr =
        MapViewOfFile(m_mapping_handle, FILE_MAP_READ, 0, 0, m_mapped_size);
    if (m_mapped_ptr == NULL) {
      LOG_ERROR_S << "Failed to map view of file (MapViewOfFile): "
                  << path.string() << ". Error code: " << GetLastError();
      CloseHandle(m_mapping_handle);
      m_mapping_handle = NULL;
      CloseHandle(m_file_handle);
      m_file_handle = INVALID_HANDLE_VALUE;
      m_mapped_size = 0;  // Reset size on failure
      return false;
    }
#else  // POSIX
       // Note: Need filesystem path string for open()
    m_fd = ::open(path.c_str(), O_RDONLY);
    if (m_fd == -1) {
      LOG_ERROR_S << "Failed to open file (open): " << path.string()
                  << ". Error: " << strerror(errno);
      m_mapped_size = 0;  // Reset size on failure
      return false;
    }

    m_mapped_ptr =
        mmap(nullptr, m_mapped_size, PROT_READ, MAP_PRIVATE, m_fd, 0);
    if (m_mapped_ptr == MAP_FAILED) {
      LOG_ERROR_S << "Failed to map file (mmap): " << path.string()
                  << ". Error: " << strerror(errno);
      ::close(m_fd);
      m_fd = -1;
      m_mapped_ptr = nullptr;
      m_mapped_size = 0;  // Reset size on failure
      return false;
    }
#endif
    LOG_DEBUG_S << "Successfully memory-mapped file: " << path.string()
                << " size: " << m_mapped_size;
    return true;
  } catch (const std::filesystem::filesystem_error& fs_ex) {
    LOG_ERROR_S << "Filesystem exception opening/mapping file " << path.string()
                << ": " << fs_ex.what() << " (Code: " << fs_ex.code().message()
                << ")";
    close();  // close() resets m_mapped_size to 0
    return false;
  } catch (const std::exception& e) {
    LOG_ERROR_S << "Generic exception opening/mapping file " << path.string()
                << ": " << e.what();
    close();  // close() resets m_mapped_size to 0
    return false;
  }
}

void memory_mapped_file_t::close()
{
  if (m_mapped_ptr != nullptr) {
#ifdef CPP_TOOLBOX_PLATFORM_WINDOWS
    if (!UnmapViewOfFile(m_mapped_ptr)) {
      LOG_WARN_S << "Failed to unmap view of file. Error code: "
                 << GetLastError();
    }
#else
    if (munmap(m_mapped_ptr, m_mapped_size) == -1) {
      LOG_WARN_S << "Failed to unmap file (munmap). Error: " << strerror(errno);
    }
#endif
    m_mapped_ptr = nullptr;
  }

#ifdef CPP_TOOLBOX_PLATFORM_WINDOWS
  if (m_mapping_handle != NULL) {
    if (!CloseHandle(m_mapping_handle)) {
      LOG_WARN_S << "Failed to close mapping handle. Error code: "
                 << GetLastError();
    }
    m_mapping_handle = NULL;
  }
  if (m_file_handle != INVALID_HANDLE_VALUE) {
    if (!CloseHandle(m_file_handle)) {
      LOG_WARN_S << "Failed to close file handle. Error code: "
                 << GetLastError();
    }
    m_file_handle = INVALID_HANDLE_VALUE;
  }
#else
  if (m_fd != -1) {
    if (::close(m_fd) == -1) {
      LOG_WARN_S << "Failed to close file descriptor. Error: "
                 << strerror(errno);
    }
    m_fd = -1;
  }
#endif
  m_mapped_size = 0;
}

}  // namespace toolbox::file
