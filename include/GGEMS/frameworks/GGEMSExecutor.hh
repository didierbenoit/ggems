#pragma once

namespace ggems {
class GGEMSExecutor {
public:
  GGEMSExecutor();
  ~GGEMSExecutor();

  void Initialize();
  void Run();
  void SelectDevices();

private:
  void RunMT();
  void Banner() const;

private:
};
} // namespace ggems
