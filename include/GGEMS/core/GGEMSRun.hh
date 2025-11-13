#pragma once

namespace ggems::core {
class GGEMSRun {
public:
  GGEMSRun();
  ~GGEMSRun();

  void Initialise();
  void Run();

private:
  void RunMT();
  void Banner() const;

private:
};
} // namespace ggems::core
