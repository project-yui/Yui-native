
#include "../include/msf_worker.hh"
#include "napi.h"

MsfWorker::~MsfWorker() {}

// Executed inside the worker-thread.
// It is not safe to access JS engine data structure
// here, so everything we need for input and output
// should go on `this`.
void MsfWorker::Execute() {
  spdlog::debug("execute start");
  std::promise<std::pair<void *, long>> myPromise;
  std::future<std::pair<void *, long>> myFuture =
      myPromise.get_future(); // 与 promise 关联的 future
  pkg.promise = &myPromise;
  yui::msf_request_add(pkg);
  result = myFuture.get();

  // you could handle errors as well
  // throw std::runtime_error("test error");
  // or like
  // Napi::AsyncWorker::SetError
  // Napi::AsyncWorker::SetError("test error");
}

// Executed when the async work is complete
// this function will be run inside the main event loop
// so it is safe to use JS engine data again
void MsfWorker::OnOK() {
  spdlog::debug("on ok....");
  auto ab = Napi::Uint8Array::New(Env(), result.second);
  auto data = (uint8_t *)ab.Data();
  spdlog::debug("write data....");
  for (int i = 0; i < result.second; i++) {
    data[i] = ((uint8_t *)result.first)[i];
  }
  delete[] (uint8_t *)result.first;
  spdlog::debug("resolve....");
  deferred.Resolve(ab);
}

void MsfWorker::OnError(Napi::Error const &error) {
  spdlog::error("error....");
  deferred.Reject(error.Value());
}