#pragma once
// ************************************************************************
// ************************************************************************


/// \cond
#include <....>
/// \endcond

#include "GGEMS/....hh"

namespace ggems::core {
/*!
 * \enum OS
 * \brief XXX
 *
 * XXX
 */
enum class  : std::uint8_t {
  ,   /*!<  */
  , /*!<  */
};

/*!
 * \struct XXX
 * \brief XXX
 *
 * XXX
 */
struct  {
  std::uint64_t ;     /*!<  */
  std::uint64_t ; /*!< */
};



/*!
 * \brief XXX
 *
 * XXX
 *
 * \param XXX XXX
 *
 * \return XXX
 */
[[nodiscard]] ReturnType
Method(Param param) noexcept;

/*!
 * \class XXX
 * \brief XXX
 *
 * XXX
 *
 * XXXX
 */
class XXX : public std::runtime_error {
public:
  /*!
   * \brief XXXX
   *
   * \param msg  XXX.
   * \param cat  XXX
   * \param loc  XXX
   * \param do_log XXX
   */
  explicit XXX(
      std::string msg, std::string cat,
      std::source_location loc = std::source_location::current(),
      bool do_log = true)
      : std::runtime_error(msg), file_(loc.file_name()),
        function_(loc.function_name()), category_(std::move(cat)),
        line_(static_cast<std::int32_t>(loc.line())) {}


protected:
  /*!
   * \brief XXX
   *
   * XXX
   *
   * \param XXX XXX
   */
  static void XXX(std::string const &XXXX) noexcept;


private:
  std::string XXX; /*!<  */
  char const *XXX; /*!<  */
};

} // namespace ggems::core
