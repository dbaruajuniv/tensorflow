/* Copyright 2018 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "tensorflow/c/env.h"

#include "tensorflow/c/c_api_internal.h"
#include "tensorflow/c/tf_status_helper.h"
#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/platform/types.h"

struct TF_StringStream {
  std::vector<::tensorflow::string>* list;
  size_t position;
};

void TF_CreateDir(const char* dirname, TF_Status* status) {
  TF_SetStatus(status, TF_OK, "");
  ::tensorflow::Set_TF_Status_from_Status(
      status, ::tensorflow::Env::Default()->CreateDir(dirname));
}

void TF_DeleteDir(const char* dirname, TF_Status* status) {
  TF_SetStatus(status, TF_OK, "");
  ::tensorflow::Set_TF_Status_from_Status(
      status, ::tensorflow::Env::Default()->DeleteDir(dirname));
}

void TF_DeleteRecursively(const char* dirname, int64_t* undeleted_file_count,
                          int64_t* undeleted_dir_count, TF_Status* status) {
  ::tensorflow::int64 f, d;

  TF_SetStatus(status, TF_OK, "");
  ::tensorflow::Set_TF_Status_from_Status(
      status, ::tensorflow::Env::Default()->DeleteRecursively(dirname, &f, &d));
  *undeleted_file_count = f;
  *undeleted_dir_count = d;
}

void TF_FileStat(const char* filename, TF_FileStatistics* stats,
                 TF_Status* status) {
  ::tensorflow::FileStatistics cc_stats;
  TF_SetStatus(status, TF_OK, "");
  ::tensorflow::Status s =
      ::tensorflow::Env::Default()->Stat(filename, &cc_stats);
  ::tensorflow::Set_TF_Status_from_Status(status, s);
  if (s.ok()) {
    stats->length = cc_stats.length;
    stats->mtime_nsec = cc_stats.mtime_nsec;
    stats->is_directory = cc_stats.is_directory;
  }
}

void TF_NewWritableFile(const char* filename, TF_WritableFileHandle** handle,
                        TF_Status* status) {
  std::unique_ptr<::tensorflow::WritableFile> f;
  TF_SetStatus(status, TF_OK, "");
  ::tensorflow::Status s =
      ::tensorflow::Env::Default()->NewWritableFile(filename, &f);
  ::tensorflow::Set_TF_Status_from_Status(status, s);

  if (s.ok()) {
    *handle = reinterpret_cast<TF_WritableFileHandle*>(f.release());
  }
}

void TF_CloseWritableFile(TF_WritableFileHandle* handle, TF_Status* status) {
  auto* cc_file = reinterpret_cast<::tensorflow::WritableFile*>(handle);
  TF_SetStatus(status, TF_OK, "");
  ::tensorflow::Set_TF_Status_from_Status(status, cc_file->Close());
  delete cc_file;
}

void TF_SyncWritableFile(TF_WritableFileHandle* handle, TF_Status* status) {
  auto* cc_file = reinterpret_cast<::tensorflow::WritableFile*>(handle);
  TF_SetStatus(status, TF_OK, "");
  ::tensorflow::Set_TF_Status_from_Status(status, cc_file->Sync());
}

void TF_FlushWritableFile(TF_WritableFileHandle* handle, TF_Status* status) {
  auto* cc_file = reinterpret_cast<::tensorflow::WritableFile*>(handle);
  TF_SetStatus(status, TF_OK, "");
  ::tensorflow::Set_TF_Status_from_Status(status, cc_file->Flush());
}

void TF_AppendWritableFile(TF_WritableFileHandle* handle, const char* data,
                           size_t length, TF_Status* status) {
  auto* cc_file = reinterpret_cast<::tensorflow::WritableFile*>(handle);
  TF_SetStatus(status, TF_OK, "");
  ::tensorflow::Set_TF_Status_from_Status(
      status, cc_file->Append(::tensorflow::StringPiece{data, length}));
}

void TF_DeleteFile(const char* filename, TF_Status* status) {
  TF_SetStatus(status, TF_OK, "");
  ::tensorflow::Set_TF_Status_from_Status(
      status, ::tensorflow::Env::Default()->DeleteFile(filename));
}

bool TF_StringStreamNext(TF_StringStream* list, const char** result) {
  if (list->position >= list->list->size()) {
    *result = nullptr;
    return false;
  }

  *result = list->list->at(list->position++).c_str();
  return true;
}

void TF_StringStreamDone(TF_StringStream* list) {
  delete list->list;
  delete list;
}
TF_StringStream* TF_GetChildren(const char* dirname, TF_Status* status) {
  auto* children = new std::vector<::tensorflow::string>;

  TF_SetStatus(status, TF_OK, "");
  ::tensorflow::Set_TF_Status_from_Status(
      status, ::tensorflow::Env::Default()->GetChildren(dirname, children));

  auto* list = new TF_StringStream;
  list->list = children;
  list->position = 0;
  return list;
}

TF_StringStream* TF_GetLocalTempDirectories() {
  auto* tmpdirs = new std::vector<::tensorflow::string>;

  ::tensorflow::Env::Default()->GetLocalTempDirectories(tmpdirs);

  auto* list = new TF_StringStream;
  list->list = tmpdirs;
  list->position = 0;
  return list;
}
