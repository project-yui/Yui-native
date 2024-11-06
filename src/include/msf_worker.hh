#ifndef __MSF_WORKER_HH__
#define __MSF_WORKER_HH__
#include "../include/native_msf.hh"
#include "spdlog/spdlog.h"
#include <future>
#include <napi.h>
/**
 * @brief 异步任务
 *
 * 参考：https://github.com/nodejs/node-addon-examples/issues/85#issuecomment-551142933
 *
 */
class MsfWorker : public Napi::AsyncWorker {
public:
  MsfWorker(Napi::Env &env, yui::CustomTaskPkg pkg,
            Napi::Promise::Deferred deferred)
      : Napi::AsyncWorker(env), pkg(pkg), deferred(deferred) {}

  ~MsfWorker();

  // Executed inside the worker-thread.
  // It is not safe to access JS engine data structure
  // here, so everything we need for input and output
  // should go on `this`.
  void Execute();

  // Executed when the async work is complete
  // this function will be run inside the main event loop
  // so it is safe to use JS engine data again
  void OnOK();

  void OnError(Napi::Error const &error);

private:
  yui::CustomTaskPkg pkg;
  std::pair<void *, long> result;
  Napi::Promise::Deferred deferred;
};
#endif