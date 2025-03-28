#include "physicalfilestream.h"
#include "../log.h"
#include "io.h"

#include <cstring>
#include <algorithm>
#include <system_error>

namespace Impacto {
namespace Io {
std::ios_base::openmode PhysicalFileStream::PrepareFileOpenMode(
    CreateFlags flags) {
  std::ios_base::openmode mode = std::ios::binary;
  IoError fileExists = PathExists(SourceFileName);
  if (fileExists == IoError_Fail) {
    ErrorCode = IoError_Fail;
    return {};
  }

  bool writeNoExistNoCreate = (flags & WRITE) &&
                              (fileExists == IoError_NotFound) &&
                              !(flags & CREATE_IF_NOT_EXISTS);
  bool readNoExist =
      !(flags & WRITE) && (flags & READ) && (fileExists == IoError_NotFound);
  if (writeNoExistNoCreate || readNoExist) {
    ErrorCode = IoError_NotFound;
    std::string errMsg =
        std::make_error_code(std::errc::no_such_file_or_directory).message();
    ImpLog(LogLevel::Error, LogChannel::IO,
           "Failed to open stream \"{:s}\", error: \"{:s}\"\n", SourceFileName,
           errMsg);
    return {};
  }

  bool writeExistNoOverwrite =
      (flags & WRITE) && fileExists && !(flags & TRUNCATE) && !(flags & APPEND);
  if (writeExistNoOverwrite) {
    // avoid truncating when in write only mode without truncate flag to
    // preserve file size
    // I think this is more intuitive than making it an error
    // or letting the truncate happen
    flags |= CreateFlagsMode::READ;
  }

  // require write for create/overwrite flags
  assert((flags & WRITE) || !(flags & CREATE_DIRS));
  assert((flags & WRITE) || !(flags & CREATE_IF_NOT_EXISTS));

  // truncate is only needed when creating nonexistent if also reading
  bool truncFlag = (flags & TRUNCATE) ||
                   ((flags & READ) && !fileExists &&
                    (flags & CREATE_IF_NOT_EXISTS) && !(flags & APPEND));

  // trunc and append are mutually exclusive
  assert(!truncFlag || !(flags & APPEND));

  if (flags & READ) mode |= std::ios::in;
  if (flags & WRITE) mode |= std::ios::out;
  if (flags & APPEND) mode |= std::ios::app;
  if (truncFlag) mode |= std::ios::trunc;
  if (flags & CREATE_DIRS) {
    auto result = Io::CreateDirectories(SourceFileName, true);
    if (result == IoError_Fail) {
      ErrorCode = IoError_Fail;
      return {};
    }
  }
  ErrorCode = IoError_OK;
  return mode;
}

IoError PhysicalFileStream::Create(std::string const& fileName, Stream** out,
                                   CreateFlags flags) {
  PhysicalFileStream* result =
      new PhysicalFileStream(GetSystemDependentPath(fileName), flags);
  if (auto err = result->ErrorCode; err != IoError_OK) {
    ImpLog(LogLevel::Error, LogChannel::IO, "Failed to open file \"{:s}\"\n",
           result->SourceFileName);
    delete result;
    return err;
  }
  if (!result->FileStream) {
    ImpLog(LogLevel::Error, LogChannel::IO,
           "Failed to open file \"{:s}\", error: \"{:s}\"\n",
           result->SourceFileName, std::generic_category().message(errno));
    delete result;
    return IoError_Fail;
  }
  result->Meta.Size = GetFileSize(result->SourceFileName);
  if (result->Meta.Size == IoError_Fail) {
    delete result;
    return IoError_Fail;
  }
  *out = (Stream*)result;
  return IoError_OK;
}

int64_t PhysicalFileStream::Read(void* buffer, int64_t sz) {
  if (sz < 0 || !(Flags & READ)) return IoError_Fail;
  if (Position >= Meta.Size) return IoError_Eof;

  int bytesToRead = std::min(sz, Meta.Size - Position);
  FileStream.read((char*)buffer, bytesToRead);
  auto read = FileStream.gcount();

  if (read == 0) {  // Check if no data was read
    if (FileStream.eof()) return IoError_Eof;
    ImpLog(LogLevel::Error, LogChannel::IO,
           "Read failed for file \"{:s}\" with error: \"{:s}\"\n",
           SourceFileName, std::generic_category().message(errno));
    FileStream.clear(FileStream.rdstate() & ~std::ios::failbit &
                     ~std::ios::eofbit);
    return IoError_Fail;
  }

  Position += read;
  return read;
}

int64_t PhysicalFileStream::Seek(int64_t offset, int origin) {
  if (!(Flags & READ) && (Flags & APPEND)) {
    // seeking is useless in readless append mode
    return IoError_Fail;
  }
  int64_t absPos;
  switch (origin) {
    case RW_SEEK_SET:
      absPos = offset;
      break;
    case RW_SEEK_CUR:
      absPos = Position + offset;
      break;
    case RW_SEEK_END:
      absPos = Meta.Size - offset;
      break;
  }

  // seeking past EOF is a legal operation, after write past EOF, the gap
  // between prev file size and write pos is zero padded
  if (absPos < 0) return IoError_Fail;
  FileStream.seekg(absPos, std::ios::beg);
  if (!FileStream && !FileStream.eof()) {
    ImpLog(LogLevel::Error, LogChannel::IO,
           "Seek failed for file \"{:s}\" with error: \"{:s}\"\n",
           SourceFileName, std::generic_category().message(errno));
    FileStream.clear(FileStream.rdstate() & ~std::ios::failbit &
                     ~std::ios::eofbit);  // Clear only failbit and eofbit
    return IoError_Fail;
  }
  Position = FileStream.tellg();
  return Position;
}

IoError PhysicalFileStream::Duplicate(Stream** outStream) {
  PhysicalFileStream* result = new PhysicalFileStream(*this);
  if (IoError err = result->ErrorCode; err != IoError_OK) {
    ImpLog(LogLevel::Error, LogChannel::IO, "Failed to open file \"{:s}\"\n",
           SourceFileName);
    delete result;
    return err;
  }
  if (!result->FileStream) {
    ImpLog(LogLevel::Error, LogChannel::IO,
           "Failed to open file \"{:s}\", error: \"{:s}\"\n", SourceFileName,
           std::generic_category().message(errno));
    delete result;
    return IoError_Fail;
  }
  result->Meta.Size = GetFileSize(result->SourceFileName);
  if (result->Meta.Size == IoError_Fail) {
    delete result;
    return IoError_Fail;
  }
  if (result->Seek(Position, RW_SEEK_SET) < 0) {
    ImpLog(LogLevel::Error, LogChannel::IO,
           "Seek failed for file \"{:s}\" with error: \"{:s}\"\n",
           SourceFileName, std::generic_category().message(errno));
    delete result;
    return IoError_Fail;
  }
  *outStream = (Stream*)result;
  return IoError_OK;
}

int64_t PhysicalFileStream::Write(void* buffer, int64_t sz, int cnt) {
  if (!(Flags & WRITE)) {
    return IoError_Fail;
  }
  FileStream.write((char*)buffer, sz * cnt);
  if (!FileStream) {
    ImpLog(LogLevel::Error, LogChannel::IO,
           "Write failed for file \"{:s}\" with error: \"{:s}\"\n",
           SourceFileName, std::generic_category().message(errno));
    FileStream.clear(FileStream.rdstate() & ~std::ios::failbit &
                     ~std::ios::eofbit);  // Clear only failbit and eofbit
    return IoError_Fail;
  }
  int64_t written = sz * cnt;
  Position += written;
  Meta.Size = std::max(Position, Meta.Size);
  return written;
}

}  // namespace Io
}  // namespace Impacto